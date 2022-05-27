#pragma once

#include <ntddk.h>
#include <fltKernel.h>

NTSTATUS HyFilterInstanceQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags);
