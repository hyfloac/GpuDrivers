#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

void HyI2CInterfaceReference(PVOID Context);
void HyI2CInterfaceDereference(PVOID Context);
NTSTATUS HyI2CInterfaceTransmitDataToDisplay(PVOID MiniportDeviceContext, D3DDDI_VIDEO_PRESENT_TARGET_ID VidPnTargetId, ULONG SevenBitI2CAddress, ULONG DataLength, const void* Data);
NTSTATUS HyI2CInterfaceReceiveDataFromDisplay(PVOID MiniportDeviceContext, D3DDDI_VIDEO_PRESENT_TARGET_ID VidPnTargetId, ULONG SevenBitI2CAddress, ULONG Flags, ULONG DataLength, PVOID Data);

#ifdef __cplusplus
} /* extern "C" */
#endif
