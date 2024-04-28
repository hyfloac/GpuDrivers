// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_child_status
#ifdef __cplusplus
extern "C" {
#endif

#include "QueryChildStatus.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryChildStatus(IN_CONST_PVOID MiniportDeviceContext, INOUT_PDXGK_CHILD_STATUS ChildStatus, IN_BOOLEAN NonDestructiveOnly)
{
    (void) MiniportDeviceContext;
    (void) ChildStatus;
    (void) NonDestructiveOnly;

    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyQueryChildStatus\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyQueryChildStatus: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If ChildStatus is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!ChildStatus)
    {
        LOG_ERROR("Invalid Parameter to HyQueryChildStatus: ChildStatus\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    return STATUS_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
