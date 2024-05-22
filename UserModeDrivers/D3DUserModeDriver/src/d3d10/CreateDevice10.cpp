// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_createdevice
#include "d3d10/CreateDevice10.hpp"
#include "d3d10/GsAdapter10.hpp"

HRESULT GsCreateDeviceD3D10(D3D10DDI_HADAPTER hAdapter, D3D10DDIARG_CREATEDEVICE* const pCreateDevice)
{
    if(!hAdapter.pDrvPrivate)
    {
        return E_INVALIDARG;
    }

    if(!pCreateDevice)
    {
        return E_INVALIDARG;
    }

    return GsAdapter10::FromHandle(hAdapter)->CreateDevice(*pCreateDevice);
}
