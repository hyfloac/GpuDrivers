#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif


NTSTATUS HyIsSupportedVidPn(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_ISSUPPORTEDVIDPN pIsSupportedVidPn);
