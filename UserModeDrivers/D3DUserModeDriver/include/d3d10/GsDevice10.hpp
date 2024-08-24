#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsBlendState10;
class GsDepthStencilState10;

class GsDevice10 final
{
    DEFAULT_DESTRUCT(GsDevice10);
    DELETE_CM(GsDevice10);
public:
    static GsDevice10* FromHandle(D3D10DDI_HDEVICE device) noexcept
    {
        return static_cast<GsDevice10*>(device.pDrvPrivate);
    }
public:
    GsDevice10(
        const D3D10DDI_HRTDEVICE driverHandle,
        const D3DDDI_DEVICECALLBACKS& deviceCallbacks,
        const D3D10DDI_HRTCORELAYER runtimeCoreLayerHandle,
        const D3D10DDI_CORELAYER_DEVICECALLBACKS& umCallbacks
    ) noexcept;

    void SetBlendState(
        const D3D10DDI_HBLENDSTATE hBlendState,
        const FLOAT BlendFactor[4],
        const UINT SampleMask
    ) noexcept;

    void SetDepthStencilState(
        const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
        const UINT StencilRef
    ) noexcept;

    SIZE_T CalcPrivateBlendStateSize(
        const D3D10_DDI_BLEND_DESC* pBlendDesc
    ) const noexcept;

    void CreateBlendState(
        const D3D10_DDI_BLEND_DESC* pBlendDesc,
        const D3D10DDI_HBLENDSTATE hBlendState,
        const D3D10DDI_HRTBLENDSTATE hRtBlendState
    ) noexcept;

    void DestroyBlendState(
        const D3D10DDI_HBLENDSTATE hBlendState
    ) noexcept;

    SIZE_T CalcPrivateDepthStencilStateSize(
        const D3D10_DDI_DEPTH_STENCIL_DESC* pDepthStencilState
    ) const noexcept;

    void CreateDepthStencilState(
        const D3D10_DDI_DEPTH_STENCIL_DESC* const pDepthStencilDesc,
        const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
        const D3D10DDI_HRTDEPTHSTENCILSTATE hRtDepthStencilState
    ) noexcept;

    void DestroyDepthStencilState(
        const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState
    ) noexcept;

    void CheckFormatSupport(
        const DXGI_FORMAT Format,
        UINT* const pFormatSupport
    ) noexcept;

    void CheckMultisampleQualityLevels(
        const DXGI_FORMAT Format,
        const UINT SampleCount,
        UINT* const pNumQualityLevels
    ) noexcept;
private:
    D3D10DDI_HRTDEVICE m_DriverHandle;
    const D3DDDI_DEVICECALLBACKS m_DeviceCallbacks;
    D3D10DDI_HRTCORELAYER m_RuntimeCoreLayerHandle;
    const D3D10DDI_CORELAYER_DEVICECALLBACKS m_UmCallbacks;

    GsBlendState10* m_BlendState;
    float m_BlendFactor[4];
    UINT m_SampleMask;

    GsDepthStencilState10* m_DepthStencilState;
    UINT m_StencilRef;
};
