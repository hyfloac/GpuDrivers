// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_remove_device
#include "HyDevice.hpp"
#include "RemoveDevice.hpp"
#include "MemoryAllocator.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyRemoveDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyRemoveDevice\n");

    // If the context is null we can skip freeing it.
    if(MiniportDeviceContext)
    {
        // Get our context structure.
        HyMiniportDevice* deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

        LOG_DEBUG("HyRemoveDevice: Freeing MiniportDeviceContext.\n");

        deviceContext->~HyMiniportDevice();

        // Free the context.
        HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);
    }

    return STATUS_SUCCESS;
}
