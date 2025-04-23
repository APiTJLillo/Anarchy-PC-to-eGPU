#ifndef ANARCHY_ENDPOINT_H
#define ANARCHY_ENDPOINT_H

/* PCIe endpoint configuration */
#define ENDPOINT_VENDOR_ID NVIDIA_RTX_4090_VENDOR_ID
#define ENDPOINT_DEVICE_ID NVIDIA_RTX_4090_DEVICE_ID
#define ENDPOINT_CLASS_CODE PCI_CLASS_VGA

/* Memory window configuration */
#define ENDPOINT_BAR0_SIZE (16 * 1024 * 1024)   /* 16MB MMIO */
#define ENDPOINT_BAR1_SIZE (256 * 1024 * 1024)  /* 256MB FB */
#define ENDPOINT_BAR3_SIZE (32 * 1024)          /* 32KB Config */

/* Endpoint capabilities */
#define ENDPOINT_MAX_PAYLOAD_SIZE 256
#define ENDPOINT_MAX_READ_REQUEST_SIZE 512

/* Endpoint state flags */
#define EP_STATE_INITIALIZED  BIT(0)
#define EP_STATE_LINKED      BIT(1)
#define EP_STATE_ENABLED     BIT(2)
#define EP_STATE_ERROR       BIT(3)

struct anarchy_endpoint_config {
    u32 state;
    u32 max_link_speed;
    u32 max_link_width;
    u32 current_link_speed;
    u32 current_link_width;
    void __iomem *mmio_base;
    void __iomem *fb_base;
    dma_addr_t mmio_phys;
    dma_addr_t fb_phys;
    struct mutex lock;
};

int anarchy_endpoint_init(struct anarchy_device *adev);
void anarchy_endpoint_exit(struct anarchy_device *adev);
int anarchy_endpoint_enable(struct anarchy_device *adev);
void anarchy_endpoint_disable(struct anarchy_device *adev);

#endif /* ANARCHY_ENDPOINT_H */
