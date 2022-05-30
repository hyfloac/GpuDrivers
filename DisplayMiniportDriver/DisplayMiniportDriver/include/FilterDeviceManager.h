#pragma once

#include <ntddk.h>
#include <fltKernel.h>

#define HY_FILTER_PORT_COUNT (16)

/**
 * Initializes a linked list of PFLT_PORT's.
 */
NTSTATUS HyInitFilterDevices(void);

/**
 *   Stores a PFLT_PORT and returns a pointer to the stored member in
 * ConnectionPortCookie.
 */
NTSTATUS HyRegisterFilterClient(PFLT_PORT ClientPort, PVOID* ConnectionPortCookie);

/**
 *   Dereferences ConnectionPortCookie and returns the PFLT_PORT in
 * ClientPort.
 */
NTSTATUS HyGetFilterClient(PVOID ConnectionPortCookie, PFLT_PORT* ClientPort);

/**
 * Frees the linked list node that ConnectionPortCookie points to.
 */
NTSTATUS HyUnregisterFilterClient(PVOID ConnectionPortCookie);
