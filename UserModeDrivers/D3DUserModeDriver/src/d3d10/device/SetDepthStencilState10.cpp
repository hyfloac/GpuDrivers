// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_setdepthstencilstate
// See https://learn.microsoft.com/en-us/windows/win32/api/d3d10/nf-d3d10-id3d10device-omsetdepthstencilstate
#include "d3d10/device/SetDepthStencilState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsSetDepthStencilState10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    const UINT StencilRef
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, hDepthStencilState: 0x{XP0}, StencilRef: 0x{XP0}",
        hDevice.pDrvPrivate,
        hDepthStencilState.pDrvPrivate,
        StencilRef
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->SetDepthStencilState(
        hDepthStencilState,
        StencilRef
    );
}
