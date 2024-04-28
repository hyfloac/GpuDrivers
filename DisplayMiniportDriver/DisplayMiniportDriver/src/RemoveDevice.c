// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_remove_device
#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "RemoveDevice.h"
#include "MemoryAllocator.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyRemoveDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    PAGED_CODE();

    LOG_DEBUG("HyRemoveDevice\n");

    // If the context is null we can skip freeing it.
    if(MiniportDeviceContext)
    {
        // Get our context structure.
        HyMiniportDeviceContext* deviceContext = MiniportDeviceContext;

        LOG_DEBUG("HyRemoveDevice: Freeing MiniportDeviceContext.\n");

        // Free the context.
        HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);
    }

    return STATUS_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
