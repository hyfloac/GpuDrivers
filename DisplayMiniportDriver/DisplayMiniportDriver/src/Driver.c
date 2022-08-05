// See https://docs.microsoft.com/en-us/windows-hardware/drivers/display/driverentry-of-display-miniport-driver
#ifdef __cplusplus
extern "C" {
#endif
    
#include <ntddk.h>
#include <dispmprt.h>

#include "DeviceComms.h"

#include "AddDevice.h"
#include "QueryAdapterInfo.h"
#include "RemoveDevice.h"
#include "StartDevice.h"
#include "StopDevice.h"
#include "SetPowerState.h"
#include "SetVidPnSourceVisibility.h"
#include "StopDeviceAndReleasePostDisplayOwnership.h"

#pragma code_seg(push)
#pragma code_seg("PAGE")

NTSTATUS DdiNoOpNTSTATUS()  // NOLINT(clang-diagnostic-strict-prototypes)
{
    return STATUS_SUCCESS;
}

void DdiNoOpVoid()  // NOLINT(clang-diagnostic-strict-prototypes)
{ }
#pragma code_seg(pop)

#pragma code_seg("INIT")
DRIVER_INITIALIZE DriverEntry;

_Use_decl_annotations_ NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    PAGED_CODE();

    // If DriverObject is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!ARGUMENT_PRESENT(DriverObject))
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // If RegistryPath is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!ARGUMENT_PRESENT(RegistryPath))
    {
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

    // Set the version to whatever is currently in the header we're compiling with.
    driverInitializationData.Version = DXGKDDI_INTERFACE_VERSION;

    driverInitializationData.DxgkDdiAddDevice = HyAddDevice;
    driverInitializationData.DxgkDdiStartDevice = HyStartDevice;
    driverInitializationData.DxgkDdiStopDevice = HyStopDevice;
    driverInitializationData.DxgkDdiRemoveDevice = HyRemoveDevice;
    driverInitializationData.DxgkDdiDispatchIoRequest = (PDXGKDDI_DISPATCH_IO_REQUEST) DdiNoOpNTSTATUS;
    driverInitializationData.DxgkDdiInterruptRoutine = NULL;
    driverInitializationData.DxgkDdiDpcRoutine = NULL;

    driverInitializationData.DxgkDdiSetPowerState = HySetPowerState;

    driverInitializationData.DxgkDdiQueryAdapterInfo = HyQueryAdapterInfo;

    driverInitializationData.DxgkDdiSetVidPnSourceVisibility = HySetVidPnSourceVisibility;

    driverInitializationData.DxgkDdiStopDeviceAndReleasePostDisplayOwnership = HyStopDeviceAndReleasePostDisplayOwnership;

    // Initialize DXGK and return the result back to the kernel.
    return DxgkInitialize(DriverObject, RegistryPath, &driverInitializationData);
}

#ifdef __cplusplus
} /* extern "C" */
#endif
