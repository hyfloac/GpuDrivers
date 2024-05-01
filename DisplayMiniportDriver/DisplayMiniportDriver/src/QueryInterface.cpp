// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_interface
// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "QueryInterface.hpp"
#include "Logging.h"

#include "I2CInterface.h"

#pragma code_seg("PAGE")

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID(GUID_FUCKY, 0xE922004D, 0xEB9C, 0x4DE1, 0x92, 0x24, 0xA9, 0xCE, 0xAA, 0x95, 0x9B, 0xCE);
#define FUCKY_VERSION (1)  // NOLINT(modernize-macro-to-enum)

typedef
_Function_class_DXGK_(DXGKDDI_OPM_GET_CERTIFICATE_SIZE)
_IRQL_requires_DXGK_(PASSIVE_LEVEL)
NTSTATUS
(*DX_FUCKY_FUNC)(
    _In_ void* MiniportDeviceContext);

typedef struct DX_FUCKY_INTERFACE
{
    USHORT Size;
    USHORT Version;
    void* Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

    DX_FUCKY_FUNC Func0;
    DX_FUCKY_FUNC Func1;
    DX_FUCKY_FUNC Func2;
} DX_FUCKY_INTERFACE;

static_assert(sizeof(DX_FUCKY_INTERFACE) == 56, "FUCKY Interface was not 56 bytes.");

static void FuckyReferenceThunk(void* const MiniportDeviceContext)
{
    (void) MiniportDeviceContext;
    LOG_WARN("FuckyReferenceThunk");
}

static void FuckyDereferenceThunk(void* const MiniportDeviceContext)
{
    (void) MiniportDeviceContext;
    LOG_WARN("FuckyDereferenceThunk");
}

static NTSTATUS FuckyThunk0(void* const MiniportDeviceContext)
{
    (void) MiniportDeviceContext;
    LOG_WARN("FuckyThunk0");
    return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS FuckyThunk1(void* const MiniportDeviceContext)
{
    (void) MiniportDeviceContext;
    LOG_WARN("FuckyThunk1");
    return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS FuckyThunk2(void* const MiniportDeviceContext)
{
    (void) MiniportDeviceContext;
    LOG_WARN("FuckyThunk2");
    return STATUS_NOT_IMPLEMENTED;
}

static BOOL GuidEqual(const GUID* const a, const GUID* const b)
{
    return RtlCompareMemory(a, b, sizeof(GUID)) == sizeof(GUID);
}

NTSTATUS HyQueryInterface(IN_CONST_PVOID MiniportDeviceContext, IN_PQUERY_INTERFACE QueryInterface)
{
    (void) MiniportDeviceContext;
    (void) QueryInterface;

    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyQueryInterface\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyQueryInterface: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If QueryInterface is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!QueryInterface)
    {
        LOG_ERROR("Invalid Parameter to HyQueryInterface: QueryInterface\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    LOG_DEBUG("HyQueryInterface: IID: {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}, Size: %d, Version: %d\n",
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

    if(GuidEqual(QueryInterface->InterfaceType, &GUID_DEVINTERFACE_OPM))
    {
        if constexpr(false)
        {
            if(QueryInterface->Size == sizeof(DXGK_OPM_INTERFACE) && QueryInterface->Version == DXGK_OPM_INTERFACE_VERSION_1)
            {
                DXGK_OPM_INTERFACE* opmInterface = (DXGK_OPM_INTERFACE*) QueryInterface->Interface;

                opmInterface->Size = sizeof(DXGK_OPM_INTERFACE);
                opmInterface->Version = DXGK_OPM_INTERFACE_VERSION_1;
                opmInterface->Context = MiniportDeviceContext;


                return STATUS_SUCCESS;
            }
        }
    }
#if DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0
    else if(GuidEqual(QueryInterface->InterfaceType, &GUID_DEVINTERFACE_OPM_2))
    {
        if constexpr(false)
        {
            if(QueryInterface->Size == sizeof(DXGK_OPM_INTERFACE_2) && QueryInterface->Version == DXGK_OPM_INTERFACE_VERSION_2)
            {
                DXGK_OPM_INTERFACE_2* opmInterface = (DXGK_OPM_INTERFACE_2*) QueryInterface->Interface;

                opmInterface->Size = sizeof(DXGK_OPM_INTERFACE_2);
                opmInterface->Version = DXGK_OPM_INTERFACE_VERSION_2;
                opmInterface->Context = MiniportDeviceContext;


                return STATUS_SUCCESS;
            }
        }
    }
    else if(GuidEqual(QueryInterface->InterfaceType, &GUID_DEVINTERFACE_OPM_2_JTP))
    {
        if constexpr(false)
        {
            if(QueryInterface->Size == sizeof(DXGK_OPM_INTERFACE_2_JTP) && QueryInterface->Version == DXGK_OPM_INTERFACE_VERSION_2_JTP)
            {
                DXGK_OPM_INTERFACE_2_JTP* opmInterface = (DXGK_OPM_INTERFACE_2_JTP*) QueryInterface->Interface;

                opmInterface->Size = sizeof(DXGK_OPM_INTERFACE_2_JTP);
                opmInterface->Version = DXGK_OPM_INTERFACE_VERSION_2_JTP;
                opmInterface->Context = MiniportDeviceContext;


                return STATUS_SUCCESS;
            }
        }
    }
#endif
#if DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_3
    else if(GuidEqual(QueryInterface->InterfaceType, &GUID_DEVINTERFACE_OPM_3))
    {
        if constexpr(false)
        {
            if(QueryInterface->Size == sizeof(DXGK_OPM_INTERFACE_3) && QueryInterface->Version == DXGK_OPM_INTERFACE_VERSION_3)
            {
                DXGK_OPM_INTERFACE_3* opmInterface = (DXGK_OPM_INTERFACE_3*) QueryInterface->Interface;

                opmInterface->Size = sizeof(DXGK_OPM_INTERFACE_3);
                opmInterface->Version = DXGK_OPM_INTERFACE_VERSION_3;
                opmInterface->Context = MiniportDeviceContext;


                return STATUS_SUCCESS;
            }
        }
    }
#endif
    else if(GuidEqual(QueryInterface->InterfaceType, &GUID_DEVINTERFACE_I2C))
    {
        if(QueryInterface->Size == sizeof(DXGK_I2C_INTERFACE) && QueryInterface->Version == DXGK_I2C_INTERFACE_VERSION_1)
        {
            DXGK_I2C_INTERFACE* pI2CInterface = reinterpret_cast<DXGK_I2C_INTERFACE*>(QueryInterface->Interface);

            pI2CInterface->Size = sizeof(DXGK_I2C_INTERFACE);
            pI2CInterface->Version = DXGK_I2C_INTERFACE_VERSION_1;
            pI2CInterface->Context = MiniportDeviceContext;
            pI2CInterface->InterfaceReference = HyI2CInterfaceReference;
            pI2CInterface->InterfaceDereference = HyI2CInterfaceDereference;
            pI2CInterface->DxgkDdiI2CTransmitDataToDisplay = HyI2CInterfaceTransmitDataToDisplay;
            pI2CInterface->DxgkDdiI2CReceiveDataFromDisplay = HyI2CInterfaceReceiveDataFromDisplay;

            return STATUS_SUCCESS;
        }
    }
    else if(GuidEqual(QueryInterface->InterfaceType, &GUID_FUCKY))
    {
        LOG_WARN("HyQueryInterface: Querying that special fucky Interface that may be required, but is undocumented, and only needs to return not-implemented...\n");

        if(QueryInterface->Size == sizeof(DX_FUCKY_INTERFACE) && QueryInterface->Version == FUCKY_VERSION)
        {
            DX_FUCKY_INTERFACE* pFuckyInterface = reinterpret_cast<DX_FUCKY_INTERFACE*>(QueryInterface->Interface);

            pFuckyInterface->Size = sizeof(DX_FUCKY_INTERFACE);
            pFuckyInterface->Version = FUCKY_VERSION;
            pFuckyInterface->Context = MiniportDeviceContext;
            pFuckyInterface->InterfaceReference = FuckyReferenceThunk;
            pFuckyInterface->InterfaceDereference = FuckyDereferenceThunk;
            pFuckyInterface->Func0 = FuckyThunk0;
            pFuckyInterface->Func1 = FuckyThunk1;
            pFuckyInterface->Func2 = FuckyThunk2;

            LOG_DEBUG("HyQueryInterface: Fucky interface version and sized matches.\n");

            return STATUS_SUCCESS;
        }
        else
        {
            LOG_ERROR("HyQueryInterface: That special fucky interface didn't have the right size or version.\n");
        }
    }

    return STATUS_NOT_SUPPORTED;
}
