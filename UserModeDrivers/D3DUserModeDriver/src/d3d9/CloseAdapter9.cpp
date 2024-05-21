// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dumddi/nc-d3dumddi-pfnd3dddi_closeadapter
#include "d3d9/CloseAdapter9.hpp"
#include "d3d9/GsAdapter9.hpp"

HRESULT GsCloseAdapterD3D9(HANDLE hAdapter)
{
    if(!hAdapter)
    {
        return E_INVALIDARG;
    }

    delete GsAdapter9::FromHandle(hAdapter);
    return S_OK;
}
