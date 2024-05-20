#include "d3d9/CreateDevice.hpp"
#include "d3d9/GsAdapter9.hpp"

HRESULT GsCreateDeviceD3D9(HANDLE hAdapter, D3DDDIARG_CREATEDEVICE* pCreateDevice)
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
