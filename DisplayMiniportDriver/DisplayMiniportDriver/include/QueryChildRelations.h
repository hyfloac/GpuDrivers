#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyQueryChildRelations(IN_CONST_PVOID MiniportDeviceContext, PDXGK_CHILD_DESCRIPTOR ChildRelations, ULONG ChildRelationsSize);
