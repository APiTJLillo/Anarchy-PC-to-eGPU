#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>

#include "../../include/anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-perf.h"
#include "pcie.h"

/* NVIDIA RTX 4090 PCIe configuration */
#define NVIDIA_RTX_4090_VENDOR_ID 0x10DE
#define NVIDIA_RTX_4090_DEVICE_ID 0x2684

/* Memory window sizes */
#define BAR0_SIZE (16 * 1024 * 1024)  /* 16MB */
#define BAR1_SIZE (256 * 1024 * 1024) /* 256MB */
#define BAR3_SIZE (32 * 1024)         /* 32KB */

/* PCIe error types */
#define TB_PCIE_ERROR_TIMEOUT -ETIMEDOUT
#define TB_PCIE_ERROR_LINK_DOWN -ENOLINK
#define TB_PCIE_ERROR_DEVICE_NOT_FOUND -ENODEV
#define TB_PCIE_ERROR_RETRY -EAGAIN

/* Enhanced recovery parameters */
#define TB_PCIE_MAX_RETRIES 5
#define TB_PCIE_RETRY_DELAY_MS 100
#define TB_PCIE_LINK_TIMEOUT_MS 1000
#define TB_PCIE_TRAINING_TIMEOUT_MS 500

/* PCIe training wait queue */
static DECLARE_WAIT_QUEUE_HEAD(pcie_training_wait);

/* PCIe recovery work structure */
struct anarchy_pcie_recovery_work {
	struct work_struct work;
	struct anarchy_device *adev;
	int error;  /* Use int instead of enum */
	int retries;
	unsigned long delay;
};

/**
 * anarchy_pcie_setup_bars - Initialize Base Address Registers
 * @adev: Anarchy device structure
 *
 * Sets up the BAR configuration for the emulated GPU.
 */
int anarchy_pcie_setup_bars(struct anarchy_device *adev)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;

	/* Initialize BAR registers */
	cfg->bar[0] = 0;
	cfg->bar_mask[0] = ~(BAR0_SIZE - 1);

	/* BAR1 is not used */
	cfg->bar[1] = 0;
	cfg->bar_mask[1] = 0;

	/* BAR2/3 for framebuffer (64-bit) */
	cfg->bar[2] = 0;
	cfg->bar[3] = 0;
	cfg->bar_mask[2] = 0xFFFFFFFF;  /* Use 32-bit mask for 64-bit BAR */
	cfg->bar_mask[3] = 0xFFFFFFFF;

	/* BAR4/5 not used */
	cfg->bar[4] = 0;
	cfg->bar[5] = 0;
	cfg->bar_mask[4] = 0;
	cfg->bar_mask[5] = 0;

	/* Map MMIO and framebuffer regions */
	cfg->mmio_base = ioremap(cfg->bar[0], BAR0_SIZE);
	if (!cfg->mmio_base)
		return -ENOMEM;

	cfg->fb_base = ioremap(cfg->bar[2], BAR1_SIZE);
	if (!cfg->fb_base) {
		iounmap(cfg->mmio_base);
		return -ENOMEM;
	}

	return 0;
}

/**
 * anarchy_pcie_setup_capabilities - Initialize PCIe capabilities
 * @adev: Anarchy device structure
 *
 * Sets up the PCIe capabilities for the emulated GPU.
 */
int anarchy_pcie_setup_capabilities(struct anarchy_device *adev)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;
	
	/* Power Management capability */
	cfg->caps.pm_cap = ANARCHY_CAP_PM;
	cfg->caps.pm_ctrl = 0;  /* Power state D0 */
	
	/* MSI capability */
	cfg->caps.msi_cap = ANARCHY_CAP_MSI;
	cfg->caps.msi_ctrl = PCI_MSI_FLAGS_64BIT;  /* 64-bit MSI support */
	cfg->caps.msi_addr = 0;
	cfg->caps.msi_data = 0;
	
	/* PCIe capability */
	cfg->caps.pcie_cap = ANARCHY_CAP_PCIE;
	cfg->caps.pcie_ctrl = PCI_EXP_FLAGS_VERS | PCI_EXP_FLAGS_TYPE;
	
	/* Link capabilities */
	cfg->caps.link_cap = (adev->max_lanes << 4) |  /* Max link width */
						(adev->max_speed << 0);    /* Max link speed */
	
	cfg->caps.link_ctrl = PCI_EXP_LNKCTL_ASPM_L0S | /* ASPM L0s enabled */
						  PCI_EXP_LNKCTL_CCC |     /* Common Clock Config */
						  PCI_EXP_LNKCTL_RL;      /* Retrain Link */
						  
	cfg->caps.link_status = (adev->max_lanes << 4) |  /* Negotiated width */
						   (adev->max_speed << 0);    /* Current speed */

	return 0;
}

/**
 * anarchy_pcie_read_config - Read from configuration space
 * @adev: Anarchy device structure
 * @where: Register offset
 * @size: Access size (1, 2, or 4 bytes)
 * @val: Value read
 */
int anarchy_pcie_read_config(struct anarchy_device *adev, int where, int size, u32 *val)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;
	u32 reg = 0;
	
	if (where >= ANARCHY_CFG_SIZE)
		return -EINVAL;
		
	switch (where & ~3) {
	case PCI_VENDOR_ID:
		reg = (NVIDIA_RTX_4090_DEVICE_ID << 16) | NVIDIA_RTX_4090_VENDOR_ID;
		break;
	case PCI_COMMAND:
		reg = (cfg->status << 16) | cfg->command;
		break;
	case PCI_CLASS_REVISION:
		reg = (PCI_CLASS_VGA << 8) | cfg->revision;
		break;
	case PCI_CACHE_LINE_SIZE:
		reg = (cfg->bist << 24) | (cfg->header_type << 16) |
			  (cfg->latency_timer << 8) | cfg->cache_line_size;
		break;
	case PCI_BASE_ADDRESS_0:
	case PCI_BASE_ADDRESS_1:
	case PCI_BASE_ADDRESS_2:
	case PCI_BASE_ADDRESS_3:
	case PCI_BASE_ADDRESS_4:
	case PCI_BASE_ADDRESS_5:
		{
			int bar = (where - PCI_BASE_ADDRESS_0) >> 2;
			reg = cfg->bar[bar];
			if (cfg->command & PCI_COMMAND_MEMORY)
				reg |= cfg->bar_mask[bar];
		}
		break;
	case PCI_INTERRUPT_LINE:
		reg = (cfg->interrupt_pin << 8) | cfg->interrupt_line;
		break;
	case PCI_ROM_ADDRESS:
		reg = cfg->rom_base | (cfg->rom_enabled ? PCI_ROM_ADDRESS_ENABLE : 0);
		break;
	case PCI_CAPABILITY_LIST:
		reg = cfg->caps.pm_cap;  /* First capability in chain */
		break;
	default:
		/* Handle capability configuration space */
		if (where >= 0x40) {
			if (where >= cfg->caps.pm_cap && where < cfg->caps.pm_cap + 8) {
				switch (where & ~3) {
				case 0: reg = (PCI_CAP_ID_PM << 8) | cfg->caps.msi_cap; break;
				case 4: reg = cfg->caps.pm_ctrl; break;
				}
			}
			else if (where >= cfg->caps.msi_cap && where < cfg->caps.msi_cap + 16) {
				switch (where & ~3) {
				case 0: reg = (PCI_CAP_ID_MSI << 8) | cfg->caps.pcie_cap | cfg->caps.msi_ctrl; break;
				case 4: reg = cfg->caps.msi_addr; break;
				case 8: reg = (u32)(cfg->caps.msi_addr & 0xFFFFFFFF00000000ULL >> 32); break;
				case 12: reg = cfg->caps.msi_data; break;
				}
			}
			else if (where >= cfg->caps.pcie_cap && where < cfg->caps.pcie_cap + 32) {
				switch (where & ~3) {
				case 0: reg = (PCI_CAP_ID_EXP << 8) | 0 | cfg->caps.pcie_ctrl; break;
				case 4: reg = cfg->caps.link_cap; break;
				case 8: reg = (cfg->caps.link_status << 16) | cfg->caps.link_ctrl; break;
				}
			}
		}
		break;
	}
	
	/* Handle non-aligned access */
	if (size == 1)
		reg = (reg >> ((where & 3) * 8)) & 0xff;
	else if (size == 2)
		reg = (reg >> ((where & 3) * 8)) & 0xffff;
	
	*val = reg;
	return 0;
}

/**
 * anarchy_pcie_write_config - Write to configuration space
 * @adev: Anarchy device structure
 * @where: Register offset
 * @size: Access size (1, 2, or 4 bytes)
 * @val: Value to write
 */
int anarchy_pcie_write_config(struct anarchy_device *adev, int where, int size, u32 val)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;
	u32 mask;
	
	if (where >= ANARCHY_CFG_SIZE)
		return -EINVAL;
		
	if (size == 1)
		mask = 0xff << ((where & 3) * 8);
	else if (size == 2)
		mask = 0xffff << ((where & 3) * 8);
	else
		mask = 0xffffffff;
		
	val = (val << ((where & 3) * 8)) & mask;
	
	switch (where & ~3) {
	case PCI_COMMAND:
		cfg->command = val & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER | PCI_COMMAND_SPECIAL |
				     PCI_COMMAND_INVALIDATE | PCI_COMMAND_PARITY |
				     PCI_COMMAND_SERR | PCI_COMMAND_FAST_BACK);
		break;
	case PCI_BASE_ADDRESS_0:
	case PCI_BASE_ADDRESS_1:
	case PCI_BASE_ADDRESS_2:
	case PCI_BASE_ADDRESS_3:
	case PCI_BASE_ADDRESS_4:
	case PCI_BASE_ADDRESS_5:
		{
			int bar = (where - PCI_BASE_ADDRESS_0) >> 2;
			if (cfg->bar_mask[bar]) {
				cfg->bar[bar] = val & ~cfg->bar_mask[bar];
				/* Handle BAR remapping here if needed */
			}
		}
		break;
	case PCI_INTERRUPT_LINE:
		cfg->interrupt_line = val & 0xff;
		break;
	case PCI_ROM_ADDRESS:
		cfg->rom_base = val & ~0x3ff;
		cfg->rom_enabled = val & PCI_ROM_ADDRESS_ENABLE;
		break;
	default:
		/* Handle capability configuration space */
		if (where >= 0x40) {
			if (where >= cfg->caps.pm_cap && where < cfg->caps.pm_cap + 8) {
				if ((where & ~3) == 4)
					cfg->caps.pm_ctrl = val & 0x8103;
			}
			else if (where >= cfg->caps.msi_cap && where < cfg->caps.msi_cap + 16) {
				switch (where & ~3) {
				case 0:
					cfg->caps.msi_ctrl = val & 0x710000;
					break;
				case 4:
					cfg->caps.msi_addr = (cfg->caps.msi_addr & 0xffffffff00000000ULL) |
							    (val & 0xfffffffc);
					break;
				case 8:
					cfg->caps.msi_addr = (cfg->caps.msi_addr & 0xffffffffULL) |
							    ((u64)val << 32);
					break;
				case 12:
					cfg->caps.msi_data = val & 0xffff;
					break;
				}
			}
			else if (where >= cfg->caps.pcie_cap && where < cfg->caps.pcie_cap + 32) {
				switch (where & ~3) {
				case 8:
					cfg->caps.link_ctrl = val & 0xffff;
					if (val & PCI_EXP_LNKCTL_RL)
						anarchy_pcie_retrain_link(adev);
					break;
				}
			}
		}
		break;
	}
	
	return 0;
}

/**
 * anarchy_pcie_retrain_link - Handle link retraining
 * @adev: Anarchy device structure
 *
 * Called when software requests link retraining.
 */
void anarchy_pcie_retrain_link(struct anarchy_device *adev)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;
	u32 link_ctrl;

	/* Read current link control */
	anarchy_pcie_read_config(adev, cfg->caps.pcie_cap + PCI_EXP_LNKCTL,
							4, &link_ctrl);

	/* Clear retrain bit */
	cfg->caps.link_ctrl &= ~PCI_EXP_LNKCTL_RL;

	/* Update link status to current capabilities */
	cfg->caps.link_status = (adev->max_lanes << 4) | (adev->max_speed << 0);
}

/**
 * anarchy_pcie_handle_error - Handle PCIe link errors
 * @adev: Anarchy device structure
 * @error: Type of error that occurred
 *
 * This function handles various PCIe link errors and initiates recovery.
 */
void anarchy_pcie_handle_error(struct anarchy_device *adev, int error)
{
	struct anarchy_pcie_recovery_work *recovery;
	
	dev_err(adev->dev, "PCIe error occurred: %d\n", error);
	
	recovery = kzalloc(sizeof(*recovery), GFP_KERNEL);
	if (!recovery) {
		dev_err(adev->dev, "Failed to allocate recovery work\n");
		return;
	}
	
	INIT_WORK(&recovery->work, anarchy_pcie_recovery_work);
	recovery->adev = adev;
	recovery->error = error;
	recovery->retries = 0;
	recovery->delay = TB_PCIE_RETRY_DELAY_MS;
	
	queue_work(adev->wq, &recovery->work);
}

/**
 * anarchy_pcie_tb_recovery - TB-specific PCIe recovery
 * @adev: Anarchy device structure
 *
 * This function implements TB-specific PCIe recovery logic.
 */
int anarchy_pcie_tb_recovery(struct anarchy_device *adev)
{
	int ret;
	struct pci_dev *pdev = to_pci_dev(adev->dev);
	
	if (!pdev)
		return -ENODEV;
		
	/* Implement TB-specific recovery here */
	ret = pcie_wait_link_training(pdev);
	if (ret)
		return ret;
		
	return anarchy_pcie_check_link_config(adev);
}

/**
 * anarchy_pcie_standard_recovery - Standard PCIe recovery
 * @adev: Anarchy device structure
 *
 * This function implements standard PCIe recovery logic.
 */
static int anarchy_pcie_standard_recovery(struct anarchy_device *adev)
{
	int ret = 0;
	int tries = 0;
	struct pci_dev *pdev = to_pci_dev(adev->dev);
	
	if (!pdev)
		return -ENODEV;
		
	while (tries++ < TB_PCIE_MAX_MODE_RETRIES) {
		/* Reset PCIe link */
		pcie_reset_link(pdev);
		
		/* Wait for link training */
		ret = pcie_wait_link_training(pdev);
		if (ret) {
			dev_warn(adev->dev, "Link training failed, attempt %d/%d\n",
					tries, TB_PCIE_MAX_MODE_RETRIES);
			continue;
		}
		
		/* Check link status */
		if (!pcie_link_is_up(pdev)) {
			dev_warn(adev->dev, "Link is down, attempt %d/%d\n",
					tries, TB_PCIE_MAX_MODE_RETRIES);
			msleep(TB_PCIE_RETRY_BASE_MS);
			continue;
		}
		
		/* Link is up, check configuration */
		ret = anarchy_pcie_check_link_config(adev);
		if (ret == 0)
			return 0;
			
		dev_warn(adev->dev, "Link config check failed: %d, attempt %d/%d\n",
				ret, tries, TB_PCIE_MAX_MODE_RETRIES);
		msleep(TB_PCIE_RETRY_BASE_MS);
	}
	
	return -ETIMEDOUT;
}

/**
 * anarchy_pcie_recovery_work - Work queue function for PCIe recovery
 * @work: Work structure
 *
 * This function implements the PCIe link recovery process.
 */
void anarchy_pcie_recovery_work(struct work_struct *work)
{
	struct anarchy_pcie_recovery_work *recovery = 
		container_of(work, struct anarchy_pcie_recovery_work, work);
	struct anarchy_device *adev = recovery->adev;
	int ret;

	mutex_lock(&adev->pcie.recovery_lock);

	/* Check if we're in a valid state for recovery */
	if (adev->pcie.state == TB_PCIE_OK) {
		mutex_unlock(&adev->pcie.recovery_lock);
		kfree(recovery);
		return;
	}

	/* Attempt TB-specific recovery first */
	ret = anarchy_pcie_tb_recovery(adev);
	if (ret == 0) {
		adev->pcie.state = TB_PCIE_OK;
		atomic_set(&adev->pcie.retries, 0);
		mutex_unlock(&adev->pcie.recovery_lock);
		kfree(recovery);
		return;
	}

	/* If TB recovery fails, try standard PCIe recovery */
	ret = anarchy_pcie_standard_recovery(adev);
	if (ret) {
		dev_crit(adev->dev, "All PCIe recovery attempts failed\n");
		adev->pcie.state = TB_PCIE_ERROR;
	} else {
		adev->pcie.state = TB_PCIE_OK;
		atomic_set(&adev->pcie.retries, 0);
	}

	mutex_unlock(&adev->pcie.recovery_lock);
	kfree(recovery);
}

/**
 * anarchy_pcie_check_link - Check PCIe link status
 * @adev: Anarchy device structure
 *
 * This function monitors the PCIe link status and handles any issues.
 */
void anarchy_pcie_check_link(struct anarchy_device *adev)
{
	struct pci_dev *pdev = to_pci_dev(adev->dev);
	u16 lnk_status;
	
	if (!pdev)
		return;
		
	pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
	
	if (lnk_status & PCI_EXP_LNKSTA_LT)
		anarchy_pcie_handle_error(adev, ANARCHY_PCIE_ERR_TRAINING_FAILED);
	else if (!(lnk_status & PCI_EXP_LNKSTA_DLLLA))
		anarchy_pcie_handle_error(adev, ANARCHY_PCIE_ERR_LINK_DOWN);
	else if ((lnk_status & PCI_EXP_LNKSTA_CLS) < adev->max_speed)
		anarchy_pcie_handle_error(adev, ANARCHY_PCIE_ERR_SPEED_DOWNGRADE);
	else if ((lnk_status & PCI_EXP_LNKSTA_NLW) < adev->max_lanes)
		anarchy_pcie_handle_error(adev, ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE);
}

/**
 * anarchy_pcie_check_link_config - Verify PCIe link configuration
 * @adev: Anarchy device structure
 *
 * Checks if the PCIe link is operating at the expected speed and width.
 */
int anarchy_pcie_check_link_config(struct anarchy_device *adev)
{
	u32 link_status;
	u16 link_width, link_speed;
	bool needs_retrain = false;

	/* Read link status */
	anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.pcie_cap + PCI_EXP_LNKSTA,
							4, &link_status);

	link_speed = link_status & PCI_EXP_LNKSTA_CLS;
	link_width = (link_status & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;

	/* Check if link is at expected configuration */
	if (link_speed < adev->max_speed) {
		dev_warn(adev->dev, "Link speed Gen%d below maximum Gen%d\n",
				link_speed, adev->max_speed);
		needs_retrain = true;
	}

	if (link_width < adev->max_lanes) {
		dev_warn(adev->dev, "Link width x%d below maximum x%d\n",
				link_width, adev->max_lanes);
		needs_retrain = true;
	}

	if (needs_retrain) {
		/* Try to retrain link at higher speed/width */
		anarchy_pcie_retrain_link(adev);
		msleep(TB_PCIE_LINK_SETTLE_MS);

		/* Check if retraining helped */
		anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.pcie_cap + PCI_EXP_LNKSTA,
								4, &link_status);
		
		link_speed = link_status & PCI_EXP_LNKSTA_CLS;
		link_width = (link_status & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;

		if (link_speed < adev->max_speed || link_width < adev->max_lanes) {
			dev_warn(adev->dev, "Link retraining failed to achieve optimal config\n");
			return -EAGAIN;
		}
	}

	return 0;
}

/**
 * anarchy_pcie_wait_for_link - Wait for PCIe link to stabilize
 * @adev: Anarchy device structure
 * @link_status: Pointer to store final link status
 *
 * Waits for PCIe link to complete training and become stable.
 * Returns 0 on success, negative error code on failure.
 */
int anarchy_pcie_wait_for_link(struct anarchy_device *adev, u32 *link_status)
{
	struct pci_dev *pdev = to_pci_dev(adev->dev);
	unsigned long timeout = jiffies + msecs_to_jiffies(TB_PCIE_LINK_TIMEOUT_MS);
	u16 lnk_status;
	int ret;

	if (!pdev)
		return -ENODEV;

	/* Wait for link training to complete */
	ret = pcie_wait_link_training(pdev);
	if (ret) {
		dev_err(adev->dev, "Link training timeout\n");
		return ret;
	}

	/* Wait for link to stabilize */
	while (time_before(jiffies, timeout)) {
		pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
		*link_status = lnk_status;

		if ((lnk_status & PCI_EXP_LNKSTA_DLLLA) && 
			!(lnk_status & PCI_EXP_LNKSTA_LT)) {
			dev_dbg(adev->dev, "Link stable: width x%d, speed Gen%d\n",
					(lnk_status & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT,
					lnk_status & PCI_EXP_LNKSTA_CLS);
			return 0;
		}

		msleep(TB_PCIE_RETRY_DELAY_MS);
	}

	dev_err(adev->dev, "Link failed to stabilize: status=0x%04x\n", lnk_status);
	return -ETIMEDOUT;
}

/**
 * anarchy_pcie_init - Initialize PCIe emulation
 * @adev: Anarchy device structure
 *
 * Sets up PCIe configuration space and memory regions.
 */
int anarchy_pcie_init(struct anarchy_device *adev)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;
	int ret;

	/* Initialize PCIe state */
	mutex_init(&adev->pcie.recovery_lock);
	atomic_set(&adev->pcie.retries, 0);
	adev->pcie.state = TB_PCIE_OK;
	adev->pcie.last_recovery = jiffies;

	/* Initialize config space */
	cfg->vendor_id = NVIDIA_RTX_4090_VENDOR_ID;
	cfg->device_id = NVIDIA_RTX_4090_DEVICE_ID;
	cfg->command = 0;
	cfg->status = PCI_STATUS_CAP_LIST | PCI_STATUS_66MHZ;
	cfg->revision = 0xa1;
	cfg->header_type = PCI_HEADER_TYPE_NORMAL;
	cfg->cache_line_size = 64;
	cfg->latency_timer = 0;
	cfg->bist = 0;
	
	/* Set up BARs */
	ret = anarchy_pcie_setup_bars(adev);
	if (ret)
		return ret;

	/* Set up capabilities */
	ret = anarchy_pcie_setup_capabilities(adev);
	if (ret)
		goto err_free_bars;

	/* Set up interrupts */
	cfg->interrupt_line = 0;
	cfg->interrupt_pin = 1;  /* INTA# */

	pr_info("Anarchy eGPU: PCIe emulation initialized\n");
	return 0;

err_free_bars:
	if (cfg->fb_base)
		iounmap(cfg->fb_base);
	if (cfg->mmio_base)
		iounmap(cfg->mmio_base);
	return ret;
}

/**
 * anarchy_pcie_cleanup - Clean up PCIe resources
 * @adev: Anarchy device structure
 *
 * This function cleans up PCIe resources and state.
 */
void anarchy_pcie_cleanup(struct anarchy_device *adev)
{
	struct anarchy_pcie_config *cfg = &adev->pcie_cfg;

	/* Clean up memory regions */
	if (cfg->fb_base) {
		iounmap(cfg->fb_base);
		cfg->fb_base = NULL;
	}
	if (cfg->mmio_base) {
		iounmap(cfg->mmio_base);
		cfg->mmio_base = NULL;
	}

	/* Reset PCIe state */
	adev->pcie.state = TB_PCIE_OK;
	atomic_set(&adev->pcie.retries, 0);

	/* Cancel any pending recovery work */
	mutex_lock(&adev->pcie.recovery_lock);
	mutex_unlock(&adev->pcie.recovery_lock);

	pr_info("Anarchy eGPU: PCIe resources cleaned up\n");
}

/**
 * anarchy_pcie_exit - Clean up PCIe emulation
 * @adev: Anarchy device structure
 */
void anarchy_pcie_exit(struct anarchy_device *adev)
{
	anarchy_pcie_cleanup(adev);
	pr_info("Anarchy eGPU: PCIe emulation cleaned up\n");
} 