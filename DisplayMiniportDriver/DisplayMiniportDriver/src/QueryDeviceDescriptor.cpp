// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_device_descriptor
#include "QueryDeviceDescriptor.hpp"

#include "HyDevice.hpp"
#include "Logging.h"
#include "RegisterMemCopy.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryDeviceDescriptor(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG ChildUid, INOUT_PDXGK_DEVICE_DESCRIPTOR DeviceDescriptor)
{
    (void) MiniportDeviceContext;
    (void) ChildUid;
    (void) DeviceDescriptor;

    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyQueryDeviceDescriptor\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyQueryDeviceDescriptor: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If DeviceDescriptor is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!DeviceDescriptor)
    {
        LOG_ERROR("Invalid Parameter to HyQueryDeviceDescriptor: DeviceDescriptor\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // We're only going to report our single display.
    if(ChildUid > 1)
    {
        return STATUS_GRAPHICS_CHILD_DESCRIPTOR_NOT_SUPPORTED;
    }

    if(DeviceDescriptor->DescriptorOffset >= 128)
    {
        return STATUS_MONITOR_NO_MORE_DESCRIPTOR_DATA;
    }

    // Get our context structure.
    const HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

    const ULONG edidOffset = DeviceDescriptor->DescriptorOffset;
    ULONG edidLength = 128;

    if(edidLength - edidOffset > DeviceDescriptor->DescriptorLength)
    {
        edidLength = DeviceDescriptor->DescriptorLength;
    }

    // const UINT8* bar0 = deviceContext->ConfigRegistersPointer;

    const volatile UINT* const edidDisplay0 = deviceContext->GetDeviceConfigRegister(HyMiniportDevice::BASE_REGISTER_EDID + HyMiniportDevice::SIZE_REGISTER_EDID * 0 + edidOffset);

    RegisterMemCopyNV32(DeviceDescriptor->DescriptorBuffer, edidDisplay0, static_cast<int>(edidLength));

    return STATUS_SUCCESS;
}
