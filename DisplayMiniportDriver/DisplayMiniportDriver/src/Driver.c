// See https://docs.microsoft.com/en-us/windows-hardware/drivers/display/driverentry-of-display-miniport-driver
#ifdef __cplusplus
extern "C" {
#endif
    
#include <ntddk.h>
#include <dispmprt.h>
#include <process.h>

#include "DeviceComms.h"

#include "AddDevice.h"
#include "Logging.h"
#include "QueryAdapterInfo.h"
#include "RemoveDevice.h"
#include "StartDevice.h"
#include "StopDevice.h"
#include "SetPowerState.h"
#include "ResetDevice.h"
#include "SetVidPnSourceVisibility.h"
#include "StopDeviceAndReleasePostDisplayOwnership.h"
#include "QueryInterface.h"

#pragma code_seg(push)
#pragma code_seg("PAGE")

NTSTATUS DdiNoOpNTSTATUS()  // NOLINT(clang-diagnostic-strict-prototypes)
{
    LOG_DEBUG("DdiNoOpNTSTATUS\n");
    return STATUS_SUCCESS;
}

void DdiNoOpVoid(void)  // NOLINT(clang-diagnostic-strict-prototypes)
{
    LOG_DEBUG("DdiNoOpVoid\n");
}

BOOLEAN DdiNoOpBool(void)  // NOLINT(clang-diagnostic-strict-prototypes)
{
    LOG_DEBUG("DdiNoOpBool\n");
    return TRUE;
}
#pragma code_seg(pop)

#pragma code_seg("INIT")
DRIVER_INITIALIZE DriverEntry;

_Use_decl_annotations_ NTSTATUS DriverEntryReal(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    PAGED_CODE();

    LOG_DEBUG("DriverEntryReal\n");

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
    driverInitializationData.DxgkDdiDispatchIoRequest = (PDXGKDDI_DISPATCH_IO_REQUEST) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiInterruptRoutine = (PDXGKDDI_INTERRUPT_ROUTINE) DdiNoOpBool;
    driverInitializationData.DxgkDdiDpcRoutine = (PDXGKDDI_DPC_ROUTINE) DdiNoOpVoid;
    driverInitializationData.DxgkDdiQueryChildRelations = (PDXGKDDI_QUERY_CHILD_RELATIONS) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiQueryChildStatus = (PDXGKDDI_QUERY_CHILD_STATUS) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiQueryDeviceDescriptor = (PDXGKDDI_QUERY_DEVICE_DESCRIPTOR) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSetPowerState = HySetPowerState;
    driverInitializationData.DxgkDdiNotifyAcpiEvent = (PDXGKDDI_NOTIFY_ACPI_EVENT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiResetDevice = HyResetDevice;
    driverInitializationData.DxgkDdiUnload = DdiNoOpVoid;
    driverInitializationData.DxgkDdiQueryInterface = HyQueryInterface;
    driverInitializationData.DxgkDdiControlEtwLogging = (PDXGKDDI_CONTROL_ETW_LOGGING) DdiNoOpVoid;
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
    driverInitializationData.DxgkDdiSetPalette = (PDXGKDDI_SETPALETTE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSetPointerPosition = (PDXGKDDI_SETPOINTERPOSITION) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSetPointerShape = (PDXGKDDI_SETPOINTERSHAPE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiResetFromTimeout = (PDXGKDDI_RESETFROMTIMEOUT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiRestartFromTimeout = (PDXGKDDI_RESTARTFROMTIMEOUT) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiEscape = (PDXGKDDI_ESCAPE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiCollectDbgInfo = (PDXGKDDI_COLLECTDBGINFO) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiQueryCurrentFence = (PDXGKDDI_QUERYCURRENTFENCE) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiIsSupportedVidPn = (PDXGKDDI_ISSUPPORTEDVIDPN) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiRecommendFunctionalVidPn = (PDXGKDDI_RECOMMENDFUNCTIONALVIDPN) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiEnumVidPnCofuncModality = (PDXGKDDI_ENUMVIDPNCOFUNCMODALITY) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiSetVidPnSourceAddress = (PDXGKDDI_SETVIDPNSOURCEADDRESS) DdiNoOpNTSTATUS;

    driverInitializationData.DxgkDdiSetVidPnSourceVisibility = HySetVidPnSourceVisibility;

    driverInitializationData.DxgkDdiCommitVidPn = (PDXGKDDI_COMMITVIDPN) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiRecommendVidPnTopology = (PDXGKDDI_RECOMMENDVIDPNTOPOLOGY) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiGetScanLine = (PDXGKDDI_GETSCANLINE) DdiNoOpNTSTATUS;
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
}

_Use_decl_annotations_ NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    __security_init_cookie();
    return DriverEntryReal(DriverObject, RegistryPath);
}

#pragma code_seg()

#ifdef __cplusplus
} /* extern "C" */
#endif
