// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_remove_device
#include "Common.h"
#include "HyDevice.hpp"
#include "RemoveDevice.hpp"
#include "MemoryAllocator.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyRemoveDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If the context is null we can skip freeing it.
    if(MiniportDeviceContext)
    {
        // Get our context structure.
        HyMiniportDevice* deviceContext = HyMiniportDevice::FromHandle(MiniportDeviceContext);

        LOG_DEBUG("Freeing MiniportDeviceContext.\n");

        deviceContext->~HyMiniportDevice();

        // Free the context.
        // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);
        delete deviceContext;
    }

    return STATUS_SUCCESS;
}
