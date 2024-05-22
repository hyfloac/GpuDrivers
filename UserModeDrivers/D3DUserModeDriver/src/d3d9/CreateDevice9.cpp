// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dumddi/nc-d3dumddi-pfnd3dddi_createdevice
#include "d3d9/CreateDevice9.hpp"
#include "d3d9/GsAdapter9.hpp"

HRESULT GsCreateDeviceD3D9(HANDLE hAdapter, D3DDDIARG_CREATEDEVICE* const pCreateDevice)
{
    if(!hAdapter)
    {
        return E_INVALIDARG;
    }

    if(!pCreateDevice)
    {
        return E_INVALIDARG;
    }

    return GsAdapter9::FromHandle(hAdapter)->CreateDevice(*pCreateDevice);
}
