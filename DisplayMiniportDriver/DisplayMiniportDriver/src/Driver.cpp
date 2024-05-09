// See https://docs.microsoft.com/en-us/windows-hardware/drivers/display/driverentry-of-display-miniport-driver
#ifdef __cplusplus
extern "C" {
#endif
    
#include <ntddk.h>
#include <dispmprt.h>
#include <process.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "Logging.h"
#include "Config.h"

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "DeviceComms.hpp"

#include "AddDevice.hpp"
#include "StartDevice.hpp"
#include "StopDevice.hpp"
#include "RemoveDevice.hpp"
#include "DispatchIoRequest.hpp"
#include "InterruptRoutine.hpp"
#include "DpcRoutine.hpp"
#include "QueryChildRelations.hpp"
#include "QueryChildStatus.hpp"
#include "QueryDeviceDescriptor.hpp"
#include "SetPowerState.hpp"
#include "NotifyAcpiEvent.hpp"
#include "ResetDevice.hpp"
#include "QueryInterface.hpp"
#include "ControlEtwLogging.hpp"
#include "QueryAdapterInfo.hpp"

#include "RecommendFunctionalVidPn.hpp"
#include "EnumVidPnCofuncModality.hpp"

#include "CollectDbgInfo.hpp"
#include "IsSupportedVidPn.hpp"

#include "SetVidPnSourceVisibility.hpp"

#include "CommitVidPn.hpp"

#if HY_BUILD_AS_KMDOD
#include "UpdateActiveVidPnPresentPath.hpp"
#endif

#include "GetScanLine.hpp"

#if HY_BUILD_AS_KMDOD
#include "PresentDisplayOnly.hpp"
#endif

#include "StopDeviceAndReleasePostDisplayOwnership.hpp"
#include "ControlInterrupt.hpp"


#include "DevThunks.hpp"

#pragma code_seg(push)
#pragma code_seg("PAGE")

NTSTATUS DdiNoOpNTSTATUS()  // NOLINT(clang-diagnostic-strict-prototypes)
{
    LOG_DEBUG("DdiNoOpNTSTATUS\n");
    return STATUS_SUCCESS;
}

void DdiNoOpVoid()  // NOLINT(clang-diagnostic-strict-prototypes)
{
    LOG_DEBUG("DdiNoOpVoid\n");
}

BOOLEAN DdiNoOpBool()  // NOLINT(clang-diagnostic-strict-prototypes)
{
    LOG_DEBUG("DdiNoOpBool\n");
    return TRUE;
}

static void HyUnload(void)
{
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyUnload\n");
}

#pragma code_seg(pop)

#pragma code_seg("INIT")
#ifdef __cplusplus
extern "C" {
#endif
DRIVER_INITIALIZE DriverEntry;
#ifdef __cplusplus
} /* extern "C" */
#endif

_Use_decl_annotations_ NTSTATUS DriverEntryReal(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    PAGED_CODE();

    //
    // Opt-in to using non-executable pool memory on Windows 8 and later.
    // This has to be done before any allocations (which our logging functions can potentially do).
    // https://msdn.microsoft.com/en-us/library/windows/hardware/hh920402(v=vs.85).aspx
    //
    ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("DriverEntryReal - " __TIMESTAMP__ "\n");

    // If DriverObject is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!ARGUMENT_PRESENT(DriverObject))
    {
        LOG_ERROR("Invalid Parameter to DriverEntry: DriverObject\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If RegistryPath is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!ARGUMENT_PRESENT(RegistryPath))
    {
        LOG_ERROR("Invalid Parameter to DriverEntry: RegistryPath\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    {
        // Initialize the emulation comms if necessary.
        const NTSTATUS deviceCommsStatus = InitDeviceComms(DriverObject);
        if(!NT_SUCCESS(deviceCommsStatus))
        {
            return deviceCommsStatus;
        }
    }

#if HY_BUILD_AS_KMDOD
    // Allocate (on the stack) and zero out the list pointers required for Display Miniport Display-Only Driver.
    KMDDOD_INITIALIZATION_DATA driverInitializationData;
    (void) RtlZeroMemory(&driverInitializationData, sizeof(driverInitializationData));

    // Set the version to whatever is currently in the header we're compiling with.
    driverInitializationData.Version = DXGKDDI_INTERFACE_VERSION;

    driverInitializationData.DxgkDdiAddDevice = HyAddDevice;
    driverInitializationData.DxgkDdiStartDevice = HyStartDevice;
    driverInitializationData.DxgkDdiStopDevice = HyStopDevice;
    driverInitializationData.DxgkDdiRemoveDevice = HyRemoveDevice;
    driverInitializationData.DxgkDdiDispatchIoRequest = HyDispatchIoRequest;
    driverInitializationData.DxgkDdiInterruptRoutine = HyInterruptRoutine;
    driverInitializationData.DxgkDdiDpcRoutine = HyDpcRoutine;
    driverInitializationData.DxgkDdiQueryChildRelations = HyQueryChildRelations;
    driverInitializationData.DxgkDdiQueryChildStatus = HyQueryChildStatus;
    driverInitializationData.DxgkDdiQueryDeviceDescriptor = HyQueryDeviceDescriptor;
    driverInitializationData.DxgkDdiSetPowerState = HySetPowerState;
    driverInitializationData.DxgkDdiNotifyAcpiEvent = HyNotifyAcpiEvent;
    driverInitializationData.DxgkDdiResetDevice = HyResetDevice;
    driverInitializationData.DxgkDdiUnload = HyUnload;
    driverInitializationData.DxgkDdiQueryInterface = HyQueryInterface;
    driverInitializationData.DxgkDdiControlEtwLogging = HyControlEtwLogging;
    driverInitializationData.DxgkDdiQueryAdapterInfo = HyQueryAdapterInfo;

    driverInitializationData.DxgkDdiSetPalette = ThunkHySetPalette;
    driverInitializationData.DxgkDdiSetPointerPosition = ThunkHySetPointerPosition;
    driverInitializationData.DxgkDdiSetPointerShape = ThunkHySetPointerShape;
    driverInitializationData.DxgkDdiEscape = ThunkHyEscape;
    driverInitializationData.DxgkDdiCollectDbgInfo = HyCollectDbgInfo;
    driverInitializationData.DxgkDdiIsSupportedVidPn = HyIsSupportedVidPn;
    driverInitializationData.DxgkDdiRecommendFunctionalVidPn = HyRecommendFunctionalVidPn;
    driverInitializationData.DxgkDdiEnumVidPnCofuncModality = HyEnumVidPnCofuncModality;

    driverInitializationData.DxgkDdiSetVidPnSourceVisibility = HySetVidPnSourceVisibility;

    driverInitializationData.DxgkDdiCommitVidPn = HyCommitVidPn;
    driverInitializationData.DxgkDdiUpdateActiveVidPnPresentPath = HyUpdateActiveVidPnPresentPath;
    driverInitializationData.DxgkDdiRecommendMonitorModes = ThunkHyRecommendMonitorModes;
#if HY_KMDOD_ENABLE_VSYNC_INTERRUPTS
    driverInitializationData.DxgkDdiGetScanLine = HyGetScanLine;
#else
    driverInitializationData.DxgkDdiGetScanLine = nullptr;
#endif
    driverInitializationData.DxgkDdiQueryVidPnHWCapability = ThunkHyQueryVidPnCapability;

    driverInitializationData.DxgkDdiPresentDisplayOnly = HyPresentDisplayOnly;

    driverInitializationData.DxgkDdiStopDeviceAndReleasePostDisplayOwnership = HyStopDeviceAndReleasePostDisplayOwnership;

    driverInitializationData.DxgkDdiSystemDisplayEnable = ThunkHySystemDisplayEnable;
    driverInitializationData.DxgkDdiSystemDisplayWrite = ThunkHySystemDisplayWrite;

    driverInitializationData.DxgkDdiGetChildContainerId = ThunkHyGetChildContainerId;

#if HY_KMDOD_ENABLE_VSYNC_INTERRUPTS
    driverInitializationData.DxgkDdiControlInterrupt = HyControlInterrupt;
#else
    driverInitializationData.DxgkDdiControlInterrupt = nullptr;
#endif

    driverInitializationData.DxgkDdiSetPowerComponentFState = ThunkHySetPowerComponentFState;
    //driverInitializationData.DxgkDdiPowerRuntimeControlRequest = ThunkHyPowerRuntimeControlRequest;

    //driverInitializationData.DxgkDdiNotifySurpriseRemoval = ThunkHyNotifySurpriseRemoval;

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
    //
    // Display only drivers support P-State management.
    //
    driverInitializationData.DxgkDdiPowerRuntimeSetDeviceHandle = (PDXGKDDI_POWERRUNTIMESETDEVICEHANDLE) DdiNoOpNTSTATUS;
#endif

    // Initialize DXGK and return the result back to the kernel.
    return DxgkInitializeDisplayOnlyDriver(DriverObject, RegistryPath, &driverInitializationData);
#else
    // Allocate (on the stack) and zero out the list pointers required for Display Miniport Driver.
    DRIVER_INITIALIZATION_DATA driverInitializationData;
    (void) RtlZeroMemory(&driverInitializationData, sizeof(driverInitializationData));

    // // Set the version to whatever is currently in the header we're compiling with.
    // driverInitializationData.Version = DXGKDDI_INTERFACE_VERSION;
    driverInitializationData.Version = DXGKDDI_INTERFACE_VERSION_VISTA;

    driverInitializationData.DxgkDdiAddDevice = HyAddDevice;
    driverInitializationData.DxgkDdiStartDevice = HyStartDevice;
    driverInitializationData.DxgkDdiStopDevice = HyStopDevice;
    driverInitializationData.DxgkDdiRemoveDevice = HyRemoveDevice;
    driverInitializationData.DxgkDdiDispatchIoRequest = HyDispatchIoRequest;
    driverInitializationData.DxgkDdiInterruptRoutine = HyInterruptRoutine;
    driverInitializationData.DxgkDdiDpcRoutine = HyDpcRoutine;
    driverInitializationData.DxgkDdiQueryChildRelations = HyQueryChildRelations;
    driverInitializationData.DxgkDdiQueryChildStatus = HyQueryChildStatus;
    driverInitializationData.DxgkDdiQueryDeviceDescriptor = HyQueryDeviceDescriptor;
    driverInitializationData.DxgkDdiSetPowerState = HySetPowerState;
    driverInitializationData.DxgkDdiNotifyAcpiEvent = HyNotifyAcpiEvent;
    driverInitializationData.DxgkDdiResetDevice = HyResetDevice;
    driverInitializationData.DxgkDdiUnload = HyUnload;
    driverInitializationData.DxgkDdiQueryInterface = HyQueryInterface;
    driverInitializationData.DxgkDdiControlEtwLogging = HyControlEtwLogging;
    driverInitializationData.DxgkDdiQueryAdapterInfo = HyQueryAdapterInfo;
    driverInitializationData.DxgkDdiCreateDevice = (PDXGKDDI_CREATEDEVICE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiCreateAllocation = (PDXGKDDI_CREATEALLOCATION) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiDestroyAllocation = (PDXGKDDI_DESTROYALLOCATION) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiDescribeAllocation = (PDXGKDDI_DESCRIBEALLOCATION) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiGetStandardAllocationDriverData = (PDXGKDDI_GETSTANDARDALLOCATIONDRIVERDATA) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiAcquireSwizzlingRange = (PDXGKDDI_ACQUIRESWIZZLINGRANGE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiReleaseSwizzlingRange = (PDXGKDDI_RELEASESWIZZLINGRANGE) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiPatch = (PDXGKDDI_PATCH) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSubmitCommand = (PDXGKDDI_SUBMITCOMMAND) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiPreemptCommand = (PDXGKDDI_PREEMPTCOMMAND) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiBuildPagingBuffer = (PDXGKDDI_BUILDPAGINGBUFFER) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSetPalette = ThunkHySetPalette;
    driverInitializationData.DxgkDdiSetPointerPosition = ThunkHySetPointerPosition;
    driverInitializationData.DxgkDdiSetPointerShape = ThunkHySetPointerShape;
    driverInitializationData.DxgkDdiResetFromTimeout = (PDXGKDDI_RESETFROMTIMEOUT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiRestartFromTimeout = (PDXGKDDI_RESTARTFROMTIMEOUT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiEscape = ThunkHyEscape;
    driverInitializationData.DxgkDdiCollectDbgInfo = HyCollectDbgInfo;
    driverInitializationData.DxgkDdiQueryCurrentFence = (PDXGKDDI_QUERYCURRENTFENCE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiIsSupportedVidPn = HyIsSupportedVidPn;
    driverInitializationData.DxgkDdiRecommendFunctionalVidPn = HyRecommendFunctionalVidPn;
    driverInitializationData.DxgkDdiEnumVidPnCofuncModality = HyEnumVidPnCofuncModality;
    driverInitializationData.DxgkDdiSetVidPnSourceAddress = (PDXGKDDI_SETVIDPNSOURCEADDRESS) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiSetVidPnSourceVisibility = HySetVidPnSourceVisibility;

    driverInitializationData.DxgkDdiCommitVidPn = HyCommitVidPn;
    driverInitializationData.DxgkDdiRecommendVidPnTopology = (PDXGKDDI_RECOMMENDVIDPNTOPOLOGY) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiGetScanLine = HyGetScanLine;
    driverInitializationData.DxgkDdiStopCapture = (PDXGKDDI_STOPCAPTURE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiCreateOverlay = (PDXGKDDI_CREATEOVERLAY) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiDestroyDevice = (PDXGKDDI_DESTROYDEVICE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiOpenAllocation = (PDXGKDDI_OPENALLOCATIONINFO) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiCloseAllocation = (PDXGKDDI_CLOSEALLOCATION) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiRender = (PDXGKDDI_RENDER) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiPresent = (PDXGKDDI_PRESENT) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiUpdateOverlay = (PDXGKDDI_UPDATEOVERLAY) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiFlipOverlay = (PDXGKDDI_FLIPOVERLAY) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiDestroyOverlay = (PDXGKDDI_DESTROYOVERLAY) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiCreateContext = (PDXGKDDI_CREATECONTEXT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiDestroyContext = (PDXGKDDI_DESTROYCONTEXT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiLinkDevice = (PDXGKDDI_LINK_DEVICE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSetDisplayPrivateDriverFormat = (PDXGKDDI_SETDISPLAYPRIVATEDRIVERFORMAT) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiStopDeviceAndReleasePostDisplayOwnership = HyStopDeviceAndReleasePostDisplayOwnership;

    // Initialize DXGK and return the result back to the kernel.
    return DxgkInitialize(DriverObject, RegistryPath, &driverInitializationData);
#endif
}

#ifdef __cplusplus
extern "C" {
#endif

_Use_decl_annotations_ NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    __security_init_cookie();
    return DriverEntryReal(DriverObject, RegistryPath);
}

#pragma code_seg()

#ifdef __cplusplus
} /* extern "C" */
#endif
