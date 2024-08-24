// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_calcprivateblendstatesize
#include "d3d10/device/CalcPrivateBlendStateSize10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

SIZE_T APIENTRY GsCalcPrivateBlendStateSize10(D3D10DDI_HDEVICE hDevice, const D3D10_DDI_BLEND_DESC* pBlendDesc)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hDevice: 0x{XP0}, pBlendDesc: 0x{XP0}",
            hDevice.pDrvPrivate,
            pBlendDesc
        );
    }
#endif

    if(!hDevice.pDrvPrivate)
    {
        return 0;
    }

    // if(!pBlendDesc)
    // {
    //     return 0;
    // }

    return GsDevice10::FromHandle(hDevice)->CalcPrivateBlendStateSize(pBlendDesc);
}
