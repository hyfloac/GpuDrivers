#pragma once

#include <ntddk.h>
#include <dispmprt.h>

typedef struct
{
    PDEVICE_OBJECT PhysicalDeviceObject;
    DXGK_START_INFO DxgkStartInfo;
    DXGKRNL_INTERFACE DxgkInterface;
} HyMiniportDeviceContext;

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext);
