#include "QueryInterface.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryInterface(IN_CONST_PVOID MiniportDeviceContext, IN_PQUERY_INTERFACE QueryInterface)
{
    (void) MiniportDeviceContext;
    (void) QueryInterface;

    LOG_DEBUG("HyQueryInterface\n");

    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyQueryInterface: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    if(!QueryInterface)
    {
        LOG_ERROR("Invalid Parameter to HyQueryInterface: QueryInterface\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    LOG_DEBUG("HyQueryInterface: IID: {%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}, Size: %d, Version: %d\n",
        QueryInterface->InterfaceType->Data1,
        QueryInterface->InterfaceType->Data2,
        QueryInterface->InterfaceType->Data3,
        QueryInterface->InterfaceType->Data4[0],
        QueryInterface->InterfaceType->Data4[1],
        QueryInterface->InterfaceType->Data4[2],
        QueryInterface->InterfaceType->Data4[3],
        QueryInterface->InterfaceType->Data4[4],
        QueryInterface->InterfaceType->Data4[5],
        QueryInterface->InterfaceType->Data4[6],
        QueryInterface->InterfaceType->Data4[7],
        QueryInterface->Size,
        QueryInterface->Version
    );

    return STATUS_NOT_SUPPORTED;
}
