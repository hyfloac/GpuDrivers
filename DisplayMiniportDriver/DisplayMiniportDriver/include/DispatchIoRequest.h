#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyDispatchIoRequest(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG VidPnSourceId, IN_PVIDEO_REQUEST_PACKET VideoRequestPacket);

