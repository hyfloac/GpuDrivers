#include "d3d10/GsDevice10.hpp"
#include "d3d10/GsResource10.hpp"
#include "d3d10/GsBlendState10.hpp"
#include "d3d10/GsDepthStencilState10.hpp"
#include "d3d10/D3D10EnumDebug.hpp"
#include "dxgi/DxgiEnumDebug.hpp"
#include "CreateAllocationDriverData.hpp"
#include "Logging.hpp"

GsDevice10::GsDevice10(
    const D3D10DDI_HRTDEVICE runtimeHandle,
    const D3DDDI_DEVICECALLBACKS& deviceCallbacks,
    const D3D10DDI_HRTCORELAYER runtimeCoreLayerHandle,
    const D3D10DDI_CORELAYER_DEVICECALLBACKS& umCallbacks
) noexcept
    : m_RuntimeHandle(runtimeHandle)
    , m_DeviceCallbacks(deviceCallbacks)
    , m_RuntimeCoreLayerHandle(runtimeCoreLayerHandle)
    , m_UmCallbacks(umCallbacks)
    , m_BlendState(nullptr)
    , m_BlendFactor { }
    , m_SampleMask(0)
    , m_DepthStencilState(nullptr)
    , m_StencilRef(0)
{ }

void GsDevice10::DynamicResourceMapDiscard(
    const D3D10DDI_HRESOURCE hResource,
    const UINT Subresource,
    const D3D10_DDI_MAP MapType,
    const UINT MapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* const pMappedSubresource
) noexcept
{
    GsResource10* const resource = GsResource10::FromHandle(hResource);

    // constexpr UINT64 PAGE_SIZE = 4096;
    //
    // UINT64 pageCount = resource->Desc().PhysicalSize / PAGE_SIZE;
    //
    // if(pageCount * PAGE_SIZE < resource->Desc().PhysicalSize)
    // {
    //     ++pageCount;
    // }
    //
    // UINT* pages = new UINT[pageCount];
    // for(UINT i = 0; i < pageCount; ++i)
    // {
    //     pages[i] = i;
    // }

    D3DDDICB_LOCK lock {};
    lock.hAllocation = resource->AllocationHandle();
    lock.PrivateDriverData = 0;
    // lock.NumPages = static_cast<UINT>(pageCount);
    // lock.pPages = pages;
    lock.NumPages = 0;
    lock.pPages = nullptr;
    lock.pData = nullptr;
    lock.Flags.Value = 0;

    switch(MapType)
    {
        case D3D10_DDI_MAP_READ:
            lock.Flags.ReadOnly = true;
            lock.Flags.WriteOnly = false;
            break;
        case D3D10_DDI_MAP_WRITE:
        case D3D10_DDI_MAP_WRITE_DISCARD:
        case D3D10_DDI_MAP_WRITE_NOOVERWRITE:
            lock.Flags.ReadOnly = false;
            lock.Flags.WriteOnly = true;
            break;
        case D3D10_DDI_MAP_READWRITE:
            lock.Flags.ReadOnly = false;
            lock.Flags.WriteOnly = false;
            break;
        default:
            lock.Flags.ReadOnly = false;
            lock.Flags.WriteOnly = false;
            break;
    }

    lock.Flags.DonotWait = (MapFlags & D3D10_DDI_MAP_FLAG_DONOTWAIT) == D3D10_DDI_MAP_FLAG_DONOTWAIT;
    lock.Flags.IgnoreSync = false;
    lock.Flags.LockEntire = true; // TODO: This is only for debug.
    lock.Flags.DonotEvict = true; // TODO: This is only for debug.
    lock.Flags.AcquireAperture = false; // TODO: This is only for debug.
    lock.Flags.Discard = true; 
    lock.Flags.UseAlternateVA = false;
    lock.Flags.IgnoreReadSync = false; // TODO: This is only for debug.
    lock.Flags.Reserved = 0;

    HRESULT status = m_DeviceCallbacks.pfnLockCb(m_RuntimeHandle.handle, &lock);

    // delete[] pages;

    if(!SUCCEEDED(status))
    {
        LOG_ERROR("Failed to lock resource: 0x{XP0}", status);
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, status);
        return;
    }

    resource->Allocation() = lock.pData;
    // We need to replace here, because we've discarded the old allocation.
    resource->AllocationHandle() = lock.hAllocation;
    pMappedSubresource->pData = lock.pData;
    pMappedSubresource->RowPitch = resource->Desc().Pitch;
    pMappedSubresource->DepthPitch = resource->Desc().SlicePitch;
}

void GsDevice10::DynamicResourceUnmap(
    const D3D10DDI_HRESOURCE hResource,
    const UINT Subresource
) noexcept
{
    GsResource10* const resource = GsResource10::FromHandle(hResource);

    // We need two handles if we have a swizzled texture.
    // https://learn.microsoft.com/en-us/windows-hardware/drivers/display/mapping-virtual-addresses-to-a-memory-segment
    D3DKMT_HANDLE handles[2];
    handles[0] = resource->AllocationHandle();
    handles[1] = 0;

    D3DDDICB_UNLOCK unlock {};
    unlock.NumAllocations = 1;
    unlock.phAllocations = handles;

    HRESULT status = m_DeviceCallbacks.pfnUnlockCb(m_RuntimeHandle.handle, &unlock);

    if(!SUCCEEDED(status))
    {
        LOG_ERROR("Failed to unlock resource: 0x{XP0}", status);
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, status);
        return;
    }
}

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

SIZE_T GsDevice10::CalcPrivateResourceSize(
    const D3D10DDIARG_CREATERESOURCE* const pCreateResource
) const noexcept
{
    TRACE_ENTRYPOINT();

    UNUSED(pCreateResource);

    return sizeof(GsResource10);
}

void GsDevice10::CreateResource(
    const D3D10DDIARG_CREATERESOURCE* const pCreateResource, 
    const D3D10DDI_HRESOURCE hResource, 
    const D3D10DDI_HRTRESOURCE hRtResource
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!pCreateResource)
    {
        LOG_ERROR("pCreateResource was not set.");
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
        return;
    }

    LOG_DEBUG(
        "pMipInfoList: 0x{XP0}, pInitialDataUP: 0x{XP0}, ResourceDimension: {}, Usage: 0x{XP0}, BindFlags: 0x{XP0}, MapFlags: 0x{XP0}, MiscFlags: 0x{XP0}, Format: {}, SampleDesc.Count: {}, SampleDesc.Quality: {}, MipLevels: {}, ArraySize: {}, pPrimaryDesc: 0x{XP0}",
        pCreateResource->pMipInfoList,
        pCreateResource->pInitialDataUP,
        D3D10ResourceTypeToString(pCreateResource->ResourceDimension),
        pCreateResource->Usage,
        pCreateResource->BindFlags,
        pCreateResource->MapFlags,
        pCreateResource->MiscFlags,
        DxgiFormatToString(pCreateResource->Format),
        pCreateResource->SampleDesc.Count,
        pCreateResource->SampleDesc.Quality,
        pCreateResource->MipLevels,
        pCreateResource->ArraySize,
        pCreateResource->pPrimaryDesc
    );

    if(!hResource.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
        return;
    }

    UINT pixelSize = 4;

    if(pCreateResource->Format == DXGI_FORMAT_R8G8B8A8_UNORM)
    {
        pixelSize = 4;
    }

    UINT64 physicalSize = 0;

    for(UINT i = 0; i < pCreateResource->MipLevels; ++i)
    {
        LOG_DEBUG(
            "[Mip {}] TexelWidth: {}, TexelHeight {}, TexelDepth {}, PhysicalWidth {}, PhysicalHeight {}, PhysicalDepth: {}",
            i,
            pCreateResource->pMipInfoList[i].TexelWidth,
            pCreateResource->pMipInfoList[i].TexelHeight,
            pCreateResource->pMipInfoList[i].TexelDepth,
            pCreateResource->pMipInfoList[i].PhysicalWidth,
            pCreateResource->pMipInfoList[i].PhysicalHeight,
            pCreateResource->pMipInfoList[i].PhysicalDepth
        );

        physicalSize += pixelSize * pCreateResource->pMipInfoList[i].PhysicalWidth * pCreateResource->pMipInfoList[i].PhysicalHeight * pCreateResource->pMipInfoList[i].PhysicalDepth;
    }

    CreateAllocationDriverData allocationData {};
    allocationData.V1.Base.Magic = CREATE_ALLOCATION_MAGIC;
    allocationData.V1.Base.Version = CreateAllocationDriverData_V1::Version;
    allocationData.V1.PhysicalSize = physicalSize;

    AllocationInfoDriverData allocationInfo {};
    allocationInfo.V1.Base.Magic = ALLOCATION_INFO_MAGIC;
    allocationInfo.V1.Base.Version = AllocationInfoDriverData_V1::Version;
    allocationInfo.V1.PhysicalSize = physicalSize;
    allocationInfo.V1.Flags.PrivateFormat = false;
    allocationInfo.V1.Flags.Swizzled = false;
    allocationInfo.V1.Flags.CubeTexture = pCreateResource->ResourceDimension == D3D10DDIRESOURCE_TEXTURECUBE;
    allocationInfo.V1.Flags.VolumeTexture = pCreateResource->ResourceDimension == D3D10DDIRESOURCE_TEXTURE3D;
    if(pCreateResource->ResourceDimension == D3D10DDIRESOURCE_BUFFER)
    {
        allocationInfo.V1.Flags.VertexBuffer = (pCreateResource->BindFlags & D3D10_DDI_BIND_VERTEX_BUFFER) == D3D10_DDI_BIND_VERTEX_BUFFER;
        allocationInfo.V1.Flags.IndexBuffer = (pCreateResource->BindFlags & D3D10_DDI_BIND_INDEX_BUFFER) == D3D10_DDI_BIND_INDEX_BUFFER;
    }
    else
    {
        allocationInfo.V1.Flags.VertexBuffer = false;
        allocationInfo.V1.Flags.IndexBuffer = false;
    }
    allocationInfo.V1.Flags.Reserved = 0;
    allocationInfo.V1.Format = D3DDDIFMT_UNKNOWN;

    switch(pCreateResource->Format)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            allocationInfo.V1.Format = D3DDDIFMT_A8R8G8B8;
            break;
        default:
            LOG_ERROR("Unsupported format: {}", DxgiFormatToString(pCreateResource->Format));
            m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
            return;
    }

    if(pCreateResource->pMipInfoList && pCreateResource->MipLevels > 0)
    {
        allocationInfo.V1.Width = pCreateResource->pMipInfoList[0].TexelWidth;
        allocationInfo.V1.Height = pCreateResource->pMipInfoList[0].TexelHeight;
        allocationInfo.V1.Pitch = pCreateResource->pMipInfoList[0].PhysicalWidth * pixelSize;
        allocationInfo.V1.Depth = pCreateResource->pMipInfoList[0].TexelDepth;
        allocationInfo.V1.SlicePitch = pCreateResource->pMipInfoList[0].PhysicalWidth * pCreateResource->pMipInfoList[0].PhysicalHeight * pixelSize;
    }

    D3DDDICB_ALLOCATE allocate {};
    allocate.pPrivateDriverData = &allocationData;
    allocate.PrivateDriverDataSize = sizeof(allocationData.V1);
    allocate.hResource = hRtResource.handle;
    allocate.NumAllocations = 1;
    D3DDDI_ALLOCATIONINFO allocationInfos[1] {};
    allocationInfos[0].hAllocation = 0;
    allocationInfos[0].pSystemMem = nullptr;
    allocationInfos[0].pPrivateDriverData = &allocationInfo;
    allocationInfos[0].PrivateDriverDataSize = sizeof(allocationInfo.V1);
    allocationInfos[0].VidPnSourceId = 0;
    allocationInfos[0].Flags.Primary = 0;
#if ((DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8) || \
     (D3D_UMD_INTERFACE_VERSION >= D3D_UMD_INTERFACE_VERSION_WIN8))
    allocationInfos[0].Flags.Stereo = 0;
#endif
    allocationInfos[0].Flags.Reserved = 0;
    allocate.pAllocationInfo = allocationInfos;
    HRESULT status = m_DeviceCallbacks.pfnAllocateCb(m_RuntimeHandle.handle, &allocate);

    if(!SUCCEEDED(status))
    {
        LOG_ERROR("Failed to allocate resource: 0x{XP0}", status);
        LogWindowsHResultAndError(status);
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, status);
        return;
    }

    GsResourceDesc10 desc {};
    desc.Width = allocationInfo.V1.Width;
    desc.Height = allocationInfo.V1.Height;
    desc.Depth = allocationInfo.V1.Depth;
    desc.MipLevels = pCreateResource->MipLevels;
    desc.ArraySize = pCreateResource->ArraySize;
    desc.Pitch = allocationInfo.V1.Pitch;
    desc.SlicePitch = allocationInfo.V1.SlicePitch;
    desc.Usage = pCreateResource->Usage;
    desc.BindFlags = pCreateResource->BindFlags;
    desc.MapFlags = pCreateResource->MapFlags;
    desc.MiscFlags = pCreateResource->MiscFlags;
    desc.Format = pCreateResource->Format;
    desc.DriverFormat = allocationInfo.V1.Format;
    desc.SampleDesc.Count = pCreateResource->SampleDesc.Count;
    desc.SampleDesc.Quality = pCreateResource->SampleDesc.Quality;
    desc.PhysicalSize = physicalSize;

    ::new(hResource.pDrvPrivate) GsResource10(
        hRtResource,
        allocationInfos[0].hAllocation,
        desc
    );
}

SIZE_T GsDevice10::CalcPrivateBlendStateSize(
    const D3D10_DDI_BLEND_DESC* const pBlendDesc
) const noexcept
{
    TRACE_ENTRYPOINT();

    UNUSED(pBlendDesc);

    return sizeof(GsBlendState10);
}

void GsDevice10::CreateBlendState(
    const D3D10_DDI_BLEND_DESC* const pBlendDesc,
    const D3D10DDI_HBLENDSTATE hBlendState, 
    const D3D10DDI_HRTBLENDSTATE hRtBlendState
) noexcept
{
    TRACE_ENTRYPOINT();

    if(!pBlendDesc)
    {
        LOG_ERROR("pBlendDesc was not set.");
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
        return;
    }

    if(!hBlendState.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
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
    const D3D10_DDI_DEPTH_STENCIL_DESC* const pDepthStencilState
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

    if(!pDepthStencilDesc)
    {
        LOG_ERROR("pDepthStencilDesc was not set.");
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
        return;
    }

    if(!hDepthStencilState.pDrvPrivate)
    {
        m_UmCallbacks.pfnSetErrorCb(m_RuntimeCoreLayerHandle, E_INVALIDARG);
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
