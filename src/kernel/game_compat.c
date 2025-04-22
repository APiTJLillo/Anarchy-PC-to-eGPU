#include <linux/module.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include "include/anarchy_device.h"
#include "include/game_compat.h"
#include "include/game_compat_types.h"

struct game_memory_region *setup_game_memory_region(struct anarchy_device *adev,
                                                  size_t size, u32 flags)
{
    struct game_memory_region *region;
    int ret;

    region = kzalloc(sizeof(*region), GFP_KERNEL);
    if (!region)
        return ERR_PTR(-ENOMEM);

    region->size = size;
    region->flags = flags;

    if (flags & REGION_FLAG_COHERENT) {
        region->vaddr = dma_alloc_coherent(&adev->dev, size,
                                         &region->dma_addr, GFP_KERNEL);
        if (!region->vaddr) {
            kfree(region);
            return ERR_PTR(-ENOMEM);
        }
    } else {
        region->vaddr = kmalloc(size, GFP_KERNEL);
        if (!region->vaddr) {
            kfree(region);
            return ERR_PTR(-ENOMEM);
        }

        region->dma_addr = dma_map_single(&adev->dev, region->vaddr,
                                        size, DMA_BIDIRECTIONAL);
        if (dma_mapping_error(&adev->dev, region->dma_addr)) {
            kfree(region->vaddr);
            kfree(region);
            return ERR_PTR(-ENOMEM);
        }
    }

    return region;
}

void cleanup_game_memory_region(struct anarchy_device *adev,
                              struct game_memory_region *region)
{
    if (!region)
        return;

    if (region->flags & REGION_FLAG_COHERENT) {
        dma_free_coherent(&adev->dev, region->size,
                         region->vaddr, region->dma_addr);
    } else {
        dma_unmap_single(&adev->dev, region->dma_addr,
                        region->size, DMA_BIDIRECTIONAL);
        kfree(region->vaddr);
    }
    kfree(region);
}

int init_game_compatibility(struct anarchy_device *adev)
{
    struct game_compat_layer *compat;
    int ret;

    compat = kzalloc(sizeof(*compat), GFP_KERNEL);
    if (!compat)
        return -ENOMEM;

    /* Set up memory regions with appropriate flags */
    compat->texture_region = setup_game_memory_region(
        adev, GAME_TEXTURE_BUFFER_SIZE,
        REGION_FLAG_COHERENT | REGION_FLAG_LOWLATENCY);
    if (IS_ERR(compat->texture_region)) {
        ret = PTR_ERR(compat->texture_region);
        goto err_free_compat;
    }

    compat->command_region = setup_game_memory_region(
        adev, GAME_COMMAND_BUFFER_SIZE,
        REGION_FLAG_COHERENT | REGION_FLAG_CACHED);
    if (IS_ERR(compat->command_region)) {
        ret = PTR_ERR(compat->command_region);
        goto err_free_texture;
    }

    adev->compat_layer = compat;
    return 0;

err_free_texture:
    cleanup_game_memory_region(adev, compat->texture_region);
err_free_compat:
    kfree(compat);
    return ret;
}

int init_game_specific(struct anarchy_device *adev, const char *game_name)
{
    struct game_compat_layer *compat = adev->compat_layer;

    if (!compat)
        return -EINVAL;

    /* TODO: Load game-specific optimizations from a database or config file */
    /* For now, just return success */
    return 0;
}

void cleanup_game_compatibility(struct anarchy_device *adev)
{
    struct game_compat_layer *compat;

    if (!adev)
        return;

    compat = adev->compat_layer;
    if (!compat)
        return;

    cleanup_game_memory_region(adev, compat->texture_region);
    cleanup_game_memory_region(adev, compat->command_region);
    kfree(compat);
    adev->compat_layer = NULL;
}

int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_name)
{
    int ret;

    ret = init_game_compatibility(adev);
    if (ret)
        return ret;

    return init_game_specific(adev, game_name);
}

EXPORT_SYMBOL_GPL(init_game_compatibility);
EXPORT_SYMBOL_GPL(cleanup_game_compatibility);
EXPORT_SYMBOL_GPL(init_game_specific);
EXPORT_SYMBOL_GPL(setup_game_memory_region);
EXPORT_SYMBOL_GPL(cleanup_game_memory_region);
EXPORT_SYMBOL_GPL(anarchy_optimize_for_game);
