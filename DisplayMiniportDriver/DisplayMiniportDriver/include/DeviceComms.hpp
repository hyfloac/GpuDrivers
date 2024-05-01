#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>

#ifdef __cplusplus
} /* extern "C" */
#endif


NTSTATUS InitDeviceComms(IN PDRIVER_OBJECT DriverObject);
