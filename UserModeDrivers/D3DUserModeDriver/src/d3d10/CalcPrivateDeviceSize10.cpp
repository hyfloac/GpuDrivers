// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_calcprivatedevicesize
#include "d3d10/CalcPrivateDeviceSize10.hpp"
#include "d3d10/GsAdapter10.hpp"
#include "Logging.hpp"

SIZE_T GsCalcPrivateDeviceSizeD3D10(
    const D3D10DDI_HADAPTER hAdapter,
    const D3D10DDIARG_CALCPRIVATEDEVICESIZE* const pCalcPrivateDeviceSize
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"Interface: 0x{X}, Version: 0x{X}, Flags: 0x{XP0}",
        pCalcPrivateDeviceSize->Interface,
        pCalcPrivateDeviceSize->Version,
        pCalcPrivateDeviceSize->Flags
    );

    if(!hAdapter.pDrvPrivate)
    {
        LOG_ERROR(u8"hAdapter was not set.");
        return 0;
    }

    if(!pCalcPrivateDeviceSize)
    {
        LOG_WARN(u8"pCalcPrivateDeviceSize was not set.");
        // return 0;
    }

    return GsAdapter10::FromHandle(hAdapter)->CalcPrivateDeviceSize(*pCalcPrivateDeviceSize);
}
