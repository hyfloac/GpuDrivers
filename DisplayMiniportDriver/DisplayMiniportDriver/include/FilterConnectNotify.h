#pragma once

#include <ntddk.h>
#include <fltKernel.h>

NTSTATUS HyFilterConnectNotify(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID* ConnectionPortCookie);
