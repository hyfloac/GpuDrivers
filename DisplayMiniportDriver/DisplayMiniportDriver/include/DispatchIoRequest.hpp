#pragma once

#include "Common.h"

NTSTATUS HyDispatchIoRequest(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG VidPnSourceId, IN_PVIDEO_REQUEST_PACKET VideoRequestPacket);
