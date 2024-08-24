// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_checkmultisamplequalitylevels
// See https://learn.microsoft.com/en-us/windows/win32/api/d3d10/nf-d3d10-id3d10device-checkmultisamplequalitylevels
#include "d3d10/device/CheckMultisampleQualityLevels10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsCheckMultisampleQualityLevels10(
    const D3D10DDI_HDEVICE hDevice,
    const DXGI_FORMAT Format,
    const UINT SampleCount,
    UINT* const pNumQualityLevels
)
{
    if constexpr(false)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hDevice: 0x{XP0}, Format: {}, SampleCount: {}, pNumQualityLevels: 0x{XP0}",
            hDevice.pDrvPrivate,
            Format,
            SampleCount,
            pNumQualityLevels
        );
    }

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->CheckMultisampleQualityLevels(
        Format,
        SampleCount,
        pNumQualityLevels
    );
}
