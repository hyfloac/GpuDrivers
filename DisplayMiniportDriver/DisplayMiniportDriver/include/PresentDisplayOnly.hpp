#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

NTSTATUS HyPresentDisplayOnly(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_PRESENT_DISPLAYONLY pPresentDisplayOnly);
