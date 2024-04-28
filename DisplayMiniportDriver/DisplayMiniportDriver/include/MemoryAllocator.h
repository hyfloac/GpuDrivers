#pragma once

#include <wdm.h>

#define POOL_TAG_DEVICE_CONTEXT ('CDSG')
#define POOL_TAG_LOGGING        ('gLSG')

void* HyAllocate(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag);
void* HyAllocateZeroed(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag);

void HyDeallocate(PVOID P, ULONG Tag);

#define HY_ALLOC(Type, PoolType, Tag)      HyAllocate(PoolType, sizeof(Type), Tag)
#define HY_ALLOC_ZERO(Type, PoolType, Tag) HyAllocateZeroed(PoolType, sizeof(Type), Tag)
#define HY_FREE(Ptr, Tag)                  HyDeallocate(Ptr, Tag)