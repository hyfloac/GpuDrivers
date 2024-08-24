// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_destroydepthstencilstate
#include "d3d10/device/DestroyDepthStencilState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsDestroyDepthStencilState10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, hDepthStencilState: 0x{XP0}",
        hDevice.pDrvPrivate,
        hDepthStencilState.pDrvPrivate
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->DestroyDepthStencilState(
        hDepthStencilState
    );
}
