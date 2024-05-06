#include <DevThunks.hpp>

#pragma code_seg("PAGE")

NTSTATUS ThunkHySetPalette(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_SETPALETTE    pSetPalette
)
{
    (void) hAdapter;
    (void) pSetPalette;
    LOG_DEBUG("ThunkHySetPalette\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHySetPointerPosition(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_SETPOINTERPOSITION    pSetPointerPosition
)
{
    (void) hAdapter;
    (void) pSetPointerPosition;
    LOG_DEBUG("ThunkHySetPointerPosition\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHySetPointerShape(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_SETPOINTERSHAPE   pSetPointerShape
)
{
    (void) hAdapter;
    (void) pSetPointerShape;
    LOG_DEBUG("ThunkHySetPointerShape\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyEscape(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_ESCAPE        pEscape
)
{
    (void) hAdapter;
    (void) pEscape;
    LOG_DEBUG("ThunkHyEscape\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyCollectDbgInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_COLLECTDBGINFO        pCollectDbgInfo
)
{
    (void) hAdapter;
    (void) pCollectDbgInfo;
    LOG_DEBUG("ThunkHyCollectDbgInfo, Reason: 0x%08X\n", pCollectDbgInfo->Reason);

    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyRecommendMonitorModes(
    IN_CONST_HANDLE                                 hAdapter,
    IN_CONST_PDXGKARG_RECOMMENDMONITORMODES_CONST   pRecommendMonitorModes
)
{
    (void) hAdapter;
    (void) pRecommendMonitorModes;
    LOG_DEBUG("ThunkHyRecommendMonitorModes\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyGetScanLine(
    IN_CONST_HANDLE             hAdapter,
    INOUT_PDXGKARG_GETSCANLINE  pGetScanLine
)
{
    (void) hAdapter;
    (void) pGetScanLine;
    LOG_DEBUG("ThunkHyGetScanLine\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyQueryVidPnCapability(
    IN_CONST_HANDLE                             hAdapter,
    INOUT_PDXGKARG_QUERYVIDPNHWCAPABILITY       pVidPnHWCaps
)
{
    (void) hAdapter;
    (void) pVidPnHWCaps;
    LOG_DEBUG("ThunkHyQueryVidPnCapability\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHySystemDisplayEnable(
    _In_ PVOID MiniportDeviceContext,
    _In_ D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId,
    _In_ PDXGKARG_SYSTEM_DISPLAY_ENABLE_FLAGS Flags,
    _Out_ UINT* Width,
    _Out_ UINT* Height,
    _Out_ D3DDDIFORMAT* ColorFormat
)
{
    (void) MiniportDeviceContext;
    (void) TargetId;
    (void) Flags;
    (void) Width;
    (void) Height;
    (void) ColorFormat;
    LOG_DEBUG("ThunkHySystemDisplayEnable\n");
    return STATUS_SUCCESS;
}

void ThunkHySystemDisplayWrite(
    _In_ PVOID MiniportDeviceContext,
    _In_reads_bytes_(SourceHeight* SourceStride) PVOID Source,
    _In_ UINT SourceWidth,
    _In_ UINT SourceHeight,
    _In_ UINT SourceStride,
    _In_ UINT PositionX,
    _In_ UINT PositionY
)
{
    (void) MiniportDeviceContext;
    (void) Source;
    (void) SourceWidth;
    (void) SourceHeight;
    (void) SourceStride;
    (void) PositionX;
    (void) PositionY;
    LOG_DEBUG("ThunkHySystemDisplayWrite\n");
}


NTSTATUS ThunkHyGetChildContainerId(
    _In_ PVOID MiniportDeviceContext,
    _In_ ULONG ChildUid,
    _Inout_ PDXGK_CHILD_CONTAINER_ID ContainerId
)
{
    (void) MiniportDeviceContext;
    (void) ChildUid;
    (void) ContainerId;
    LOG_DEBUG("ThunkHyGetChildContainerId\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyControlInterrupt(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_DXGK_INTERRUPT_TYPE    InterruptType,
    IN_BOOLEAN                      EnableInterrupt
)
{
    (void) hAdapter;
    (void) InterruptType;
    (void) EnableInterrupt;
    LOG_DEBUG("ThunkHyControlInterrupt, Type: %d, Enable: %d\n", InterruptType, EnableInterrupt);
    //return STATUS_SUCCESS;
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS ThunkHySetPowerComponentFState(
    IN_CONST_HANDLE DriverContext,
    UINT            ComponentIndex,
    UINT            FState
)
{
    (void) DriverContext;
    (void) ComponentIndex;
    (void) FState;
    LOG_DEBUG("ThunkHySetPowerComponentFState\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyPowerRuntimeControlRequest(
    IN_CONST_HANDLE DriverContext,
    IN              LPCGUID PowerControlCode,
    IN OPTIONAL     PVOID InBuffer,
    IN              SIZE_T InBufferSize,
    OUT OPTIONAL    PVOID OutBuffer,
    IN              SIZE_T OutBufferSize,
    OUT OPTIONAL    PSIZE_T BytesReturned
)
{
    (void) DriverContext;
    (void) PowerControlCode;
    (void) InBuffer;
    (void) InBufferSize;
    (void) OutBuffer;
    (void) OutBufferSize;
    (void) BytesReturned;
    LOG_DEBUG("ThunkHyPowerRuntimeControlRequest\n");
    return STATUS_SUCCESS;
}

NTSTATUS ThunkHyNotifySurpriseRemoval(
    _In_ PVOID MiniportDeviceContext,
    _In_ DXGK_SURPRISE_REMOVAL_TYPE RemovalType
)
{
    (void) MiniportDeviceContext;
    (void) RemovalType;
    LOG_DEBUG("ThunkHyNotifySurpriseRemoval\n");
    return STATUS_SUCCESS;
}
