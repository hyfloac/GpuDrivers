#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsDevice9 final
{
    DEFAULT_DESTRUCT(GsDevice9);
    DELETE_CM(GsDevice9);
public:
    static GsDevice9* FromHandle(HANDLE adapter) noexcept
    {
        return reinterpret_cast<GsDevice9*>(adapter);
    }
public:
    GsDevice9(
        const HANDLE driverHandle,
        const D3DDDI_DEVICECALLBACKS& deviceCallbacks
    ) noexcept;

private:
private:
    HANDLE m_DriverHandle;
    const D3DDDI_DEVICECALLBACKS m_DeviceCallbacks;
};
