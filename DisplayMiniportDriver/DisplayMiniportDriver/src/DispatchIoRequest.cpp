// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_dispatch_io_request

#include "Common.h"
#include "DispatchIoRequest.hpp"
#include "Logging.h"

NTSTATUS HyDispatchIoRequest(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG VidPnSourceId, IN_PVIDEO_REQUEST_PACKET VideoRequestPacket)
{
    (void) MiniportDeviceContext;
    (void) VidPnSourceId;
    (void) VideoRequestPacket;
        
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If VideoRequestPacket is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!VideoRequestPacket)
    {
        LOG_ERROR("Invalid Parameter: VideoRequestPacket\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    VideoRequestPacket->StatusBlock->Status = STATUS_NOT_SUPPORTED;

    return STATUS_NOT_SUPPORTED;
}
