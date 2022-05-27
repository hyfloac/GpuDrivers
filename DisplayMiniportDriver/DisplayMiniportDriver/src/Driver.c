// See https://docs.microsoft.com/en-us/windows-hardware/drivers/display/driverentry-of-display-miniport-driver
#ifdef __cplusplus
extern "C" {
#endif
    
#include <ntddk.h>
#include <dispmprt.h>
#include <fltKernel.h>

#include "FilterUnload.h"
#include "FilterInstanceSetup.h"
#include "FilterInstanceQueryTeardown.h"

#include "AddDevice.h"
#include "RemoveDevice.h"
#include "StartDevice.h"

DRIVER_INITIALIZE DriverEntry;
PFLT_FILTER FilterHandle;

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
        // Allocate (on the stack) and zero out the Filter Registration data.
        FLT_REGISTRATION filterRegistrationData;
        (void) RtlZeroMemory(&filterRegistrationData, sizeof(filterRegistrationData));

        // Set the Filter Registration Structure size and version.
        filterRegistrationData.Size = (USHORT) sizeof(filterRegistrationData);
        // Always use the latest version from WDK when compiling.
        filterRegistrationData.Version = FLT_REGISTRATION_VERSION;

        // We don't need any of the supported flags.
        filterRegistrationData.Flags = 0;

        // For now we're not using contexts or registrations, not until I figure out if they're needed and what for.
        filterRegistrationData.ContextRegistration = NULL;
        filterRegistrationData.OperationRegistration = NULL;

        // Set the unload callback, currently this always succeeds.
        filterRegistrationData.FilterUnloadCallback = HyFilterUnload;
        // We'll only let manual connections, namely from our emulated device.
        filterRegistrationData.InstanceSetupCallback = HyFilterInstanceSetup;
        // Set the query teardown callback, currently this always succeeds.
        filterRegistrationData.InstanceQueryTeardownCallback = HyFilterInstanceQueryTeardown;
        // We currently don't care when an instance is being torn down.
        filterRegistrationData.InstanceTeardownStartCallback = NULL;
        filterRegistrationData.InstanceTeardownCompleteCallback = NULL;
        // We currently don't care what the generated file name is.
        filterRegistrationData.GenerateFileNameCallback = NULL;
        filterRegistrationData.NormalizeNameComponentCallback = NULL;
        filterRegistrationData.NormalizeContextCleanupCallback = NULL;
        // We currently don't use transactions.
        filterRegistrationData.TransactionNotificationCallback = NULL;
        // We still don't care about file name generation.
        filterRegistrationData.NormalizeNameComponentExCallback = NULL;
        // We don't care about section notifications.
        filterRegistrationData.SectionNotificationCallback = NULL;

        PFLT_FILTER FilterHandle;
        const NTSTATUS filterRegistrationStatus = FltRegisterFilter(DriverObject, &filterRegistrationData, &FilterHandle);

        // Propagate errors.
        if(!NT_SUCCESS(filterRegistrationStatus))
        {
            return filterRegistrationStatus;
        }

        // Start accepting filter requests.
        FltStartFiltering(FilterHandle);
    }



    // Allocate (on the stack) and zero out the list pointers required for Display Miniport Driver.
    DRIVER_INITIALIZATION_DATA driverInitializationData;
    (void) RtlZeroMemory(&driverInitializationData, sizeof(driverInitializationData));

    // Set the version to whatever is currently in the header we're compiling with.
    driverInitializationData.Version = DXGKDDI_INTERFACE_VERSION;

    driverInitializationData.DxgkDdiAddDevice = HyAddDevice;
    driverInitializationData.DxgkDdiStartDevice = HyStartDevice;
    driverInitializationData.DxgkDdiStopDevice = NULL;
    driverInitializationData.DxgkDdiRemoveDevice = HyRemoveDevice;

    // Initialize DXGK and return the result back to the kernel.
    return DxgkInitialize(DriverObject, RegistryPath, &driverInitializationData);
}

#ifdef __cplusplus
} /* extern "C" */
#endif
