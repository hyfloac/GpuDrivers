#pragma once

#include "Common.h"
#include "Objects.hpp"

class GsGraphicsContext final
{
    DEFAULT_DESTRUCT(GsGraphicsContext);
    DEFAULT_CM_PU(GsGraphicsContext);
public:
    void* operator new(SIZE_T count);
    void operator delete(void* ptr);
public:
    static GsGraphicsContext* FromHandle(HANDLE hDevice) noexcept
    {
        return static_cast<GsGraphicsContext*>(hDevice);
    }
public:
    GsGraphicsContext(
        HANDLE dxgkHandle,
        const bool isSystemDevice, 
        const bool isGdiDevice
    ) noexcept;

    [[nodiscard]] bool IsSystemDevice() const noexcept { return m_IsSystemDevice; }
    [[nodiscard]] bool IsGdiDevice() const noexcept { return m_IsGdiDevice; }
    [[nodiscard]] DXGK_DEVICEINFO& DeviceInfo() noexcept { return m_DeviceInfo; }
private:
    HANDLE m_DxgkHandle;  // NOLINT(clang-diagnostic-unused-private-field)
    UINT m_IsSystemDevice : 1;
    UINT m_IsGdiDevice : 1;
    UINT m_Pad0 : 30;  // NOLINT(clang-diagnostic-unused-private-field)
    DXGK_DEVICEINFO m_DeviceInfo;
};
