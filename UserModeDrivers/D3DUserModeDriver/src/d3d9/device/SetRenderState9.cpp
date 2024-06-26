// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dumddi/nc-d3dumddi-pfnd3dddi_setrenderstate
#include "d3d9/device/SetRenderState9.hpp"
#include "d3d9/GsDevice9.hpp"

HRESULT GsSetRenderState9(HANDLE hDevice, const D3DDDIARG_RENDERSTATE* const pRenderState)
{
    if(!hDevice)
    {
        return E_INVALIDARG;
    }

    if(!pRenderState)
    {
        return E_INVALIDARG;
    }

    return GsDevice9::FromHandle(hDevice)->SetRenderState(*pRenderState);
}
