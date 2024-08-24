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

void GsDevice10::CheckFormatSupport(
    const DXGI_FORMAT Format, 
    UINT* const pFormatSupport
) noexcept
{
    if constexpr(false)
    {
        TRACE_ENTRYPOINT();
    }

    if(!pFormatSupport)
    {
        // This by spec has to be E_INVALIDARG, it cannot be E_POINTER.
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
        return;
    }

    // TODO: Implement proper format support checking.

    switch(Format)
    {
        case DXGI_FORMAT_UNKNOWN:
        // Typeless formats
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC6H_TYPELESS:
            //   Typeless formats obviously cannot be sampled or rendered to since
            // we don't know the bit format.
            *pFormatSupport = 0;
            break;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            // These formats can be sampled and rendered to.
            *pFormatSupport = 
                D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE | 
                D3D10_DDI_FORMAT_SUPPORT_RENDERTARGET | 
                D3D10_DDI_FORMAT_SUPPORT_BLENDABLE;
            return;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            //   These are depth-stencil formats, we can render to it, but you need
            // to mask some of the bits to sample from it.
            *pFormatSupport = D3D10_DDI_FORMAT_SUPPORT_RENDERTARGET;
            return;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            // These are masked depth-stencil formats. They're only for sampling.
            *pFormatSupport = D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE;
            return;
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_R1_UNORM:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            // We'll pretend you can sample from all of these.
            *pFormatSupport = D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE;
            return;
        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_YUY2:

        case DXGI_FORMAT_P208:
        case DXGI_FORMAT_V208:
        case DXGI_FORMAT_V408:

        case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
        case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
            // These are formats I don't want to deal wiht for now.
            *pFormatSupport = 0;
            break;
#if (D3D_UMD_INTERFACE_VERSION >= D3D_UMD_INTERFACE_VERSION_WIN8)
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
        case DXGI_FORMAT_NV11:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
        case DXGI_FORMAT_A8P8:
            *pFormatSupport = D3D10_DDI_FORMAT_SUPPORT_NOT_SUPPORTED;
            break;
#endif
        default:
            // This format does not exist.
            m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_FAIL);
            break;
    }
}

void GsDevice10::CheckMultisampleQualityLevels(
    const DXGI_FORMAT Format, 
    const UINT SampleCount, 
    UINT* const pNumQualityLevels
) noexcept
{
    if constexpr(false)
    {
        TRACE_ENTRYPOINT();
    }

    UNUSED(Format);

    if(!pNumQualityLevels)
    {
        // This by spec has to be E_INVALIDARG, it cannot be E_POINTER.
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
        return;
    }

    if(SampleCount == 1)
    {
        //   If the driver receives 1 in SampleCount, the driver always returns
        // 1 in the variable that pNumQualityLevels points to.
        // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_checkmultisamplequalitylevels
        *pNumQualityLevels = 1;
    }

    if(SampleCount == 0 || SampleCount > 32)
    {
        //   If the driver receives 0 or greater than 32 in SampleCount, the
        // driver always returns 0 in the variable that pNumQualityLevels points to.
        // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_checkmultisamplequalitylevels
        *pNumQualityLevels = 0;
    }

    // We're just going to ignore multisampling for now.
    // TODO: Add multisampling support to texture formats.
    *pNumQualityLevels = 0;
}
