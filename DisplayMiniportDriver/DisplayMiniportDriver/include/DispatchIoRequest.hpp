#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif


NTSTATUS HyDispatchIoRequest(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG VidPnSourceId, IN_PVIDEO_REQUEST_PACKET VideoRequestPacket);

