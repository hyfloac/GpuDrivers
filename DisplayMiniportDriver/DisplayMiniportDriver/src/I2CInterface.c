#include "I2CInterface.h"

#pragma code_seg("PAGE")

void HyI2CInterfaceReference(PVOID Context)
{
    (void) Context;

    PAGED_CODE()
}

void HyI2CInterfaceDereference(PVOID Context)
{
    (void) Context;
    PAGED_CODE()
}

NTSTATUS HyI2CInterfaceTransmitDataToDisplay(PVOID MiniportDeviceContext, D3DDDI_VIDEO_PRESENT_TARGET_ID VidPnTargetId, ULONG SevenBitI2CAddress, ULONG DataLength, const void* Data)
{
    (void) MiniportDeviceContext;
    (void) VidPnTargetId;
    (void) SevenBitI2CAddress;
    (void) DataLength;
    (void) Data;

    PAGED_CODE();
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS HyI2CInterfaceReceiveDataFromDisplay(PVOID MiniportDeviceContext, D3DDDI_VIDEO_PRESENT_TARGET_ID VidPnTargetId, ULONG SevenBitI2CAddress, ULONG Flags, ULONG DataLength, PVOID Data)
{
    (void) MiniportDeviceContext;
    (void) VidPnTargetId;
    (void) SevenBitI2CAddress;
    (void) Flags;
    (void) DataLength;
    (void) Data;

    PAGED_CODE();
    return STATUS_NOT_IMPLEMENTED;
}
