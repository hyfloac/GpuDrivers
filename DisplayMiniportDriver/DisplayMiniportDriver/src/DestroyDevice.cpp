// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_destroydevice
#include "Common.h"
#include "DestroyDevice.hpp"
#include "GsLogicalDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyDestroyDevice(IN_CONST_HANDLE hDevice)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If hDevice is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hDevice)
    {
        LOG_ERROR("Invalid Parameter: hDevice\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    GsLogicalDevice* const logicalDevice = GsLogicalDevice::FromHandle(hDevice);

    delete logicalDevice;

    return STATUS_SUCCESS;
}
