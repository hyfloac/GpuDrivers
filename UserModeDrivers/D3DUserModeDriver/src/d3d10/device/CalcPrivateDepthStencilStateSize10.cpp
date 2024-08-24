// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_calcprivateblendstatesize
#include "d3d10/device/CalcPrivateDepthStencilStateSize10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

SIZE_T APIENTRY GsCalcPrivateDepthStencilStateSize10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_DEPTH_STENCIL_DESC* const pDepthStencilState
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, pDepthStencilState: 0x{XP0}",
        hDevice.pDrvPrivate,
        pDepthStencilState
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return 0;
    }

    if(!pDepthStencilState)
    {
        LOG_WARN(u8"pDepthStencilState was not set.");
        // return 0;
    }

    return GsDevice10::FromHandle(hDevice)->CalcPrivateDepthStencilStateSize(
        pDepthStencilState
    );
}
