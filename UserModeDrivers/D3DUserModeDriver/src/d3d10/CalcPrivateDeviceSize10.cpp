// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_calcprivatedevicesize
#include "d3d10/CalcPrivateDeviceSize10.hpp"
#include "d3d10/GsAdapter10.hpp"
#include "Logging.hpp"

SIZE_T GsCalcPrivateDeviceSizeD3D10(D3D10DDI_HADAPTER hAdapter, const D3D10DDIARG_CALCPRIVATEDEVICESIZE* const pCalcPrivateDeviceSize)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"Interface: 0x{X}, Version: 0x{X}, Flags: 0x{XP0}",
            pCalcPrivateDeviceSize->Interface,
            pCalcPrivateDeviceSize->Version,
            pCalcPrivateDeviceSize->Flags
        );
    }
#endif

    if(!hAdapter.pDrvPrivate)
    {
        return 0;
    }

    if(!pCalcPrivateDeviceSize)
    {
        return 0;
    }

    return GsAdapter10::FromHandle(hAdapter)->CalcPrivateDeviceSize(*pCalcPrivateDeviceSize);
}
