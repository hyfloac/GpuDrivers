#include "d3d9/GsAdapter9.hpp"
#include "d3d9/GsDevice9.hpp"
#include <ConPrinter.hpp>

#include "d3d9/SetRenderState9.hpp"

GsAdapter9::GsAdapter9(
    const HANDLE driverHandle, 
    const D3DDDI_ADAPTERCALLBACKS& adapterCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_AdapterCallbacks(adapterCallbacks)
{ }

HRESULT GsAdapter9::GetCaps(const D3DDDIARG_GETCAPS& getCaps) noexcept
{
    switch(getCaps.Type)
    {
        case D3DDDICAPS_DDRAW: return GetCapsDDraw(getCaps);
        case D3DDDICAPS_DDRAW_MODE_SPECIFIC: return GetCapsDDrawModeSpecific(getCaps);
        case D3DDDICAPS_GETD3D9CAPS: return GetCapsD3D9(getCaps);
        default: return E_OUTOFMEMORY;
    }
}

#define GEN_NOOP_HRESULT(PTR) \
    { \
        using fun_t = HRESULT(*)(); \
        fun_t fun = []() -> HRESULT \
        { \
            ConPrinter::PrintLn("{}", #PTR); \
            return S_OK; \
        }; \
        (PTR) = reinterpret_cast<decltype(PTR)>(fun); \
    }

HRESULT GsAdapter9::CreateDevice(D3DDDIARG_CREATEDEVICE& createDevice) noexcept
{
    D3DDDICB_CREATECONTEXT createContext { };
    createContext.NodeOrdinal = 0;
    createContext.EngineAffinity = 0;
    createContext.Flags.Value = 0;
    createContext.pPrivateDriverData = nullptr;
    createContext.PrivateDriverDataSize = 0;

    createDevice.pCallbacks->pfnCreateContextCb(createDevice.hDevice, &createContext);

    GsDevice9* const device = new GsDevice9(createDevice.hDevice, *createDevice.pCallbacks);

    createDevice.hDevice = device;

    createDevice.pDeviceFuncs->pfnSetRenderState = GsSetRenderState;
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnUpdateWInfo);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetTextureStageState);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetTexture);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetPixelShader);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetPixelShaderConst);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetStreamSourceUm);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetIndices);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetIndicesUm);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDrawPrimitive);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDrawIndexedPrimitive);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDrawRectPatch);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDrawTriPatch);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDrawPrimitive2);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDrawIndexedPrimitive2);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnVolBlt);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnBufBlt);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnTexBlt);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnStateSet);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetPriority);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnClear);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnUpdatePalette);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetPalette);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetVertexShaderConst);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnMultiplyTransform);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetTransform);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetViewport);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetZRange);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetMaterial);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetLight);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateLight);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyLight);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetClipPlane);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnGetInfo);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnLock);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnUnlock);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateResource);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyResource);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetDisplayMode);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnPresent);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnFlush);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateVertexShaderFunc);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDeleteVertexShaderFunc);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetVertexShaderFunc);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateVertexShaderDecl);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDeleteVertexShaderDecl);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetVertexShaderDecl);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetVertexShaderConstI);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetVertexShaderConstB);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetScissorRect);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetStreamSource);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetStreamSourceFreq);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetConvolutionKernelMono);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnComposeRects);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnBlt);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnColorFill);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDepthFill);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateQuery);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyQuery);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnIssueQuery);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnGetQueryData);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetRenderTarget);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetDepthStencil);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnGenerateMipSubLevels);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetPixelShaderConstI);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetPixelShaderConstB);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreatePixelShader);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDeletePixelShader);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateDecodeDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyDecodeDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetDecodeRenderTarget);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDecodeBeginFrame);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDecodeEndFrame);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDecodeExecute);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDecodeExtensionExecute);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateVideoProcessDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyVideoProcessDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnVideoProcessBeginFrame);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnVideoProcessEndFrame);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetVideoProcessRenderTarget);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnVideoProcessBlt);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateExtensionDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyExtensionDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnExtensionExecute);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCreateOverlay);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnUpdateOverlay);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnFlipOverlay);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnGetOverlayColorControls);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnSetOverlayColorControls);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyOverlay);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnDestroyDevice);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnQueryResourceResidency);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnOpenResource);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnGetCaptureAllocationHandle);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnCaptureToSysMem);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnLockAsync);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnUnlockAsync);
    GEN_NOOP_HRESULT(createDevice.pDeviceFuncs->pfnRename);


    return S_OK;
}

#undef GEN_NOOP_HRESULT

HRESULT GsAdapter9::GetCapsDDraw(const D3DDDIARG_GETCAPS& getCaps) noexcept
{
    if(getCaps.Type != D3DDDICAPS_DDRAW)
    {
        return E_INVALIDARG;
    }

    if(getCaps.DataSize != sizeof(DDRAW_CAPS))
    {
        return E_INVALIDARG;
    }

    if(!getCaps.pData)
    {
        return E_INVALIDARG;
    }

    DDRAW_CAPS* const caps = reinterpret_cast<DDRAW_CAPS*>(getCaps.pData);

    caps->Caps = DDRAW_CAPS_ZBLTS | DDRAW_CAPS_COLORKEY | DDRAW_CAPS_BLTDEPTHFILL;
    caps->Caps2 = DDRAW_CAPS2_FLIPINTERVAL | DDRAW_CAPS2_FLIPNOVSYNC | DDRAW_CAPS2_DYNAMICTEXTURES;
    caps->CKeyCaps = DDRAW_CKEYCAPS_SRCBLT | DDRAW_CKEYCAPS_DESTBLT;
    caps->FxCaps = 0;
    caps->MaxVideoPorts = 1;

    return S_OK;
}

HRESULT GsAdapter9::GetCapsDDrawModeSpecific(const D3DDDIARG_GETCAPS& getCaps) noexcept
{
    if(getCaps.Type != D3DDDICAPS_DDRAW_MODE_SPECIFIC)
    {
        return E_INVALIDARG;
    }

    if(getCaps.DataSize != sizeof(DDRAW_MODE_SPECIFIC_CAPS))
    {
        return E_INVALIDARG;
    }

    if(!getCaps.pData)
    {
        return E_INVALIDARG;
    }

    DDRAW_MODE_SPECIFIC_CAPS* const caps = reinterpret_cast<DDRAW_MODE_SPECIFIC_CAPS*>(getCaps.pData);

    caps->Caps = 0;
    caps->CKeyCaps = 0;
    caps->FxCaps = 0;
    caps->MaxVisibleOverlays = 0;
    caps->MinOverlayStretch = 1;
    caps->MaxOverlayStretch = 32000;

    return S_OK;
}

HRESULT GsAdapter9::GetCapsD3D9(const D3DDDIARG_GETCAPS& getCaps) noexcept
{
    if(getCaps.Type != D3DDDICAPS_GETD3D9CAPS)
    {
        return E_INVALIDARG;
    }

    if(getCaps.DataSize != sizeof(D3DCAPS9))
    {
        return E_INVALIDARG;
    }

    if(!getCaps.pData)
    {
        return E_INVALIDARG;
    }

    D3DCAPS9* const caps = reinterpret_cast<D3DCAPS9*>(getCaps.pData);

    caps->DeviceType = D3DDEVTYPE_HAL;
    caps->AdapterOrdinal = 0;
    caps->Caps = 0;
    caps->Caps2 = 
        D3DCAPS2_CANAUTOGENMIPMAP | 
        D3DCAPS2_CANCALIBRATEGAMMA | 
        D3DCAPS2_CANSHARERESOURCE | 
        D3DCAPS2_DYNAMICTEXTURES | 
        D3DCAPS2_FULLSCREENGAMMA;
    caps->Caps3 = 
        D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD | 
        D3DCAPS3_COPY_TO_VIDMEM | 
        D3DCAPS3_COPY_TO_SYSTEMMEM | 
        D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION;
    caps->PresentationIntervals = 
        D3DPRESENT_INTERVAL_IMMEDIATE | 
        D3DPRESENT_INTERVAL_ONE | 
        D3DPRESENT_INTERVAL_TWO;
    caps->CursorCaps = 
        D3DCURSORCAPS_COLOR | 
        D3DCURSORCAPS_LOWRES;
    caps->DevCaps = 
        D3DDEVCAPS_CANBLTSYSTONONLOCAL | 
        D3DDEVCAPS_CANRENDERAFTERFLIP | 
        D3DDEVCAPS_DRAWPRIMITIVES2 | 
        D3DDEVCAPS_DRAWPRIMITIVES2EX | 
        D3DDEVCAPS_DRAWPRIMTLVERTEX | 
        D3DDEVCAPS_EXECUTESYSTEMMEMORY | 
        D3DDEVCAPS_EXECUTEVIDEOMEMORY |
        D3DDEVCAPS_HWRASTERIZATION | 
        D3DDEVCAPS_HWTRANSFORMANDLIGHT |
        D3DDEVCAPS_NPATCHES |
        D3DDEVCAPS_PUREDEVICE |
        D3DDEVCAPS_RTPATCHES |
        D3DDEVCAPS_RTPATCHHANDLEZERO |
        D3DDEVCAPS_TEXTUREVIDEOMEMORY;
    caps->PrimitiveMiscCaps =
        D3DPMISCCAPS_MASKZ |
        D3DPMISCCAPS_CULLCW |
        D3DPMISCCAPS_CULLCCW |
        D3DPMISCCAPS_COLORWRITEENABLE |
        D3DPMISCCAPS_CLIPPLANESCALEDPOINTS |
        D3DPMISCCAPS_CLIPTLVERTS |
        D3DPMISCCAPS_BLENDOP |
        D3DPMISCCAPS_INDEPENDENTWRITEMASKS |
        D3DPMISCCAPS_PERSTAGECONSTANT |
        D3DPMISCCAPS_FOGANDSPECULARALPHA |
        D3DPMISCCAPS_SEPARATEALPHABLEND |
        D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS |
        D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING;
    caps->RasterCaps =
        D3DPRASTERCAPS_ANISOTROPY |
        D3DPRASTERCAPS_COLORPERSPECTIVE |
        D3DPRASTERCAPS_DEPTHBIAS |
        D3DPRASTERCAPS_FOGRANGE |
        D3DPRASTERCAPS_FOGVERTEX |
        D3DPRASTERCAPS_MIPMAPLODBIAS |
        D3DPRASTERCAPS_MULTISAMPLE_TOGGLE |
        D3DPRASTERCAPS_SCISSORTEST |
        D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS |
        D3DPRASTERCAPS_ZFOG |
        D3DPRASTERCAPS_ZTEST;
    caps->ZCmpCaps =
        D3DPCMPCAPS_ALWAYS |
        D3DPCMPCAPS_EQUAL |
        D3DPCMPCAPS_GREATER |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_LESS |
        D3DPCMPCAPS_LESSEQUAL |
        D3DPCMPCAPS_NEVER |
        D3DPCMPCAPS_NOTEQUAL;
    caps->SrcBlendCaps =
        D3DPBLENDCAPS_BLENDFACTOR |
        D3DPBLENDCAPS_BOTHINVSRCALPHA |
        D3DPBLENDCAPS_BOTHSRCALPHA |
        D3DPBLENDCAPS_DESTALPHA |
        D3DPBLENDCAPS_DESTCOLOR |
        D3DPBLENDCAPS_INVDESTALPHA |
        D3DPBLENDCAPS_INVDESTCOLOR |
        D3DPBLENDCAPS_INVSRCALPHA |
        D3DPBLENDCAPS_INVSRCCOLOR |
        D3DPBLENDCAPS_INVSRCCOLOR2 |
        D3DPBLENDCAPS_ONE |
        D3DPBLENDCAPS_SRCALPHA |
        D3DPBLENDCAPS_SRCALPHASAT |
        D3DPBLENDCAPS_SRCCOLOR |
        D3DPBLENDCAPS_SRCCOLOR2 |
        D3DPBLENDCAPS_ZERO;
    caps->DestBlendCaps = caps->SrcBlendCaps;
    caps->AlphaCmpCaps = caps->ZCmpCaps;
    caps->ShadeCaps =
        D3DPSHADECAPS_ALPHAGOURAUDBLEND |
        D3DPSHADECAPS_COLORGOURAUDRGB |
        D3DPSHADECAPS_FOGGOURAUD |
        D3DPSHADECAPS_SPECULARGOURAUDRGB;
    caps->TextureCaps =
        D3DPTEXTURECAPS_ALPHA |
        D3DPTEXTURECAPS_CUBEMAP |
        D3DPTEXTURECAPS_MIPCUBEMAP |
        D3DPTEXTURECAPS_MIPMAP |
        D3DPTEXTURECAPS_MIPVOLUMEMAP |
        D3DPTEXTURECAPS_PERSPECTIVE |
        D3DPTEXTURECAPS_VOLUMEMAP;
    caps->TextureFilterCaps = 
        D3DPTFILTERCAPS_MAGFPOINT |
        D3DPTFILTERCAPS_MAGFLINEAR |
        D3DPTFILTERCAPS_MAGFANISOTROPIC |
        D3DPTFILTERCAPS_MINFPOINT |
        D3DPTFILTERCAPS_MINFLINEAR |
        D3DPTFILTERCAPS_MINFANISOTROPIC |
        D3DPTFILTERCAPS_MIPFPOINT |
        D3DPTFILTERCAPS_MIPFLINEAR;
    caps->CubeTextureFilterCaps = caps->TextureFilterCaps;
    caps->VolumeTextureFilterCaps = caps->TextureFilterCaps;
    caps->TextureAddressCaps =
        D3DPTADDRESSCAPS_BORDER |
        D3DPTADDRESSCAPS_CLAMP |
        D3DPTADDRESSCAPS_INDEPENDENTUV |
        D3DPTADDRESSCAPS_MIRROR |
        D3DPTADDRESSCAPS_MIRRORONCE |
        D3DPTADDRESSCAPS_WRAP;
    caps->VolumeTextureAddressCaps = caps->TextureAddressCaps;
    caps->LineCaps =
        D3DLINECAPS_ALPHACMP |
        D3DLINECAPS_ANTIALIAS |
        D3DLINECAPS_BLEND |
        D3DLINECAPS_FOG |
        D3DLINECAPS_TEXTURE |
        D3DLINECAPS_ZTEST;
    caps->MaxTextureWidth = 65536;
    caps->MaxTextureHeight = 65536;
    caps->MaxVolumeExtent = 1024;
    caps->MaxTextureRepeat = 65536;
    caps->MaxTextureAspectRatio = 65536;
    caps->MaxAnisotropy = 1.0; // Todo: Find a real value
    caps->MaxVertexW = 1.0; // Todo: Find a real value
    caps->GuardBandLeft = 0.0;
    caps->GuardBandTop = 0.0;
    caps->GuardBandRight = 0.0;
    caps->GuardBandBottom = 0.0;
    caps->ExtentsAdjust = 0;
    caps->StencilCaps =
        D3DSTENCILCAPS_KEEP |
        D3DSTENCILCAPS_ZERO |
        D3DSTENCILCAPS_REPLACE |
        D3DSTENCILCAPS_INCRSAT |
        D3DSTENCILCAPS_DECRSAT |
        D3DSTENCILCAPS_INVERT |
        D3DSTENCILCAPS_INCR |
        D3DSTENCILCAPS_DECR;
    caps->FVFCaps = 0;
    // TODO: Fill out the rest of these.

    return S_OK;
}
