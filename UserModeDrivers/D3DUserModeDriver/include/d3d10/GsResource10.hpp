#pragma once

#include "Common.hpp"
#include <Objects.hpp>

struct GsResourceDesc10 final
{
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    UINT ArraySize;
    UINT Pitch;
    UINT SlicePitch;
    UINT Usage;
    UINT BindFlags;
    UINT MapFlags;
    UINT MiscFlags;
    DXGI_FORMAT Format;
    D3DDDIFORMAT DriverFormat;
    DXGI_SAMPLE_DESC SampleDesc;
    UINT64 PhysicalSize;
};

class GsResource10 final
{
    DEFAULT_DESTRUCT(GsResource10);
    DELETE_CM(GsResource10);
public:
    static GsResource10* FromHandle(const D3D10DDI_HRESOURCE handle) noexcept
    {
        return static_cast<GsResource10*>(handle.pDrvPrivate);
    }
public:
    GsResource10(
        const D3D10DDI_HRTRESOURCE runtimeHandle,
        const D3DKMT_HANDLE allocationHandle,
        const GsResourceDesc10& desc
    ) noexcept;

    [[nodiscard]] D3D10DDI_HRTRESOURCE RuntimeHandle() const noexcept { return m_RuntimeHandle; }
    [[nodiscard]] D3DKMT_HANDLE  AllocationHandle() const noexcept { return m_AllocationHandle; }
    [[nodiscard]] D3DKMT_HANDLE& AllocationHandle()       noexcept { return m_AllocationHandle; }
    [[nodiscard]] const GsResourceDesc10& Desc() const noexcept { return m_Desc; }

    [[nodiscard]] void*  Allocation() const noexcept { return m_Allocation; }
    [[nodiscard]] void*& Allocation()       noexcept { return m_Allocation; }
private:
    D3D10DDI_HRTRESOURCE m_RuntimeHandle;
    D3DKMT_HANDLE m_AllocationHandle;
    GsResourceDesc10 m_Desc;
    void* m_Allocation;
};
