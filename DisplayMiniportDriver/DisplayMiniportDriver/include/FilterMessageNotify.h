#pragma once

#include <ntddk.h>
#include <fltKernel.h>

NTSTATUS HyFilterMessageNotify(PVOID PortCookie, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength, PULONG ReturnOutputBufferLength);
