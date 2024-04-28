#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyQueryChildStatus(IN_CONST_PVOID MiniportDeviceContext, INOUT_PDXGK_CHILD_STATUS ChildStatus, IN_BOOLEAN NonDestructiveOnly);
