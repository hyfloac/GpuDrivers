// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_calcprivatedevicesize
#include "d3d10/CalcPrivateDeviceSize10.hpp"
#include "d3d10/GsAdapter10.hpp"

SIZE_T GsCalcPrivateDeviceSizeD3D10(D3D10DDI_HADAPTER hAdapter, const D3D10DDIARG_CALCPRIVATEDEVICESIZE* const pCalcPrivateDeviceSize)
{
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
