// ReSharper disable CppCStyleCast
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>

BOOLEAN RegisterMemCopyVV32(volatile void* const Destination, const volatile void* const Source, const UINT64 ByteCount);

BOOLEAN RegisterMemCopyNV32(void* const Destination, const volatile void* const Source, const UINT64 ByteCount);

#ifdef __cplusplus
} /* extern "C" */
#endif
