#ifdef __cplusplus
extern "C" {
#endif

#include <fltKernel.h>
#include <ntstrsafe.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "DeviceComms.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include "FilterDeviceManager.h"
#include "FilterUnload.h"
#include "FilterInstanceSetup.h"
#include "FilterInstanceQueryTeardown.h"
#include "FilterConnectNotify.h"
#include "FilterDisconnectNotify.h"
#include "FilterMessageNotify.h"
#ifdef __cplusplus
} /* extern "C" */
#endif
#include "Logging.h"

#ifndef HY_USE_EMULATION
  #define HY_USE_EMULATION (0)
#endif

PFLT_FILTER FilterHandle;

NTSTATUS InitDeviceComms(IN PDRIVER_OBJECT DriverObject)
{
    (void) DriverObject;

    PAGED_CODE();

    LOG_DEBUG("InitDeviceComms\n");

#if HY_USE_EMULATION
    // Init the filter port linked list.
    HyInitFilterDevices();

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
    
    const NTSTATUS filterRegistrationStatus = FltRegisterFilter(DriverObject, &filterRegistrationData, &FilterHandle);

    // Propagate errors.
    if(!NT_SUCCESS(filterRegistrationStatus))
    {
        return filterRegistrationStatus;
    }

    // Initialize the name of our port.
    UNICODE_STRING portName;
    const NTSTATUS unicodeStringStatus = RtlUnicodeStringInit(&portName, L"\\EmuGpuPort");

    if(!NT_SUCCESS(unicodeStringStatus))
    {
        return unicodeStringStatus;
    }

    // Initialize the default security descriptor.
    PSECURITY_DESCRIPTOR commSecurityDesc;
    const NTSTATUS defaultSecurityDescStatus = FltBuildDefaultSecurityDescriptor(&commSecurityDesc, FLT_PORT_CONNECT);

    if(!NT_SUCCESS(defaultSecurityDescStatus))
    {
        return defaultSecurityDescStatus;
    }

    // Initialize the filter attributes.
    OBJECT_ATTRIBUTES commPortObjectAttribs;
    InitializeObjectAttributes(&commPortObjectAttribs, &portName, OBJ_KERNEL_HANDLE, NULL, commSecurityDesc);

    PFLT_PORT ServerPort;

    // Create the port for the user mode emulator to connect to.
    const NTSTATUS filterCreateCommPortStatus = FltCreateCommunicationPort(
        FilterHandle,
        &ServerPort,
        &commPortObjectAttribs,
        NULL,
        HyFilterConnectNotify,
        HyFilterDisconnectNotify,
        HyFilterMessageNotify,
        HY_FILTER_PORT_COUNT
    );

    if(!NT_SUCCESS(filterCreateCommPortStatus))
    {
        return filterCreateCommPortStatus;
    }

    // Start accepting filter requests.
    FltStartFiltering(FilterHandle);
#else
    LOG_DEBUG("Device comms are not used for a real device.\n");
#endif

    return STATUS_SUCCESS;
}
