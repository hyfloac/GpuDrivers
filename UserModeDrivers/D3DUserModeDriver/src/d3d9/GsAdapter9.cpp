#include "d3d9/GsAdapter9.hpp"
#include "d3d9/GsDevice9.hpp"

GsAdapter9::GsAdapter9(
    const HANDLE driverHandle, 
    const D3DDDI_ADAPTERCALLBACKS& adapterCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_AdapterCallbacks(adapterCallbacks)
{ }

HRESULT GsAdapter9::GetCaps(const D3DDDIARG_GETCAPS& getCaps) noexcept
{
    switch(getCaps.Type)
    {
        case D3DDDICAPS_DDRAW: return GetCapsDDraw(getCaps);
        default: return E_OUTOFMEMORY;
    }
}

HRESULT GsAdapter9::CreateDevice(D3DDDIARG_CREATEDEVICE& createDevice) noexcept
{
    D3DDDICB_CREATECONTEXT createContext { };
    createContext.NodeOrdinal = 0;
    createContext.EngineAffinity = 0;
    createContext.Flags.Value = 0;
    createContext.pPrivateDriverData = nullptr;
    createContext.PrivateDriverDataSize = 0;

    createDevice.pCallbacks->pfnCreateContextCb(createDevice.hDevice, &createContext);

    GsDevice9* const device = new GsDevice9(createDevice.hDevice, *createDevice.pCallbacks);

    createDevice.hDevice = device;


    return S_OK;
}

HRESULT GsAdapter9::GetCapsDDraw(const D3DDDIARG_GETCAPS& getCaps) noexcept
{
    if(getCaps.Type != D3DDDICAPS_DDRAW)
    {
        return E_INVALIDARG;
    }

    if(getCaps.DataSize != sizeof(DDRAW_CAPS))
    {
        return E_INVALIDARG;
    }

    if(!getCaps.pData)
    {
        return E_INVALIDARG;
    }

    DDRAW_CAPS* const caps = reinterpret_cast<DDRAW_CAPS*>(getCaps.pData);

    caps->Caps = DDRAW_CAPS_ZBLTS | DDRAW_CAPS_COLORKEY | DDRAW_CAPS_BLTDEPTHFILL;
    caps->Caps2 = DDRAW_CAPS2_FLIPINTERVAL | DDRAW_CAPS2_FLIPNOVSYNC | DDRAW_CAPS2_DYNAMICTEXTURES;
    caps->CKeyCaps = DDRAW_CKEYCAPS_SRCBLT | DDRAW_CKEYCAPS_DESTBLT;
    caps->FxCaps = 0;
    caps->MaxVideoPorts = 1;

    return S_OK;
}
