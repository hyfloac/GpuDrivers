// ReSharper disable CppClangTidyModernizeMacroToEnum
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <wdm.h>

#define POOL_TAG_DEVICE_CONTEXT   ('CDSG')
#define POOL_TAG_LOGGING          ('gLSG')
#define POOL_TAG_PRESENT          ('rPSG')
#define POOL_TAG_GRAPHICS_CONTEXT ('CGSG')
#define POOL_TAG_RESOURCE         ('sRSG')

void* HyAllocate(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag);
void* HyAllocateZeroed(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag);

void HyDeallocate(PVOID P, ULONG Tag);

#ifdef __cplusplus
  #define HY_ALLOC(Type, PoolType, Tag)      static_cast<Type*>(HyAllocate(PoolType, sizeof(Type), Tag))
  #define HY_ALLOC_ZERO(Type, PoolType, Tag) static_cast<Type*>(HyAllocateZeroed(PoolType, sizeof(Type), Tag))
#else
  #define HY_ALLOC(Type, PoolType, Tag)      HyAllocate(PoolType, sizeof(Type), Tag)
  #define HY_ALLOC_ZERO(Type, PoolType, Tag) HyAllocateZeroed(PoolType, sizeof(Type), Tag)
#endif
#define HY_FREE(Ptr, Tag)                  HyDeallocate(Ptr, Tag)


#ifdef __cplusplus
} /* extern "C" */
#endif
