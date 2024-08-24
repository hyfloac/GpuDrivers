#include "d3d10/GsDevice10.hpp"
#include "d3d10/GsBlendState10.hpp"
#include "d3d10/GsDepthStencilState10.hpp"
#include "Logging.hpp"

GsDevice10::GsDevice10(
    const D3D10DDI_HRTDEVICE driverHandle, 
    const D3DDDI_DEVICECALLBACKS& deviceCallbacks,
    const D3D10DDI_HRTCORELAYER runtimeCoreLayerHandle,
    const D3D10DDI_CORELAYER_DEVICECALLBACKS& umCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_DeviceCallbacks(deviceCallbacks)
    , m_RuntimeCoreLayerHandle(runtimeCoreLayerHandle)
    , m_UmCallbacks(umCallbacks)
    , m_BlendState(nullptr)
    , m_BlendFactor { }
    , m_SampleMask(0)
    , m_DepthStencilState(nullptr)
    , m_StencilRef(0)
{ }

void GsDevice10::SetBlendState(
    const D3D10DDI_HBLENDSTATE hBlendState, 
    const FLOAT BlendFactor[4], 
    const UINT SampleMask
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!hBlendState.pDrvPrivate)
    {
        m_BlendState = nullptr;
    }
    else
    {
        m_BlendState = GsBlendState10::FromHandle(hBlendState);
    }

    m_BlendFactor[0] = BlendFactor[0];
    m_BlendFactor[1] = BlendFactor[1];
    m_BlendFactor[2] = BlendFactor[2];
    m_BlendFactor[3] = BlendFactor[3];

    m_SampleMask = SampleMask;
}

void GsDevice10::SetDepthStencilState(
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState, 
    const UINT StencilRef
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!hDepthStencilState.pDrvPrivate)
    {
        m_DepthStencilState = nullptr;
    }
    else
    {
        m_DepthStencilState = GsDepthStencilState10::FromHandle(hDepthStencilState);
    }

    m_StencilRef = StencilRef;
}

SIZE_T GsDevice10::CalcPrivateBlendStateSize(
    const D3D10_DDI_BLEND_DESC* pBlendDesc
) const noexcept
{
    TRACE_ENTRYPOINT();

    UNUSED(pBlendDesc);

    return sizeof(GsBlendState10);
}

void GsDevice10::CreateBlendState(
    const D3D10_DDI_BLEND_DESC* pBlendDesc, 
    const D3D10DDI_HBLENDSTATE hBlendState, 
    const D3D10DDI_HRTBLENDSTATE hRtBlendState
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!hBlendState.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_OUTOFMEMORY);
        return;
    }

    ::new(hBlendState.pDrvPrivate) GsBlendState10(
        *pBlendDesc,
        hRtBlendState
    );
}

void GsDevice10::DestroyBlendState(
    const D3D10DDI_HBLENDSTATE hBlendState
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!hBlendState.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_HANDLE);
        return;
    }

    GsBlendState10* const blendState = GsBlendState10::FromHandle(hBlendState);

    if(blendState == m_BlendState)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_ACCESSDENIED);
        return;
    }

    blendState->~GsBlendState10();
}

SIZE_T GsDevice10::CalcPrivateDepthStencilStateSize(
    const D3D10_DDI_DEPTH_STENCIL_DESC* pDepthStencilState
) const noexcept
{
    TRACE_ENTRYPOINT();

    UNUSED(pDepthStencilState);

    return sizeof(GsDepthStencilState10);
}

void GsDevice10::CreateDepthStencilState(
    const D3D10_DDI_DEPTH_STENCIL_DESC* const pDepthStencilDesc, 
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState, 
    const D3D10DDI_HRTDEPTHSTENCILSTATE hRtDepthStencilState
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!hDepthStencilState.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_OUTOFMEMORY);
        return;
    }

    ::new(hDepthStencilState.pDrvPrivate) GsDepthStencilState10(
        *pDepthStencilDesc,
        hRtDepthStencilState
    );
}

void GsDevice10::DestroyDepthStencilState(
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!hDepthStencilState.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_HANDLE);
        return;
    }

    GsDepthStencilState10* const depthStencilState = GsDepthStencilState10::FromHandle(hDepthStencilState);

    if(depthStencilState == m_DepthStencilState)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_ACCESSDENIED);
        return;
    }

    depthStencilState->~GsDepthStencilState10();
}
