#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyQueryInterface(IN_CONST_PVOID MiniportDeviceContext, IN_PQUERY_INTERFACE QueryInterface);
