// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_child_relations
#ifdef __cplusplus
extern "C" {
#endif

#include "QueryChildRelations.h"
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

    if(1 * sizeof(DXGK_CHILD_DESCRIPTOR) > ChildRelationsSize)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // If ChildRelations is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!ChildRelations)
    {
        LOG_ERROR("Invalid Parameter to HyQueryChildRelations: ChildRelations\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    (void) RtlZeroMemory(ChildRelations, 1 * sizeof(DXGK_CHILD_DESCRIPTOR));

    return STATUS_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
