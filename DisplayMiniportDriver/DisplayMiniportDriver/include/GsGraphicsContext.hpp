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
    GsGraphicsContext(const bool isSoftwareDevice, const bool isGdiDevice) noexcept;

    [[nodiscard]] bool IsSoftwareDevice() const noexcept { return m_IsSoftwareDevice; }
    [[nodiscard]] bool IsGdiDevice() const noexcept { return m_IsGdiDevice; }
private:
    UINT m_IsSoftwareDevice : 1;
    UINT m_IsGdiDevice : 1;
    UINT m_Pad0 : 30;
};
