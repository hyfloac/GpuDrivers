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
    static GsGraphicsContext* FromHandle(HANDLE hContext) noexcept
    {
        return static_cast<GsGraphicsContext*>(hContext);
    }
public:
    GsGraphicsContext(
        const UINT nodeOrdinal,
        const UINT engineAffinity,
        const bool isSystemContext,
        const bool isGdiContext,
        const bool isVirtualAddressing,
        const bool isSystemProtectedContext,
        const bool isHwQueueSupported,
        const bool isTestContext
    ) noexcept;

    [[nodiscard]] UINT NodeOrdinal() const noexcept { return m_NodeOrdinal; }
    [[nodiscard]] bool IsSystemContext() const noexcept { return m_IsSystemContext; }
    [[nodiscard]] bool IsGdiContext() const noexcept { return m_IsGdiContext; }
    [[nodiscard]] bool IsVirtualAddressing() const noexcept { return m_IsVirtualAddressing; }
    [[nodiscard]] bool IsSystemProtectedContext() const noexcept { return m_IsSystemProtectedContext; }
    [[nodiscard]] bool IsHwQueueSupported() const noexcept { return m_IsHwQueueSupported; }
    [[nodiscard]] bool IsTestContext() const noexcept { return m_IsTestContext; }
    [[nodiscard]] DXGK_CONTEXTINFO& ContextInfo() noexcept { return m_ContextInfo; }
private:
    UINT m_NodeOrdinal;
    UINT m_EngineAffinity;
    UINT m_IsSystemContext : 1;
    UINT m_IsGdiContext : 1;
    UINT m_IsVirtualAddressing : 1;
    UINT m_IsSystemProtectedContext : 1;
    UINT m_IsHwQueueSupported : 1;
    UINT m_IsTestContext : 1;
    UINT m_Pad0 : 26;  // NOLINT(clang-diagnostic-unused-private-field)
    DXGK_CONTEXTINFO m_ContextInfo;
};
