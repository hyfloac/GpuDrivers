// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_add_device
#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>
#include <wdmguid.h>

#include "AddDevice.h"
#include "MemoryAllocator.h"
#include "Logging.h"

#pragma code_seg("PAGE")

static NTSTATUS GetAdapterInfo(HyMiniportDeviceContext* const miniportDeviceContext);

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext)
{
    PAGED_CODE();

    LOG_DEBUG("HyAddDevice\n");

    // If PhysicalDeviceObject is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!PhysicalDeviceObject)
    {
        LOG_ERROR("Invalid Parameter to HyAddDevice: PhysicalDeviceObject\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If MiniportDeviceContext is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyAddDevice: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // Allocate a new object to represent our device context.
    HyMiniportDeviceContext* const deviceContext = HY_ALLOC_ZERO(HyMiniportDeviceContext, NonPagedPoolNx, POOL_TAG_DEVICE_CONTEXT);

    // If allocation failed inform the kernel.
    if(!deviceContext)
    {
        LOG_ERROR("Failed to allocate MiniportDeviceContext\n");
        return STATUS_NO_MEMORY;
    }

    deviceContext->Flags.Value = 0;
    
    // Store the adapter handle.
    deviceContext->PhysicalDeviceObject = PhysicalDeviceObject;
    deviceContext->MonitorPowerState = PowerDeviceD0;
    deviceContext->AdapterPowerState = PowerDeviceD0;

    {
        const NTSTATUS getAdapterStatus = GetAdapterInfo(deviceContext);
        if(!NT_SUCCESS(getAdapterStatus))
        {
            return getAdapterStatus;
        }
    }

    // Return our device context to the driver.
    *MiniportDeviceContext = deviceContext;

    return STATUS_SUCCESS;
}

static NTSTATUS GetPCIInterface(PDEVICE_OBJECT physicalDeviceObject, PBUS_INTERFACE_STANDARD pciInterface);

static NTSTATUS GetAdapterInfo(HyMiniportDeviceContext* const miniportDeviceContext)
{
    PAGED_CODE();

    if(!miniportDeviceContext)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    LOG_DEBUG("GetAdapterInfo: PDO: 0x%p, Type: %d, Device Type: 0x%08X\n", miniportDeviceContext->PhysicalDeviceObject, miniportDeviceContext->PhysicalDeviceObject->Type, miniportDeviceContext->PhysicalDeviceObject->DeviceType);

    // This needs to be in non-paged memory. https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/obtaining-device-configuration-information-at-irql---dispatch-level
    PBUS_INTERFACE_STANDARD pciInterface = HY_ALLOC_ZERO(BUS_INTERFACE_STANDARD, NonPagedPoolNx, POOL_TAG_DEVICE_CONTEXT);

    if(!pciInterface)
    {
        LOG_ERROR("Failed to allocate BUS_INTERFACE_STANDARD\n");
        return STATUS_NO_MEMORY;
    }

    {
        // Get the PCI Interface.
        const NTSTATUS getPciInterfaceStatus = GetPCIInterface(miniportDeviceContext->PhysicalDeviceObject, pciInterface);

        // Propagate errors.
        if(!NT_SUCCESS(getPciInterfaceStatus))
        {
            // Redirect positional invalid parameter errors to a generic invalid parameter error.
            if(getPciInterfaceStatus == STATUS_INVALID_PARAMETER_1 || getPciInterfaceStatus == STATUS_INVALID_PARAMETER_2)
            {
                return STATUS_INVALID_PARAMETER;
            }

            return getPciInterfaceStatus;
        }
    }

    NTSTATUS retStatus = STATUS_SUCCESS;

    {
        // Attempt to get the PCI Bus Number.
        ULONG propertyLength;
        const NTSTATUS busStatus = IoGetDeviceProperty(miniportDeviceContext->PhysicalDeviceObject, DevicePropertyBusNumber, sizeof(ULONG), &miniportDeviceContext->PCIBusNumber, &propertyLength);

        if(!NT_SUCCESS(busStatus))
        {
            LOG_ERROR("Failed to retrieve PCI bus number. 0x%08X\n", busStatus);
            retStatus = busStatus;
        }
    }

    if(NT_SUCCESS(retStatus))
    {
        // Attempt to get the PCI Slot Address
        ULONG address;
        ULONG propertyLength;
        const NTSTATUS slotStatus = IoGetDeviceProperty(miniportDeviceContext->PhysicalDeviceObject, DevicePropertyAddress, sizeof(PCI_SLOT_NUMBER), &address, &propertyLength);

        if(!NT_SUCCESS(slotStatus))
        {
            LOG_ERROR("Failed to retrieve PCI address. 0x%08X\n", slotStatus);
            retStatus = slotStatus;
        }
        else
        {
            miniportDeviceContext->PCISlotNumber.u.bits.DeviceNumber = (address >> 16) & 0xFFFF;
            miniportDeviceContext->PCISlotNumber.u.bits.FunctionNumber = address & 0xFFFF;

            LOG_DEBUG("Adapter: Device: 0x%04X Function: 0x%04X\n", miniportDeviceContext->PCISlotNumber.u.bits.DeviceNumber, miniportDeviceContext->PCISlotNumber.u.bits.FunctionNumber);
        }
    }

    if(NT_SUCCESS(retStatus))
    {
        if(!pciInterface->GetBusData)
        {
            LOG_ERROR("Failed to read PCI device configuration space.\n");
            retStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            // Attempt to get the PCI configuration space.
            if(pciInterface->GetBusData(pciInterface->Context, PCI_WHICHSPACE_CONFIG, &miniportDeviceContext->PCIConfig, 0, sizeof(PCI_COMMON_CONFIG)) != sizeof(PCI_COMMON_CONFIG))
            {
                LOG_ERROR("Failed to read PCI device configuration space.\n");
                retStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if(pciInterface->InterfaceDereference)
    {
        pciInterface->InterfaceDereference(pciInterface->Context);
    }

    return retStatus;
}

static NTSTATUS GetPCIInterface(PDEVICE_OBJECT physicalDeviceObject, PBUS_INTERFACE_STANDARD pciInterface)
{
    PAGED_CODE();

    // If physicalDeviceObject is null inform the caller that the first parameter was invalid.
    // This should probably never happen.
    if(!physicalDeviceObject)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pciInterface is null inform the caller that the second parameter was invalid.
    // This should probably never happen.
    if(!pciInterface)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // In case PCI chooses to treat our synchronous request as an asynchronous one, we need an event to wait on.
    KEVENT pciEvent;
    KeInitializeEvent(&pciEvent, NotificationEvent, FALSE);

    // Get the highest object in the PDO stack.
    PDEVICE_OBJECT targetObject = IoGetAttachedDeviceReference(physicalDeviceObject);

    LOG_DEBUG("GetPCIInterface: Highest object on PDO Stack: 0x%p, Type: %d, Device Type: 0x%08X\n", targetObject, targetObject->Type, targetObject->DeviceType);

    // Initialize the PnP IRP request and status block.
    IO_STATUS_BLOCK ioStatusBlock;
    PIRP irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP, targetObject, NULL, 0, NULL, &pciEvent, &ioStatusBlock);

    // If we somehow failed to initialize the IRP, release the PDO parent object reference and return an error.
    if(!irp)
    {
        ObDereferenceObject(targetObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Required by verifier
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    // Get the stack location info for the request.
    PIO_STACK_LOCATION ioStackLocation = IoGetNextIrpStackLocation(irp);

    // Setup the call to Query the PCI Interface from the PnP Manager
    ioStackLocation->MajorFunction = IRP_MJ_PNP;
    ioStackLocation->MinorFunction = IRP_MN_QUERY_INTERFACE;
    ioStackLocation->Parameters.QueryInterface.InterfaceType = &GUID_BUS_INTERFACE_STANDARD;
    ioStackLocation->Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    ioStackLocation->Parameters.QueryInterface.Version = 1;
    ioStackLocation->Parameters.QueryInterface.Interface = (PINTERFACE) pciInterface;
    ioStackLocation->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    // Perform the actual query.
    NTSTATUS callDriverStatus = IoCallDriver(targetObject, irp);

    // If the request was treated as asynchronous by PCI, wait on the event.
    if(callDriverStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&pciEvent, Executive, KernelMode, FALSE, NULL);
        callDriverStatus = ioStatusBlock.Status;
    }

    // Release the PDO parent object reference.
    ObDereferenceObject(targetObject);

    // Check for and log errors.
    if(!NT_SUCCESS(callDriverStatus) || !NT_SUCCESS(ioStatusBlock.Status))
    {
        LOG_ERROR("Failed to query BUS_INTERFACE_STANDARD. 0x%08X 0x%08X\n", callDriverStatus, ioStatusBlock.Status);
        return STATUS_NOINTERFACE;
    }

    return STATUS_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
