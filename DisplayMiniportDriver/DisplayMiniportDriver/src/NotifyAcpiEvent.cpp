// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_notify_acpi_event
#include "Common.h"
#include "NotifyAcpiEvent.hpp"
#include "Logging.h"

#pragma code_seg("PAGE_K")

NTSTATUS HyNotifyAcpiEvent(IN_CONST_PVOID MiniportDeviceContext, IN_DXGK_EVENT_TYPE EventType, IN_ULONG Event, IN_PVOID Argument, OUT_PULONG AcpiFlags)
{
    (void) MiniportDeviceContext;
    (void) EventType;
    (void) Event;
    (void) Argument;
    (void) AcpiFlags;

    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    return STATUS_NOT_IMPLEMENTED;
}
