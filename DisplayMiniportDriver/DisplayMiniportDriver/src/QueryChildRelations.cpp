// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_child_relations
#include "Common.h"
#include "HyDevice.hpp"
#include "QueryChildRelations.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryChildRelations(IN_CONST_PVOID MiniportDeviceContext, PDXGK_CHILD_DESCRIPTOR ChildRelations, ULONG ChildRelationsSize)
{
    (void) MiniportDeviceContext;
    (void) ChildRelations;
    (void) ChildRelationsSize;

    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyQueryChildRelations\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyQueryChildRelations: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    if(ChildRelationsSize == 0)
    {
        LOG_DEBUG("HyQueryChildRelations: ChildRelationsSize was 0.\n");
        return STATUS_SUCCESS;
    }

    // If ChildRelations is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!ChildRelations)
    {
        LOG_ERROR("Invalid Parameter to HyQueryChildRelations: ChildRelations\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

    NTSTATUS status = deviceContext->QueryChildRelations(ChildRelations, ChildRelationsSize);

    if(status == STATUS_INVALID_PARAMETER_1)
    {
        status = STATUS_INVALID_PARAMETER_2;
    }
    else if(status == STATUS_INVALID_PARAMETER_2)
    {
        status = STATUS_INVALID_PARAMETER_3;
    }

    return status;
}
