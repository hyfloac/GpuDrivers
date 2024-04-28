// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_dpc_routine
#ifdef __cplusplus
extern "C" {
#endif

#include "DpcRoutine.h"
#include "Logging.h"

#pragma code_seg("_KTEXT")

void HyDpcRoutine(IN_CONST_PVOID MiniportDeviceContext)
{
    (void) MiniportDeviceContext;

    CHECK_IRQL(DISPATCH_LEVEL);

    LOG_DEBUG("HyDpcRoutine\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyDpcRoutine: MiniportDeviceContext\n");
        return;
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif
