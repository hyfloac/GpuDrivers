#include "d3d10/GsDevice10.hpp"
#include "d3d10/GsBlendState10.hpp"
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
{ }

void GsDevice10::SetBlendState(
    const D3D10DDI_HBLENDSTATE hBlendState, 
    const FLOAT BlendFactor[4], 
    const UINT SampleMask
) noexcept
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT();
    }
#endif

    if(!hBlendState.pDrvPrivate)
    {
        // This should never happen.
        //   If the user sets the blend state to null, the D3D10 runtime sets
        // passes the default state it created.
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

SIZE_T GsDevice10::CalcPrivateBlendStateSize(const D3D10_DDI_BLEND_DESC* pBlendDesc) const noexcept
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT();
    }
#endif

    UNUSED(pBlendDesc);

    return sizeof(GsBlendState10);
}

void GsDevice10::CreateBlendState(
    const D3D10_DDI_BLEND_DESC* pBlendDesc, 
    const D3D10DDI_HBLENDSTATE hBlendState, 
    const D3D10DDI_HRTBLENDSTATE hRtBlendState
) noexcept
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT();
    }
#endif

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
