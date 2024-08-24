// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_createdepthstencilstate
#include "d3d10/device/CreateDepthStencilState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsCreateDepthStencilState10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_DEPTH_STENCIL_DESC* const pDepthStencilDesc,
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    const D3D10DDI_HRTDEPTHSTENCILSTATE hRtDepthStencilState
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, pDepthStencilDesc: 0x{XP0}, hDepthStencilState: 0x{XP0}, hRtDepthStencilState: 0x{XP0}",
        hDevice.pDrvPrivate,
        pDepthStencilDesc,
        hDepthStencilState.pDrvPrivate,
        hRtDepthStencilState.handle
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->CreateDepthStencilState(
        pDepthStencilDesc, 
        hDepthStencilState, 
        hRtDepthStencilState
    );
}

