#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsAdapter10 final
{
    DEFAULT_DESTRUCT(GsAdapter10);
    DELETE_CM(GsAdapter10);
public:
    static GsAdapter10* FromHandle(const D3D10DDI_HADAPTER handle) noexcept
    {
        return static_cast<GsAdapter10*>(handle.pDrvPrivate);
    }
public:
    GsAdapter10(
        const D3D10DDI_HRTADAPTER runtimeHandle,
        const D3DDDI_ADAPTERCALLBACKS& adapterCallbacks
    ) noexcept;

    SIZE_T CalcPrivateDeviceSize(const D3D10DDIARG_CALCPRIVATEDEVICESIZE& calcPrivateDeviceSize) noexcept;
    HRESULT CreateDevice(D3D10DDIARG_CREATEDEVICE& createDevice) noexcept;
private:
    D3D10DDI_HRTADAPTER m_RuntimeHandle;
    const D3DDDI_ADAPTERCALLBACKS m_AdapterCallbacks;
};
