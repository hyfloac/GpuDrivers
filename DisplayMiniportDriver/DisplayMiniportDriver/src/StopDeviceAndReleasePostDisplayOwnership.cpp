// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_stop_device_and_release_post_display_ownership
#include "Common.h"
#include "HyDevice.hpp"
#include "StopDeviceAndReleasePostDisplayOwnership.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyStopDeviceAndReleasePostDisplayOwnership(IN_CONST_PVOID MiniportDeviceContext, IN_CONST_D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId, PDXGK_DISPLAY_INFORMATION DisplayInfo)
{
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

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(MiniportDeviceContext);

    return deviceContext->StopDeviceAndReleasePostDisplayOwnership(TargetId, DisplayInfo);
}
