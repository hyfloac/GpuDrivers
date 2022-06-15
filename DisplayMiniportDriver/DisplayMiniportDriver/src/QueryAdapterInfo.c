// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_queryadapterinfo
#include <ntddk.h>
#include <dispmprt.h>

#include "QueryAdapterInfo.h"

#pragma code_seg("PAGE")

static NTSTATUS FillUmDriverPrivate(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo);
static NTSTATUS FillDriverCaps(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo);

NTSTATUS HyQueryAdapterInfo(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    PAGED_CODE();

    // If MiniportDeviceContext (hAdapter) is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pQueryAdapterInfo is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pQueryAdapterInfo)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    switch(pQueryAdapterInfo->Type)
    {
        case DXGKQAITYPE_UMDRIVERPRIVATE: return FillUmDriverPrivate(hAdapter, pQueryAdapterInfo);
        case DXGKQAITYPE_DRIVERCAPS: return FillDriverCaps(hAdapter, pQueryAdapterInfo);
        default: return STATUS_NOT_IMPLEMENTED;
    }

    return STATUS_SUCCESS;
}

static NTSTATUS FillUmDriverPrivate(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    PAGED_CODE();

    // Validate that the input data is not null.
    if(!pQueryAdapterInfo->pInputData)
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Validate that the output data is not null.
    if(!pQueryAdapterInfo->pOutputData)
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Validate that the input data size matches our internal data type.
    if(pQueryAdapterInfo->InputDataSize != sizeof(HyPrivateDriverData))
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the output data size matches our internal data type.
    if(pQueryAdapterInfo->OutputDataSize != sizeof(HyPrivateDriverData))
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    const HyPrivateDriverData* const inputDriverData = pQueryAdapterInfo->pInputData;
    HyPrivateDriverData* const outputDriverData = pQueryAdapterInfo->pOutputData;

    // Validate that the input data Magic value matches our Magic value.
    if(inputDriverData->Magic != HY_PRIVATE_DRIVER_DATA_MAGIC)
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the internally stored size matches our internal data type.
    if(inputDriverData->Size != sizeof(HyPrivateDriverData))
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the Miniport Display Driver and the User Mode Driver are using the same version of the private data.
    if(inputDriverData->Version != HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION)
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Set the same basic data for output
    outputDriverData->Magic = HY_PRIVATE_DRIVER_DATA_MAGIC;
    outputDriverData->Size = sizeof(HyPrivateDriverData);
    outputDriverData->Version = HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION;

    // Set the page size to 64KiB.
    outputDriverData->PageSize = 65536;

    return STATUS_SUCCESS;
}

static NTSTATUS FillDriverCaps(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    PAGED_CODE();

    if(!pQueryAdapterInfo->pOutputData)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if(pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_DRIVERCAPS))
    {
        return STATUS_INVALID_PARAMETER;
    }

    DXGK_DRIVERCAPS* const driverCaps = pQueryAdapterInfo->pOutputData;

    driverCaps->MaxPointerWidth = 256;
    driverCaps->MaxPointerHeight = 256;
    driverCaps->PointerCaps.Monochrome = TRUE;
    driverCaps->PointerCaps.Color = TRUE;
    driverCaps->PointerCaps.MaskedColor = FALSE;
    driverCaps->PointerCaps.Reserved = 0;

    driverCaps->MemoryManagementCaps.IoMmuSupported = TRUE;

    driverCaps->SupportNonVGA = TRUE;

    return STATUS_SUCCESS;
}
