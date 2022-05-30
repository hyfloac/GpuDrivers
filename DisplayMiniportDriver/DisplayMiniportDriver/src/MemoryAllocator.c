#include <wdm.h>

#include "MemoryAllocator.h"

#ifndef USE_LEGACY_ALLOCATOR
  #define USE_LEGACY_ALLOCATOR (0)
#endif

static POOL_FLAGS HyConvertPoolTypeToFlags(const POOL_TYPE PoolType, BOOLEAN ZeroInit);

void* HyAllocate(const POOL_TYPE PoolType, const SIZE_T NumberOfBytes, const ULONG Tag)
{
    PAGED_CODE();

#if USE_LEGACY_ALLOCATOR
    return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
#else
    return ExAllocatePool2(HyConvertPoolTypeToFlags(PoolType, FALSE), NumberOfBytes, Tag);
#endif
}

void* HyAllocateZeroed(const POOL_TYPE PoolType, const SIZE_T NumberOfBytes, const ULONG Tag)
{
    PAGED_CODE();

#if USE_LEGACY_ALLOCATOR
    // Create the allocation.
    void* const allocation = ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);

    // Zero the buffer. This is done to match functionality with ExAllocatePool2.
    (void) RtlZeroMemory(allocation, NumberOfBytes);

    return allocation;
#else
    return ExAllocatePool2(HyConvertPoolTypeToFlags(PoolType, TRUE), NumberOfBytes, Tag);
#endif
}

void HyDeallocate(const PVOID P, const ULONG Tag)
{
    PAGED_CODE();

#if USE_LEGACY_ALLOCATOR
    ExFreePoolWithTag(P, Tag);
#else
    ExFreePool2(P, Tag, NULL, 0);
#endif
}

static POOL_FLAGS HyConvertPoolTypeToFlags(const POOL_TYPE PoolType, BOOLEAN ZeroInit)
{
    PAGED_CODE();

    POOL_FLAGS ret = 0;
    if((PoolType & NonPagedPoolExecute) == NonPagedPoolExecute)
    {
        ret |= POOL_FLAG_NON_PAGED_EXECUTE;
    }
    if((PoolType & NonPagedPoolNx) == NonPagedPoolNx)
    {
        ret |= POOL_FLAG_NON_PAGED;
    }
    if((PoolType & PagedPool) == PagedPool)
    {
        ret |= POOL_FLAG_PAGED;
    }
    if((PoolType & NonPagedPoolCacheAligned) == NonPagedPoolCacheAligned)
    {
        ret |= POOL_FLAG_NON_PAGED_EXECUTE | POOL_FLAG_CACHE_ALIGNED;
    }
    if((PoolType & NonPagedPoolNxCacheAligned) == NonPagedPoolNxCacheAligned)
    {
        ret |= POOL_FLAG_NON_PAGED | POOL_FLAG_CACHE_ALIGNED;
    }
    if((PoolType & PagedPoolCacheAligned) == PagedPoolCacheAligned)
    {
        ret |= POOL_FLAG_PAGED | POOL_FLAG_CACHE_ALIGNED;
    }
    if((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) == POOL_RAISE_IF_ALLOCATION_FAILURE)
    {
        ret |= POOL_FLAG_RAISE_ON_FAILURE;
    }
    if(!ZeroInit)
    {
        ret |= POOL_FLAG_UNINITIALIZED;
    }

    return ret;
}
