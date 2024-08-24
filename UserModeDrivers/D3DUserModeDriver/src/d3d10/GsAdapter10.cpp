#include "d3d10/GsAdapter10.hpp"
#include "d3d10/GsDevice10.hpp"
#include <ConPrinter.hpp>
#include <new>

#include "Logging.hpp"

#include "d3d10/device/SetBlendState10.hpp"

#include "d3d10/device/CalcPrivateBlendStateSize10.hpp"
#include "d3d10/device/CreateBlendState10.hpp"

#include "d3d10/device/DestroyDevice10.hpp"

GsAdapter10::GsAdapter10(
    const D3D10DDI_HRTADAPTER runtimeHandle, 
    const D3DDDI_ADAPTERCALLBACKS& adapterCallbacks
) noexcept
    : m_RuntimeHandle(runtimeHandle)
    , m_AdapterCallbacks(adapterCallbacks)
{ }

SIZE_T GsAdapter10::CalcPrivateDeviceSize(const D3D10DDIARG_CALCPRIVATEDEVICESIZE& calcPrivateDeviceSize) noexcept
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT();
    }
#endif
    (void) calcPrivateDeviceSize;

    return sizeof(GsDevice10);
}

#define GEN_NOOP_VOID(PTR) \
    { \
        using fun_t = void(*)(); \
        fun_t fun = []() -> void \
        { \
            ConPrinter::PrintLn("{}", #PTR); \
        }; \
        (PTR) = reinterpret_cast<decltype(PTR)>(fun); \
    }

#define GEN_NOOP_BOOL(PTR) \
    { \
        using fun_t = BOOL(*)(); \
        fun_t fun = []() -> BOOL \
        { \
            ConPrinter::PrintLn("{}", #PTR); \
            return FALSE; \
        }; \
        (PTR) = reinterpret_cast<decltype(PTR)>(fun); \
    }

#define GEN_NOOP_SIZE(PTR) \
    { \
        using fun_t = SIZE_T(*)(); \
        fun_t fun = []() -> SIZE_T \
        { \
            ConPrinter::PrintLn("{}", #PTR); \
            return 0; \
        }; \
        (PTR) = reinterpret_cast<decltype(PTR)>(fun); \
    }

HRESULT GsAdapter10::CreateDevice(D3D10DDIARG_CREATEDEVICE& createDevice) noexcept
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT();
    }
#endif

    GsDevice10* device;
    //   It seems like in D3D10 creation of the device was changed to have
    // the D3D10 runtime allocate storage (using pfnCalcPrivateDeviceSize),
    // and then provide that buffer to the UMD.
    if(!createDevice.hDrvDevice.pDrvPrivate)
    {
        device = new(::std::nothrow) GsDevice10(
            createDevice.hRTDevice,     // driverHandle
            *createDevice.pKTCallbacks, // deviceCallbacks
            createDevice.hRTCoreLayer,  // runtimeCoreLayerHandle
            *createDevice.pUMCallbacks  // umCallbacks
        );

        createDevice.hDrvDevice.pDrvPrivate = device;
    }
    else
    {
        device = ::new(createDevice.hDrvDevice.pDrvPrivate) GsDevice10(
            createDevice.hRTDevice,     // driverHandle
            *createDevice.pKTCallbacks, // deviceCallbacks
            createDevice.hRTCoreLayer,  // runtimeCoreLayerHandle
            *createDevice.pUMCallbacks  // umCallbacks
        );
    }

    // Order of functions is in decreasing order of priority ( as far as performance is concerned ).
    // !!! BEGIN HIGH-FREQUENCY !!!
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDefaultConstantBufferUpdateSubresourceUP);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnVsSetConstantBuffers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnPsSetShaderResources);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnPsSetShader);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnPsSetSamplers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnVsSetShader);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDrawIndexed);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDraw);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicIABufferMapNoOverwrite);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicIABufferUnmap);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicConstantBufferMapDiscard);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicIABufferMapDiscard);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicConstantBufferUnmap);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnPsSetConstantBuffers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnIaSetInputLayout);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnIaSetVertexBuffers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnIaSetIndexBuffer);
    // !!! END HIGH-FREQUENCY !!!

    // Order of functions is in decreasing order of priority ( as far as performance is concerned ).
    // !!! BEGIN MIDDLE-FREQUENCY !!!
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDrawIndexedInstanced);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDrawInstanced);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicResourceMapDiscard);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDynamicResourceUnmap);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnGsSetConstantBuffers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnGsSetShader);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnIaSetTopology);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnStagingResourceMap);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnStagingResourceUnmap);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnVsSetShaderResources);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnVsSetSamplers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnGsSetShaderResources);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnGsSetSamplers);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetRenderTargets);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnShaderResourceViewReadAfterWriteHazard);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceReadAfterWriteHazard);
    // GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetBlendState);
    createDevice.pDeviceFuncs->pfnSetBlendState = GsSetBlendState10;
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetDepthStencilState);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetRasterizerState);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnQueryEnd);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnQueryBegin);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceCopyRegion);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceUpdateSubresourceUP);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSoSetTargets);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDrawAuto);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetViewports);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetScissorRects);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnClearRenderTargetView);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnClearDepthStencilView);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetPredication);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnQueryGetData);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnFlush);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnGenMips);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceCopy);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceResolveSubresource);
    // !!! END MIDDLE-FREQUENCY !!!

    // Infrequent paths:
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceMap);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnResourceUnmap);
    GEN_NOOP_BOOL(createDevice.pDeviceFuncs->pfnResourceIsStagingBusy);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnRelocateDeviceFuncs);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateResourceSize);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateOpenedResourceSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateResource);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnOpenResource);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyResource);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateShaderResourceViewSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateShaderResourceView);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyShaderResourceView);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateRenderTargetViewSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateRenderTargetView);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyRenderTargetView);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateDepthStencilViewSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateDepthStencilView);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyDepthStencilView);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateElementLayoutSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateElementLayout);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyElementLayout);
    // GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateBlendStateSize);
    createDevice.pDeviceFuncs->pfnCalcPrivateBlendStateSize = GsCalcPrivateBlendStateSize10;
    // GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateBlendState);
    createDevice.pDeviceFuncs->pfnCreateBlendState = GsCreateBlendState10;
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyBlendState);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateDepthStencilStateSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateDepthStencilState);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyDepthStencilState);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateRasterizerStateSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateRasterizerState);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyRasterizerState);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateShaderSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateVertexShader);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateGeometryShader);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreatePixelShader);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateGeometryShaderWithStreamOutput);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateGeometryShaderWithStreamOutput);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyShader);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateSamplerSize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateSampler);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroySampler);
    GEN_NOOP_SIZE(createDevice.pDeviceFuncs->pfnCalcPrivateQuerySize);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCreateQuery);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnDestroyQuery);

    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCheckFormatSupport);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCheckMultisampleQualityLevels);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCheckCounterInfo);
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnCheckCounter);

    createDevice.pDeviceFuncs->pfnDestroyDevice = GsDestroyDevice10;
    GEN_NOOP_VOID(createDevice.pDeviceFuncs->pfnSetTextFilterSize);

    return S_OK;
}

#undef GEN_NOOP_SIZE
#undef GEN_NOOP_BOOL
#undef GEN_NOOP_VOID
