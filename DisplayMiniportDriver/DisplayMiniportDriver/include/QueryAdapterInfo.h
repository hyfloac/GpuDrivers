#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyQueryAdapterInfo(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo);

#define HY_PRIVATE_DRIVER_DATA_MAGIC (0x48794444)

#define HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION (1)

typedef struct HyPrivateDriverData
{
    // A magic value to verify that this is our data.
    UINT Magic; // HY_PRIVATE_DRIVER_DATA_MAGIC 0x48794444 HyDD
    // The size of this structure, this is an additional validation measure for checking that it is our data.
    UINT Size; // sizeof(HyPrivateDriverData)
    // An incremental version identifier, this is an additional validation measure for checking that we're communicating with our driver.
    UINT Version; // HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION
    
    // The size of our GPU's pages.
    ULONGLONG PageSize;
} HyPrivateDriverData;
