// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_destroydevice
#include "d3d10/device/DestroyDevice10.hpp"
#include "d3d10/GsDevice10.hpp"

void GsDestroyDevice10(D3D10DDI_HDEVICE hDevice)
{
    if(!hDevice.pDrvPrivate)
    {
        return;
    }

    GsDevice10::FromHandle(hDevice)->~GsDevice10();
}

