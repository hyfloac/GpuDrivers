#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif


NTSTATUS HyStartDevice(IN_CONST_PVOID MiniportDeviceContext, IN_PDXGK_START_INFO DxgkStartInfo, IN_PDXGKRNL_INTERFACE DxgkInterface, OUT_PULONG NumberOfVideoPresentSurfaces, OUT_PULONG NumberOfChildren);
