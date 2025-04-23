#include <linux/module.h>
#include <linux/device.h>
#include "include/anarchy_device.h"
#include "include/common.h"
#include "include/pcie_forward.h"
#include "include/pcie_state.h"
#include "include/pcie_types.h"

int anarchy_device_init(struct anarchy_device *adev)
{
    int ret;

    if (!adev)
        return -EINVAL;

    /* Initialize device state */
    adev->state = ANARCHY_DEVICE_STATE_INITIALIZING;
    adev->flags = 0;
    mutex_init(&adev->lock);

    /* Initialize workqueue */
    adev->wq = create_singlethread_workqueue("anarchy-device");
    if (!adev->wq)
        return -ENOMEM;

    /* Initialize PCIe subsystem */
    ret = anarchy_pcie_init(adev);
    if (ret)
        goto err_wq;

    /* Initialize ring buffers */
    ret = anarchy_ring_init(adev, &adev->tx_ring);
    if (ret)
        goto err_pcie;

    ret = anarchy_ring_init(adev, &adev->rx_ring);
    if (ret)
        goto err_tx_ring;

    /* Initialize performance monitoring */
    ret = anarchy_perf_init(adev);
    if (ret)
        goto err_rx_ring;

    /* Initialize power management */
    ret = anarchy_power_init(adev);
    if (ret)
        goto err_perf;

    /* Start performance monitoring */
    ret = anarchy_perf_start(adev);
    if (ret)
        goto err_power;

    /* Train PCIe link */
    ret = anarchy_pcie_train_link(adev);
    if (ret)
        goto err_perf_start;

    /* Start ring buffers */
    ret = anarchy_ring_start(adev, &adev->tx_ring, true);
    if (ret)
        goto err_pcie_train;

    ret = anarchy_ring_start(adev, &adev->rx_ring, false);
    if (ret)
        goto err_tx_start;

    adev->state = ANARCHY_DEVICE_STATE_READY;
    adev->flags |= ANARCHY_DEVICE_FLAG_INITIALIZED;
    return 0;

err_tx_start:
    anarchy_ring_stop(adev, &adev->tx_ring);
err_pcie_train:
    anarchy_pcie_disable(adev);
err_perf_start:
    anarchy_perf_stop(adev);
err_power:
    anarchy_power_exit(adev);
err_perf:
    anarchy_perf_exit(adev);
err_rx_ring:
    anarchy_ring_cleanup(adev, &adev->rx_ring);
err_tx_ring:
    anarchy_ring_cleanup(adev, &adev->tx_ring);
err_pcie:
    anarchy_pcie_exit(adev);
err_wq:
    destroy_workqueue(adev->wq);
    return ret;
}
EXPORT_SYMBOL_GPL(anarchy_device_init);

void anarchy_device_exit(struct anarchy_device *adev)
{
    if (!adev)
        return;

    /* Stop services */
    anarchy_perf_stop(adev);
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);
    anarchy_pcie_disable(adev);

    /* Cleanup subsystems */
    anarchy_power_exit(adev);
    anarchy_perf_exit(adev);
    anarchy_ring_cleanup(adev, &adev->rx_ring);
    anarchy_ring_cleanup(adev, &adev->tx_ring);
    anarchy_pcie_exit(adev);

    /* Cleanup device */
    if (adev->wq)
        destroy_workqueue(adev->wq);

    adev->state = ANARCHY_DEVICE_STATE_UNINITIALIZED;
    adev->flags &= ~ANARCHY_DEVICE_FLAG_INITIALIZED;
}
EXPORT_SYMBOL_GPL(anarchy_device_exit);

int anarchy_device_connect(struct anarchy_device *adev)
{
    int ret;

    if (!adev)
        return -EINVAL;

    mutex_lock(&adev->lock);

    /* Train PCIe link */
    ret = anarchy_pcie_train_link(adev);
    if (ret)
        goto unlock;

    /* Start ring buffers */
    ret = anarchy_ring_start(adev, &adev->tx_ring, true);
    if (ret)
        goto disable_pcie;

    ret = anarchy_ring_start(adev, &adev->rx_ring, false);
    if (ret)
        goto stop_tx;

    adev->flags |= ANARCHY_DEVICE_FLAG_CONNECTED;
    mutex_unlock(&adev->lock);
    return 0;

stop_tx:
    anarchy_ring_stop(adev, &adev->tx_ring);
disable_pcie:
    anarchy_pcie_disable(adev);
unlock:
    mutex_unlock(&adev->lock);
    return ret;
}

void anarchy_device_disconnect(struct anarchy_device *adev)
{
    if (!adev)
        return;

    mutex_lock(&adev->lock);

    /* Stop everything */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);
    anarchy_pcie_disable(adev);

    adev->flags &= ~ANARCHY_DEVICE_FLAG_CONNECTED;
    mutex_unlock(&adev->lock);
}

int anarchy_device_suspend(struct anarchy_device *adev)
{
    int ret = 0;

    if (!adev)
        return -EINVAL;

    mutex_lock(&adev->lock);

    /* Stop performance monitoring */
    anarchy_perf_stop(adev);

    /* Stop ring buffers */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);

    /* Disable PCIe link */
    anarchy_pcie_disable(adev);

    adev->flags |= ANARCHY_DEVICE_FLAG_SUSPENDED;
    mutex_unlock(&adev->lock);
    return ret;
}
EXPORT_SYMBOL_GPL(anarchy_device_suspend);

int anarchy_device_resume(struct anarchy_device *adev)
{
    int ret;

    if (!adev)
        return -EINVAL;

    mutex_lock(&adev->lock);

    /* Train PCIe link */
    ret = anarchy_pcie_train_link(adev);
    if (ret)
        goto unlock;

    /* Start ring buffers */
    ret = anarchy_ring_start(adev, &adev->tx_ring, true);
    if (ret)
        goto disable_pcie;

    ret = anarchy_ring_start(adev, &adev->rx_ring, false);
    if (ret)
        goto stop_tx;

    /* Resume performance monitoring */
    ret = anarchy_perf_start(adev);
    if (ret)
        goto stop_rx;

    adev->flags &= ~ANARCHY_DEVICE_FLAG_SUSPENDED;
    mutex_unlock(&adev->lock);
    return 0;

stop_rx:
    anarchy_ring_stop(adev, &adev->rx_ring);
stop_tx:
    anarchy_ring_stop(adev, &adev->tx_ring);
disable_pcie:
    anarchy_pcie_disable(adev);
unlock:
    mutex_unlock(&adev->lock);
    return ret;
}
EXPORT_SYMBOL_GPL(anarchy_device_resume);
