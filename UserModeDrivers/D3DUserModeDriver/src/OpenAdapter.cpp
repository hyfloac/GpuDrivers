#include <Windows.h>
#include <d3dumddi.h>

extern "C" HRESULT __declspec(dllexport) OpenAdapter(D3DDDIARG_OPENADAPTER* const pOpenAdapter)
{
    return S_OK;
}

extern "C" HRESULT __declspec(dllexport) OpenAdapter10(D3DDDIARG_OPENADAPTER* const pOpenAdapter)
{
    if(!pOpenAdapter)
    {
        return E_INVALIDARG;
    }

    if((pOpenAdapter->Interface >> 16) < 10)
    {
        return E_INVALIDARG;
    }

    pOpenAdapter->DriverVersion = D3D_UMD_INTERFACE_VERSION;

    pOpenAdapter->pAdapterCallbacks->pfnQueryAdapterInfoCb();

    return S_OK;
}
