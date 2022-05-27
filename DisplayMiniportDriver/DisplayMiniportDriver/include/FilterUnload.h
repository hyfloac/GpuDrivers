#pragma once

#include <ntddk.h>
#include <fltKernel.h>

NTSTATUS HyFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
