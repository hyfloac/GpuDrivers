#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

NTSTATUS HyStopDeviceAndReleasePostDisplayOwnership(IN_CONST_PVOID MiniportDeviceContext, IN_CONST_D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId, PDXGK_DISPLAY_INFORMATION DisplayInfo);
