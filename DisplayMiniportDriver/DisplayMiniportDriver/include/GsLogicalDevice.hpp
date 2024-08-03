#pragma once

#include "Common.h"
#include "Objects.hpp"

class GsLogicalDevice final
{
    DEFAULT_DESTRUCT(GsLogicalDevice);
    DEFAULT_CM_PU(GsLogicalDevice);
public:
    void* operator new(SIZE_T count);
    void operator delete(void* ptr);
public:
    static GsLogicalDevice* FromHandle(HANDLE hDevice) noexcept
    {
        return static_cast<GsLogicalDevice*>(hDevice);
    }
public:
    GsLogicalDevice(
        HANDLE dxgkHandle,
        const bool isSystemDevice, 
        const bool isGdiDevice
    ) noexcept;

    [[nodiscard]] bool IsSystemDevice() const noexcept { return m_IsSystemDevice; }
    [[nodiscard]] bool IsGdiDevice() const noexcept { return m_IsGdiDevice; }
    [[nodiscard]] DXGK_DEVICEINFO& DeviceInfo() noexcept { return m_DeviceInfo; }

    NTSTATUS CreateContext(INOUT_PDXGKARG_CREATECONTEXT pCreateContext) noexcept;
private:
    HANDLE m_DxgkHandle;  // NOLINT(clang-diagnostic-unused-private-field)
    UINT m_IsSystemDevice : 1;
    UINT m_IsGdiDevice : 1;
    UINT m_Pad0 : 30;  // NOLINT(clang-diagnostic-unused-private-field)
    DXGK_DEVICEINFO m_DeviceInfo;
};
