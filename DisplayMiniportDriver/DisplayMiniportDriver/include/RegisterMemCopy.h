// ReSharper disable CppCStyleCast
#pragma once

#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN RegisterMemCopyVV32(volatile void* const Destination, const volatile void* const Source, const UINT64 ByteCount);

BOOLEAN RegisterMemCopyNV32(void* const Destination, const volatile void* const Source, const UINT64 ByteCount);

#ifdef __cplusplus
} /* extern "C" */
#endif
