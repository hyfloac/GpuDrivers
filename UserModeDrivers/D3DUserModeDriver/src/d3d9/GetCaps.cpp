#include "d3d9/GetCaps.hpp"
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
