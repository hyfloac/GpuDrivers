// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_child_status
#include "Common.h"
#include "QueryChildStatus.hpp"
#include "HyDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryChildStatus(IN_CONST_PVOID MiniportDeviceContext, INOUT_PDXGK_CHILD_STATUS ChildStatus, IN_BOOLEAN NonDestructiveOnly)
{
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If ChildStatus is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!ChildStatus)
    {
        LOG_ERROR("Invalid Parameter: ChildStatus\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(MiniportDeviceContext);

    return deviceContext->QueryChildStatus(ChildStatus, NonDestructiveOnly);
}
