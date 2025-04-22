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
#include <linux/mutex.h>
#include <linux/spinlock.h>

#include "../../include/anarchy-egpu.h"
#include "../../include/anarchy-pcie.h"
#include "anarchy-debug.h"
#include "anarchy-perf.h"
#include "pcie.h"
#include <linux/device.h>

/* NVIDIA RTX 4090 PCIe configuration */
#define NVIDIA_RTX_4090_VENDOR_ID 0x10DE
#define NVIDIA_RTX_4090_DEVICE_ID 0x2684
#define NVIDIA_RTX_4090_CLASS_CODE 0x030000  /* VGA compatible controller */
#define NVIDIA_RTX_4090_SUBSYS_VENDOR_ID 0x10DE
#define NVIDIA_RTX_4090_SUBSYS_ID 0x2684

/* Memory window sizes for RTX 4090 */
#define BAR0_SIZE (16 * 1024 * 1024)     /* 16MB - MMIO */
#define BAR1_SIZE (256 * 1024 * 1024)    /* 256MB - MMIO */
#define BAR2_SIZE (4ULL * 1024 * 1024 * 1024)  /* 4GB - Lower FB */
#define BAR3_SIZE (4ULL * 1024 * 1024 * 1024)  /* 4GB - Upper FB */

/* PCIe error types */
#define ANARCHY_PCIE_ERR_TIMEOUT -ETIMEDOUT
#define ANARCHY_PCIE_ERR_LINK_DOWN -ENOLINK
#define ANARCHY_PCIE_ERR_DEVICE_NOT_FOUND -ENODEV
#define ANARCHY_PCIE_ERR_RETRY -EAGAIN
#define ANARCHY_PCIE_ERR_TRAINING_FAILED -EAGAIN
#define ANARCHY_PCIE_ERR_SPEED_DOWNGRADE -EINVAL
#define ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE -EINVAL

/* Enhanced recovery parameters */
#define ANARCHY_PCIE_MAX_RETRIES 5
#define ANARCHY_PCIE_RETRY_DELAY_MS 1000
#define ANARCHY_PCIE_LINK_TIMEOUT_MS 1000
#define ANARCHY_PCIE_TRAINING_TIMEOUT_MS 500

/* PCIe training wait queue */
wait_queue_head_t pcie_training_wait;

/* PCIe recovery work structure */
struct anarchy_pcie_recovery_work {
	struct work_struct work;
	struct anarchy_pcie_state *pcie;
	unsigned int delay;
};

/* Function declarations */
static void anarchy_pcie_recovery_work_handler(struct work_struct *work);
static int anarchy_pcie_check_link_config(struct anarchy_pcie_state *pcie);
static int anarchy_pcie_standard_recovery(struct anarchy_pcie_state *pcie);
static int anarchy_pcie_tb_recovery(struct anarchy_pcie_state *pcie);
static void update_pcie_state(struct anarchy_pcie_state *pcie,
                            enum anarchy_pcie_link_state new_state,
                            const char *reason);
static bool pcie_is_thunderbolt(struct pci_dev *pdev);

/* PCIe management functions */
void anarchy_pcie_retrain_link(struct anarchy_pcie_state *pcie);
void anarchy_pcie_handle_error(struct anarchy_pcie_state *pcie, enum anarchy_pcie_error_type error);
void anarchy_pcie_check_link(struct anarchy_pcie_state *pcie);
int anarchy_pcie_train_link(struct anarchy_pcie_state *pcie);
int anarchy_pcie_wait_for_link(struct anarchy_pcie_state *pcie, u32 *link_status);

/* Initialization and cleanup */
int anarchy_pcie_init(struct pci_dev *nhi, struct anarchy_pcie_state *pcie);
void anarchy_pcie_cleanup(struct anarchy_pcie_state *pcie);
void anarchy_pcie_exit(struct anarchy_pcie_state *pcie);

/* PCIe capability flags */
#define PCI_EXP_FLAGS_VER2 PCI_EXP_FLAGS_VERS /* PCIe 2.0 */
#define PCI_EXP_FLAGS_TYPE_RC (PCI_EXP_TYPE_ROOT_PORT << 4) /* Root Complex */
#define PCI_EXP_DEVCAP_PAYLOAD_512B PCI_EXP_DEVCTL_PAYLOAD_512B /* 512B max payload */

/* PCIe link status helpers */
bool pcie_link_is_up(struct pci_dev *pdev)
{
	u16 lnk_status;
	int ret;

	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
	if (ret)
		return false;

	return !!(lnk_status & PCI_EXP_LNKSTA_DLLLA);
}

void pcie_reset_link(struct pci_dev *pdev)
{
	u16 lnk_ctrl;
	int ret;

	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
	if (ret)
		return;

	lnk_ctrl |= PCI_EXP_LNKCTL_RL;
	pcie_capability_write_word(pdev, PCI_EXP_LNKCTL, lnk_ctrl);
}

/* PCIe recovery work handler */
static void anarchy_pcie_recovery_work_handler(struct work_struct *work)
{
	struct anarchy_pcie_recovery_work *recovery = 
		container_of(work, struct anarchy_pcie_recovery_work, work);
	struct anarchy_pcie_state *pcie = recovery->pcie;
	struct anarchy_device *adev = to_anarchy_device(pcie);
	int ret;

	spin_lock(&pcie->recovery_lock);

	if (pcie->state != ANARCHY_PCIE_STATE_RECOVERY) {
		spin_unlock(&pcie->recovery_lock);
		return;
	}

	ret = anarchy_pcie_standard_recovery(pcie);
	if (ret) {
		dev_err(&pcie->nhi->dev, "PCIe recovery failed: %d\n", ret);
    pcie->state = ANARCHY_PCIE_STATE_ERROR;
		spin_unlock(&pcie->recovery_lock);
		return;
	}

    pcie->state = ANARCHY_PCIE_STATE_NORMAL;
	spin_unlock(&pcie->recovery_lock);
}

/* Standard PCIe recovery attempt */
static int anarchy_pcie_standard_recovery(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;
	int timeout = ANARCHY_PCIE_LINK_TIMEOUT_MS;
	int ret;

	if (!pdev)
		return -EINVAL;

	/* Reset the PCIe link */
	pcie_reset_link(pdev);

	/* Wait for link to stabilize */
	msleep(100);

	/* Check if link is up */
	while (!pcie_link_is_up(pdev) && timeout > 0) {
		msleep(10);
		timeout -= 10;
	}

	if (timeout <= 0)
		return -ETIMEDOUT;

	/* Re-enable device */
	ret = pci_enable_device(pdev);
	if (ret)
		return ret;

	/* Restore PCIe configuration */
	ret = anarchy_pcie_setup_capabilities(pcie);
	if (ret)
		return ret;

	return 0;
}

/**
 * anarchy_pcie_setup_bars - Initialize Base Address Registers
 * @pcie: PCIe state structure
 *
 * Sets up the BAR configuration for the emulated GPU.
 */
int anarchy_pcie_setup_bars(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;
	int i;

	if (!pdev)
		return -EINVAL;

	for (i = 0; i < 6; i++) {
		if (!pci_resource_len(pdev, i))
			continue;

		pcie->config.bar[i] = pci_resource_start(pdev, i);
		/* bar_size is not in the config structure, skip it */
	}

	return 0;
}

/**
 * anarchy_pcie_setup_capabilities - Initialize PCIe capabilities
 * @pcie: PCIe state structure
 */
int anarchy_pcie_setup_capabilities(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;
	int ret;

	if (!pdev)
		return -EINVAL;

	/* Enable bus mastering */
	pci_set_master(pdev);

	/* Setup memory regions */
	ret = pci_request_regions(pdev, "anarchy-gpu");
	if (ret) {
		dev_err(&pdev->dev, "Failed to request PCI regions: %d\n", ret);
		return ret;
	}

	/* Enable PCI Express capability */
	ret = pcie_capability_set_word(pdev, PCI_EXP_DEVCTL, 
								 PCI_EXP_DEVCTL_RELAX_EN | 
								 PCI_EXP_DEVCTL_NOSNOOP_EN);
	if (ret) {
		dev_err(&pdev->dev, "Failed to set PCIe device control: %d\n", ret);
		goto err_release;
	}

	/* Set up PCIe configuration */
	pcie->config.vendor_id = pdev->vendor;
	pcie->config.device_id = pdev->device;
	pcie->config.command = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	pcie->config.status = 0;

	/* Map BARs */
	ret = anarchy_pcie_setup_bars(pcie);
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup BARs: %d\n", ret);
		goto err_release;
	}

	return 0;

err_release:
	pci_release_regions(pdev);
	return ret;
}

/**
 * anarchy_pcie_read_config - Read from configuration space
 * @pcie: PCIe state structure
 * @where: Register offset
 * @size: Access size (1, 2, or 4 bytes)
 * @val: Value read
 */
int anarchy_pcie_read_config(struct anarchy_pcie_state *pcie, int where, int size, u32 *val)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;

	if (!pcie || !pdev || !val)
		return -EINVAL;

	switch (size) {
	case 1:
		return pci_read_config_byte(pdev, where, (u8 *)val);
	case 2:
		return pci_read_config_word(pdev, where, (u16 *)val);
	case 4:
		return pci_read_config_dword(pdev, where, val);
	default:
		return -EINVAL;
	}
}

/**
 * anarchy_pcie_write_config - Write to configuration space
 * @pcie: PCIe state structure
 * @where: Register offset
 * @size: Access size (1, 2, or 4 bytes)
 * @val: Value to write
 */
int anarchy_pcie_write_config(struct anarchy_pcie_state *pcie, int where, int size, u32 val)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;

	if (!pcie || !pdev)
		return -EINVAL;

	switch (size) {
	case 1:
		return pci_write_config_byte(pdev, where, val);
	case 2:
		return pci_write_config_word(pdev, where, val);
	case 4:
		return pci_write_config_dword(pdev, where, val);
	default:
		return -EINVAL;
	}
}

/**
 * anarchy_pcie_retrain_link - Handle link retraining
 * @pcie: PCIe state structure
 */
void anarchy_pcie_retrain_link(struct anarchy_pcie_state *pcie)
{
    struct anarchy_device *adev;

    if (!pcie || !pcie->nhi)
        return;

    adev = to_anarchy_device(pcie);
    if (!adev)
        return;

    spin_lock(&pcie->recovery_lock);

    if (pcie->state != ANARCHY_PCIE_STATE_NORMAL) {
        spin_unlock(&pcie->recovery_lock);
        return;
    }

    pcie->state = ANARCHY_PCIE_STATE_RECOVERY;
    pcie->last_state_change = jiffies;

    /* Schedule recovery work */
    schedule_work(&pcie->recovery_work);

    spin_unlock(&pcie->recovery_lock);
}

/**
 * anarchy_pcie_handle_error - Handle PCIe error
 * @pcie: PCIe state structure
 * @error: Error code
 */
void anarchy_pcie_handle_error(struct anarchy_pcie_state *pcie, enum anarchy_pcie_error_type error)
{
	struct anarchy_device *adev;
	struct anarchy_pcie_recovery_work *recovery;
	
	if (!pcie)
		return;

	adev = to_anarchy_device(pcie);
	if (!adev)
		return;

	recovery = kzalloc(sizeof(*recovery), GFP_KERNEL);
	if (!recovery)
		return;

	recovery->pcie = pcie;
	recovery->delay = ANARCHY_PCIE_RETRY_DELAY_MS;
	INIT_WORK(&recovery->work, anarchy_pcie_recovery_work_handler);

	spin_lock(&pcie->recovery_lock);
	pcie->state = ANARCHY_PCIE_STATE_ERROR;
	pcie->last_recovery = jiffies;
	spin_unlock(&pcie->recovery_lock);
	
	schedule_work(&recovery->work);
}

/**
 * anarchy_pcie_tb_recovery - TB-specific PCIe recovery
 * @pcie: PCIe state structure
 */
static int anarchy_pcie_tb_recovery(struct anarchy_pcie_state *pcie)
{
	/* This function will be implemented in thunderbolt.c */
	return -ENOSYS;
}

/**
 * anarchy_pcie_check_link - Check PCIe link status
 * @pcie: PCIe state structure
 */
void anarchy_pcie_check_link(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev;
	u32 link_status;
	int ret;

	if (!pcie || !adev || !adev->nhi)
		return;

	pdev = adev->nhi;

	ret = anarchy_pcie_wait_for_link(pcie, &link_status);
	if (ret || !pcie_link_is_up(pdev)) {
		pr_err("PCIe link check failed, initiating recovery\n");
		anarchy_pcie_handle_error(pcie, ANARCHY_PCIE_ERROR_LINK_DOWN);
	}
}

/**
 * anarchy_pcie_check_link_config - Verify PCIe link configuration
 * @pcie: PCIe state structure
 */
int anarchy_pcie_check_link_config(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;
	u16 lnk_status;
	int ret;

	if (!pdev)
		return -EINVAL;

	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
	if (ret)
		return ret;

	pcie->config.caps.link_status = lnk_status;

	if ((lnk_status & PCI_EXP_LNKSTA_CLS) < (pcie->config.caps.link_cap & PCI_EXP_LNKCAP_SLS) ||
		((lnk_status & PCI_EXP_LNKSTA_NLW) >> 4) < ((pcie->config.caps.link_cap & PCI_EXP_LNKCAP_MLW) >> 4)) {
		pr_warn("Link running at reduced speed or width\n");
		return -EIO;
	}

	return 0;
}

/**
 * anarchy_pcie_wait_for_link - Wait for PCIe link to stabilize
 * @pcie: PCIe state structure
 * @link_status: Pointer to store final link status
 */
int anarchy_pcie_wait_for_link(struct anarchy_pcie_state *pcie, u32 *link_status)
{
struct anarchy_device *adev = to_anarchy_device(pcie);
struct pci_dev *pdev = adev->nhi;
int timeout = ANARCHY_PCIE_LINK_TIMEOUT_MS;
	u16 lnk_status;

	if (!pdev || !link_status)
		return -EINVAL;

	while (timeout > 0) {
		pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
		*link_status = lnk_status;

		if (!(lnk_status & PCI_EXP_LNKSTA_LT) && (lnk_status & PCI_EXP_LNKSTA_DLLLA))
			return 0;

		msleep(1);
		timeout--;
	}

	return -ETIMEDOUT;
}

/**
 * anarchy_pcie_train_link - Train PCIe link to maximum capabilities
 * @pcie: PCIe state structure
 */
int anarchy_pcie_train_link(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;
	u16 lnk_status, lnk_ctrl;
	int ret;

	if (!pdev)
		return -EINVAL;

	/* Read current link status and control */
	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
	if (ret)
		return ret;

	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
	if (ret)
		return ret;

	/* Set target speed and width */
	lnk_ctrl &= ~PCI_EXP_LNKCTL_ASPMC;
	lnk_ctrl |= (pcie->config.caps.link_cap & PCI_EXP_LNKCAP_SLS);
	
	ret = pcie_capability_write_word(pdev, PCI_EXP_LNKCTL, lnk_ctrl);
	if (ret)
		return ret;

	/* Initiate link training */
	ret = pcie_wait_link_training(pdev);
	if (ret) {
		pr_err("Link training timeout\n");
		return ret;
	}

	/* Verify link status */
	ret = anarchy_pcie_check_link_config(pcie);
	if (ret) {
		pr_err("Link configuration check failed after training\n");
		return ret;
	}

	return 0;
}

/**
 * update_pcie_state - Update PCIe state with logging
 * @pcie: PCIe state structure
 * @new_state: New PCIe state
 * @reason: Reason for state change
 */
void update_pcie_state(struct anarchy_pcie_state *pcie,
                     enum anarchy_pcie_link_state new_state,
                     const char *reason)
{
    struct anarchy_device *adev = to_anarchy_device(pcie);
    struct device *dev;

    if (!adev || !adev->nhi)
        return;

    dev = &adev->nhi->dev;

    if (pcie->state != new_state) {
        dev_info(dev, "PCIe state change: %d -> %d (%s)\n",
                pcie->state, new_state, reason);
        pcie->state = new_state;
        pcie->last_state_change = jiffies;
    }
}

/**
 * anarchy_pcie_init - Initialize PCIe state
 * @nhi: PCI device
 * @pcie: PCIe state structure
 */
int anarchy_pcie_init(struct pci_dev *nhi, struct anarchy_pcie_state *pcie)
{
	if (!nhi || !pcie)
		return -EINVAL;

	pcie->nhi = nhi;
	pcie->state = ANARCHY_PCIE_STATE_UNKNOWN;
	atomic_set(&pcie->retries, 0);
	spin_lock_init(&pcie->recovery_lock);
INIT_WORK(&pcie->recovery_work, anarchy_pcie_recovery_work_handler);

	return 0;
}

/**
 * anarchy_pcie_cleanup - Clean up PCIe resources
 * @pcie: PCIe state structure
 */
void anarchy_pcie_cleanup(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev;

	if (!pcie)
		return;

	adev = container_of(pcie, struct anarchy_device, pcie);
	cancel_work_sync(&adev->recovery_work);
	
	if (pcie->config.mmio_base)
		iounmap(pcie->config.mmio_base);
	if (pcie->config.mmio_base2)
		iounmap(pcie->config.mmio_base2);
	if (pcie->config.fb_base)
		iounmap(pcie->config.fb_base);

	memset(pcie, 0, sizeof(*pcie));
}

/**
 * anarchy_pcie_exit - Clean up and exit PCIe driver
 * @pcie: PCIe state structure
 */
void anarchy_pcie_exit(struct anarchy_pcie_state *pcie)
{
	if (!pcie)
		return;

	anarchy_pcie_disable(pcie);
	anarchy_pcie_cleanup(pcie);
}

/* Stub functions for GPU features - implement these in gpu.c */
int anarchy_gpu_init_features(struct anarchy_device *adev)
{
	/* TODO: Implement in gpu.c */
	return 0;
}

void anarchy_gpu_cleanup_features(struct anarchy_device *adev)
{
	/* TODO: Implement in gpu.c */
}

/**
 * anarchy_pcie_enable - Enable PCIe link
 * @pcie: PCIe state structure
 */
int anarchy_pcie_enable(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev;
	int ret;

	if (!pcie || !adev || !adev->nhi)
		return -EINVAL;

	pdev = adev->nhi;

	ret = pci_enable_device(pdev);
	if (ret) {
		pr_err("Failed to enable PCIe device\n");
		return ret;
	}

	pci_set_master(pdev);
	
	spin_lock(&pcie->recovery_lock);
	pcie->state = ANARCHY_PCIE_STATE_NORMAL;
	spin_unlock(&pcie->recovery_lock);

	return 0;
}

/**
 * anarchy_pcie_disable - Disable PCIe link
 * @pcie: PCIe state structure
 */
void anarchy_pcie_disable(struct anarchy_pcie_state *pcie)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev;

	if (!pcie || !adev || !adev->nhi)
		return;

	pdev = adev->nhi;

	spin_lock(&pcie->recovery_lock);
	pcie->state = ANARCHY_PCIE_STATE_LINK_DOWN;
	spin_unlock(&pcie->recovery_lock);

	pci_clear_master(pdev);
	pci_disable_device(pdev);
}

int anarchy_pcie_set_link_control(struct anarchy_pcie_state *pcie, u32 control_bits)
{
	struct anarchy_device *adev = to_anarchy_device(pcie);
	struct pci_dev *pdev = adev->nhi;
	u16 lnk_ctrl;
	
	if (!pcie || !pdev)
		return -EINVAL;

	pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
	lnk_ctrl |= control_bits;
	pcie_capability_write_word(pdev, PCI_EXP_LNKCTL, lnk_ctrl);

	return 0;
}

int pcie_wait_link_training(struct pci_dev *pdev)
{
	u16 lnk_status;
	int timeout = ANARCHY_PCIE_TRAINING_TIMEOUT_MS;

	if (!pdev)
		return -EINVAL;

	while (timeout > 0) {
		pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
		if (!(lnk_status & PCI_EXP_LNKSTA_LT))
			return 0;
		msleep(1);
		timeout--;
	}

	return -ETIMEDOUT;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("PCIe Management for eGPU Driver");

/* Helper function to check if device is behind Thunderbolt */
static bool pcie_is_thunderbolt(struct pci_dev *pdev)
{
	struct pci_dev *parent = pdev;
	
	while (parent) {
		if (parent->vendor == PCI_VENDOR_ID_INTEL &&
			(parent->device == 0x1575 || /* Alpine Ridge */
			 parent->device == 0x15c0 || /* Titan Ridge */
			 parent->device == 0x8a1d || /* Maple Ridge */
			 parent->device == 0x1578))  /* Alpine Ridge LP */
			return true;
		parent = parent->bus->self;
	}
	
	return false;
}
