// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nc-fltkernel-pflt_instance_query_teardown_callback
// #include <ntddk.h>
#include <fltKernel.h>

#include "FilterInstanceQueryTeardown.h"

#pragma code_seg("PAGE")

NTSTATUS HyFilterInstanceQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags)
{
    (void) FltObjects;
    (void) Flags;

    PAGED_CODE();

    // The user application is allowed to manually detach.
    return STATUS_SUCCESS;
}
