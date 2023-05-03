// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_queryadapterinfo
#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "QueryAdapterInfo.h"
#include "Logging.h"

#pragma code_seg("PAGE")

static NTSTATUS FillUmDriverPrivate(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo);
static NTSTATUS FillDriverCaps(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo);
static NTSTATUS FillQuerySegment(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo);

NTSTATUS HyQueryAdapterInfo(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    PAGED_CODE();

    LOG_DEBUG("HyQueryAdapterInfo\n");

    // If MiniportDeviceContext (hAdapter) is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HyQueryAdapterInfo: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pQueryAdapterInfo is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pQueryAdapterInfo)
    {
        LOG_ERROR("Invalid Parameter to HyQueryAdapterInfo: pQueryAdapterInfo\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    switch(pQueryAdapterInfo->Type)
    {
        case DXGKQAITYPE_UMDRIVERPRIVATE: return FillUmDriverPrivate(hAdapter, pQueryAdapterInfo);
        case DXGKQAITYPE_DRIVERCAPS: return FillDriverCaps(hAdapter, pQueryAdapterInfo);
        case DXGKQAITYPE_QUERYSEGMENT: return FillQuerySegment(hAdapter, pQueryAdapterInfo);
        default: return STATUS_NOT_IMPLEMENTED;
    }

    // return STATUS_SUCCESS;
}

static NTSTATUS FillUmDriverPrivate(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    (void) hAdapter;

    PAGED_CODE();

    // Validate that the input data is not null.
    if(!pQueryAdapterInfo->pInputData)
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: pQueryAdapterInfo->pInputData\n");
        return STATUS_INVALID_PARAMETER;
    }

    // Validate that the output data is not null.
    if(!pQueryAdapterInfo->pOutputData)
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: pQueryAdapterInfo->pOutputData\n");
        return STATUS_INVALID_PARAMETER;
    }

    // Validate that the input data size matches our internal data type.
    if(pQueryAdapterInfo->InputDataSize != sizeof(HyPrivateDriverData))
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: pQueryAdapterInfo->InputDataSize != sizeof(HyPrivateDriverData) [%z]\n", sizeof(HyPrivateDriverData));
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the output data size matches our internal data type.
    if(pQueryAdapterInfo->OutputDataSize != sizeof(HyPrivateDriverData))
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: pQueryAdapterInfo->OutputDataSize != sizeof(HyPrivateDriverData) [%z]\n", sizeof(HyPrivateDriverData));
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    const HyPrivateDriverData* const inputDriverData = pQueryAdapterInfo->pInputData;
    HyPrivateDriverData* const outputDriverData = pQueryAdapterInfo->pOutputData;

    // Validate that the input data Magic value matches our Magic value.
    if(inputDriverData->Magic != HY_PRIVATE_DRIVER_DATA_MAGIC)
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: inputDriverData->Magic != HY_PRIVATE_DRIVER_DATA_MAGIC [0x%08X]\n", HY_PRIVATE_DRIVER_DATA_MAGIC);
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the internally stored size matches our internal data type.
    if(inputDriverData->Size != sizeof(HyPrivateDriverData))
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: inputDriverData->Size != sizeof(HyPrivateDriverData) [%z]\n", sizeof(HyPrivateDriverData));
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the Miniport Display Driver and the User Mode Driver are using the same version of the private data.
    if(inputDriverData->Version != HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION)
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: inputDriverData->Version != HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION [0x%08X]\n", HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION);
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
    (void) hAdapter;

    PAGED_CODE();

    if(!pQueryAdapterInfo->pOutputData)
    {
        LOG_ERROR("Invalid Parameter to FillDriverCaps: pQueryAdapterInfo->pOutputData\n");
        return STATUS_INVALID_PARAMETER;
    }

    if(pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_DRIVERCAPS))
    {
        LOG_ERROR("Invalid Parameter to FillDriverCaps: pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_DRIVERCAPS) [%z]\n", sizeof(DXGK_DRIVERCAPS));
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

static NTSTATUS FillQuerySegment(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    (void) hAdapter;

    PAGED_CODE();

    // Validate that the input data is not null.
    if(!pQueryAdapterInfo->pInputData)
    {
        LOG_ERROR("Invalid Parameter to FillQuerySegment: pQueryAdapterInfo->pInputData\n");
        return STATUS_INVALID_PARAMETER;
    }

    // Validate that the output data is not null.
    if(!pQueryAdapterInfo->pOutputData)
    {
        LOG_ERROR("Invalid Parameter to FillQuerySegment: pQueryAdapterInfo->pOutputData\n");
        return STATUS_INVALID_PARAMETER;
    }

    if(pQueryAdapterInfo->InputDataSize != sizeof(DXGK_QUERYSEGMENTIN))
    {
        LOG_ERROR("Invalid Parameter to FillQuerySegment: pQueryAdapterInfo->InputDataSize != sizeof(DXGK_QUERYSEGMENTIN) [%z]\n", sizeof(DXGK_QUERYSEGMENTIN));
        return STATUS_INVALID_PARAMETER;
    }

    if(pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_QUERYSEGMENTOUT))
    {
        LOG_ERROR("Invalid Parameter to FillQuerySegment: pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_QUERYSEGMENTOUT) [%z]\n", sizeof(DXGK_QUERYSEGMENTOUT));
        return STATUS_INVALID_PARAMETER;
    }

    HyMiniportDeviceContext* const deviceContext = hAdapter;
    DXGK_QUERYSEGMENTOUT* const querySegment = pQueryAdapterInfo->pOutputData;

    if(querySegment->pSegmentDescriptor)
    {
        if(!deviceContext->Flags.IsEmulated)
        {
            // Need to query the adapter for its memory information.
        }
        else
        {
            if(querySegment->NbSegment > 1)
            {
                querySegment->NbSegment = 1;
            }

            querySegment->pSegmentDescriptor[0].BaseAddress.QuadPart = 0;
            querySegment->pSegmentDescriptor[0].CpuTranslatedAddress.QuadPart = 0;
            querySegment->pSegmentDescriptor[0].Size = 0;

            querySegment->pSegmentDescriptor[0].Flags.Value = 0;

            querySegment->pSegmentDescriptor[0].Flags.Aperture = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.Agp = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.CpuVisible = TRUE;
            querySegment->pSegmentDescriptor[0].Flags.CacheCoherent = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.PitchAlignment = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.PopulatedFromSystemMemory = TRUE;

            querySegment->pSegmentDescriptor[0].Flags.Use64KBPages = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.ReservedSysMem = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.SupportsCpuHostAperture = FALSE;
            querySegment->pSegmentDescriptor[0].Flags.SupportsCachedCpuHostAperture = FALSE;
        }
    }
    else
    {
        if(!deviceContext->Flags.IsEmulated)
        {
            // Need to query the adapter for its memory information.
            querySegment->NbSegment = 1;
        }
        else
        {
            querySegment->NbSegment = 1;
        }
    }

    // The paging buffer segment being 0 forces to use write-combined memory. This seems to be what we want when emulating as we presumably don't have an aperture.
    querySegment->PagingBufferSegmentId = 0;
    // We shouldn't need a paging buffer as all memory is already local to the system, hopefully this is a valid value.
    querySegment->PagingBufferSize = 0;
    querySegment->PagingBufferPrivateDataSize = 0;

    return STATUS_SUCCESS;
}
