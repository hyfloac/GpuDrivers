#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HySetVidPnSourceVisibility(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility);
