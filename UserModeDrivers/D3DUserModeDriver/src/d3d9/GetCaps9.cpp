// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dumddi/nc-d3dumddi-pfnd3dddi_getcaps
#include "d3d9/GetCaps9.hpp"
#include "d3d9/GsAdapter9.hpp"

HRESULT GsGetCapsD3D9(HANDLE hAdapter, const D3DDDIARG_GETCAPS* pGetCaps)
{
    if(!hAdapter)
    {
        return E_INVALIDARG;
    }

    if(!pGetCaps)
    {
        return E_INVALIDARG;
    }

    return GsAdapter9::FromHandle(hAdapter)->GetCaps(*pGetCaps);
}
