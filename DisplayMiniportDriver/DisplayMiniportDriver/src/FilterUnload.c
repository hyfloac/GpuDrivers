// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nc-fltkernel-pflt_filter_unload_callback
#include <ntddk.h>
#include <fltKernel.h>

#include "FilterUnload.h"

#pragma code_seg("PAGE")

NTSTATUS HyFilterUnload(const FLT_FILTER_UNLOAD_FLAGS Flags)
{
    PAGED_CODE();

    // The filter can be unloaded.
    return STATUS_SUCCESS;
}
