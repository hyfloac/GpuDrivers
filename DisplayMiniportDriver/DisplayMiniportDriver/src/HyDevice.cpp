#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"
#include <wdmguid.h>
#include "RegisterMemCopy.h"
#include <ntstrsafe.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "HyDevice.hpp"
#include "Logging.h"
#include "MemoryAllocator.h"
#include "Config.h"
#include "GsLogicalDevice.hpp"

#pragma code_seg(push)
#pragma code_seg("PAGE")

// Display-Only Devices can only return display modes of D3DDDIFMT_A8R8G8B8.
// Color conversion takes place if the app's fullscreen backbuffer has different format.
// Full display drivers can add more if the hardware supports them.
static D3DDDIFORMAT gPixelFormats[] = {
    D3DDDIFMT_A8R8G8B8
};

static NTSTATUS MapFrameBuffer(PHYSICAL_ADDRESS PhysicalAddress, ULONG Length, void** VirtualAddress) noexcept;

void* HyMiniportDevice::operator new(const SIZE_T count)
{
    return HyAllocate(ExDefaultNonPagedPoolType, count, POOL_TAG_DEVICE_CONTEXT);
}

void HyMiniportDevice::operator delete(void* const ptr)
{
    HyDeallocate(ptr, POOL_TAG_DEVICE_CONTEXT);
}

HyMiniportDevice::HyMiniportDevice(PDEVICE_OBJECT PhysicalDeviceObject) noexcept
    : m_PhysicalDeviceObject(PhysicalDeviceObject)
    , m_DxgkStartInfo { }
    , m_DxgkInterface { }
    , m_DeviceInfo { }
    , m_DeviceId(0)
    , m_Flags { .Value = 0 }
    , m_PCIBusNumber(0)
    , m_PCISlotNumber { }
    , m_PCIConfig { }
    , m_ConfigRegistersPointer(nullptr)
    , m_CurrentDisplayMode { }
    , m_AdapterPowerState(PowerDeviceUnspecified)
    , m_CurrentLogLockValue(1)
    , m_PresentManager(this)
    , m_MemoryManager()
{ }

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

    LOG_DEBUG("GetPCIInterface: Highest object on PDO Stack: 0x%p, Type: %d, Device Type: %s [0x%08X]\n", targetObject, targetObject->Type, GetFileDeviceString(targetObject->DeviceType), targetObject->DeviceType);

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
    // Initialize the status to error in case the bus driver does not 
    // set it correctly.
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
    // if(!NT_SUCCESS(callDriverStatus) || !NT_SUCCESS(ioStatusBlock.Status))
    if(!NT_SUCCESS(callDriverStatus))
    {
        LOG_ERROR("Failed to query BUS_INTERFACE_STANDARD. 0x%08X 0x%08X\n", callDriverStatus, ioStatusBlock.Status);
        return STATUS_NOINTERFACE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::GetAdapterInfo() noexcept
{
    LOG_DEBUG("PDO: 0x%p, Type: %d, Device Type: %s [0x%08X]\n", m_PhysicalDeviceObject, m_PhysicalDeviceObject->Type, GetFileDeviceString(m_PhysicalDeviceObject->DeviceType), m_PhysicalDeviceObject->DeviceType);

    // This needs to be in non-paged memory. https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/obtaining-device-configuration-information-at-irql---dispatch-level
    PBUS_INTERFACE_STANDARD pciInterface = HY_ALLOC_ZERO(BUS_INTERFACE_STANDARD, NonPagedPoolNx, POOL_TAG_DEVICE_CONTEXT);

    if(!pciInterface)
    {
        LOG_ERROR("Failed to allocate BUS_INTERFACE_STANDARD\n");
        return STATUS_NO_MEMORY;
    }

    {
        // Get the PCI Interface.
        const NTSTATUS getPciInterfaceStatus = GetPCIInterface(m_PhysicalDeviceObject, pciInterface);

        // Propagate errors.
        if(!NT_SUCCESS(getPciInterfaceStatus))
        {
            // Free the pciInterface on failure.
            // 2024-04-26: This was causing a memory leak.
            HY_FREE(pciInterface, POOL_TAG_DEVICE_CONTEXT);

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
        const NTSTATUS busStatus = IoGetDeviceProperty(m_PhysicalDeviceObject, DevicePropertyBusNumber, sizeof(ULONG), &m_PCIBusNumber, &propertyLength);

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
        const NTSTATUS slotStatus = IoGetDeviceProperty(m_PhysicalDeviceObject, DevicePropertyAddress, sizeof(PCI_SLOT_NUMBER), &address, &propertyLength);

        if(!NT_SUCCESS(slotStatus))
        {
            LOG_ERROR("Failed to retrieve PCI address. 0x%08X\n", slotStatus);
            retStatus = slotStatus;
        }
        else
        {
            m_PCISlotNumber.u.bits.DeviceNumber = (address >> 16) & 0xFFFF;
            m_PCISlotNumber.u.bits.FunctionNumber = address & 0xFFFF;

            LOG_DEBUG("Adapter: Device: 0x%04X Function: 0x%04X\n", m_PCISlotNumber.u.bits.DeviceNumber, m_PCISlotNumber.u.bits.FunctionNumber);
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
            if(pciInterface->GetBusData(pciInterface->Context, PCI_WHICHSPACE_CONFIG, &m_PCIConfig, 0, sizeof(PCI_COMMON_CONFIG)) != sizeof(PCI_COMMON_CONFIG))
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

    // Free the pciInterface after we've de-referenced it.
    // 2024-04-26: This was causing a memory leak.
    HY_FREE(pciInterface, POOL_TAG_DEVICE_CONTEXT);

    return retStatus;
}

NTSTATUS HyMiniportDevice::StartDevice(IN_PDXGK_START_INFO DxgkStartInfo, IN_PDXGKRNL_INTERFACE DxgkInterface, OUT_PULONG NumberOfVideoPresentSurfaces, OUT_PULONG NumberOfChildren) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    // Copy and save the DXGK start info.
    m_DxgkStartInfo = *DxgkStartInfo;
    // Copy and save the DXGK interface functions.
    m_DxgkInterface = *DxgkInterface;

    // Get the basic data about our device.
    {
        const NTSTATUS getDeviceInfoStatus = m_DxgkInterface.DxgkCbGetDeviceInformation(m_DxgkInterface.DeviceHandle, &m_DeviceInfo);

        if(!NT_SUCCESS(getDeviceInfoStatus))
        {
            LOG_ERROR("Failed to get device info.\n");
            return getDeviceInfoStatus;
        }

        LOG_DEBUG("Registry Path: %S\n", m_DeviceInfo.DeviceRegistryPath.Buffer);
    }

    // Check that the device is in fact ours.
    {
        const NTSTATUS checkDeviceStatus = CheckDevice();

        if(!NT_SUCCESS(checkDeviceStatus))
        {
            LOG_ERROR("Failed to check device: 0x%08X.\n", checkDeviceStatus);
            return checkDeviceStatus;
        }
    }

    LOG_DEBUG("Check device passed.\n");

    // Acquire POST display data
    {
        const NTSTATUS loadPostInfoStatus = LoadPostDisplayInfo();

        if(!NT_SUCCESS(loadPostInfoStatus))
        {
            LOG_ERROR("Failed to acquire POST display: 0x%08X.\n", loadPostInfoStatus);
        }
    }

    bool gpuTypeFound = false;

    // Check if this is a PCI device or a ROOT device.
    {
        const NTSTATUS checkDevicePrefixStatus = CheckDevicePrefix(&gpuTypeFound);

        if(!NT_SUCCESS(checkDevicePrefixStatus))
        {
            LOG_ERROR("Failed to check device prefix: 0x%08X.\n", checkDevicePrefixStatus);
            return checkDevicePrefixStatus;
        }
    }

    m_MemoryManager.Init(m_DeviceId, m_DeviceInfo, m_DxgkInterface);

    if(m_DeviceId == 0x0001)
    {
        m_ConfigRegistersPointer = m_MemoryManager.MappedBarMap().Region0.VirtualPointer;
    }

    m_MemoryManager.InitSegments(GetDeviceVramSize());

    //   If we haven't found the GPU type yet (i.e. it's not a software
    // device, then we'll ask the GPU what type it is).
    if(!gpuTypeFound)
    {
        const UINT gpuType = *GetDeviceConfigRegister(REGISTER_EMULATION);

        switch(gpuType)
        {
            case VALUE_REGISTER_EMULATION_MICROPROCESSOR:
                m_Flags.GpuType = GPU_TYPE_MICROPROCESSOR;
                break;
            case VALUE_REGISTER_EMULATION_FPGA:
                m_Flags.GpuType = GPU_TYPE_FPGA;
                break;
            case VALUE_REGISTER_EMULATION_SIMULATION:
                m_Flags.GpuType = GPU_TYPE_SIMULATED;
                break;
            default:
                m_Flags.GpuType = GPU_TYPE_SOFTWARE;
                break;
        }
    }

    //   DxgkCbAcquirePostDisplayOwnership may return a DXGK_DISPLAY_INFORMATION structure with the Width 
    // member set to zero. This indicates that either the current display device is not capable of POST operations or the 
    // operating system does not have the current display information for the current POST device. 
    // 
    // Query the display info from the GPU BAR0.
    if(m_CurrentDisplayMode[0].DisplayInfo.Width == 0)
    {
        const UINT width = *GetDeviceConfigRegister(BASE_REGISTER_DI + SIZE_REGISTER_DI * 0 + OFFSET_REGISTER_DI_WIDTH);
        m_CurrentDisplayMode[0].DisplayInfo.Width = width;
        m_CurrentDisplayMode[0].DisplayInfo.Height = *GetDeviceConfigRegister(BASE_REGISTER_DI + SIZE_REGISTER_DI * 0 + OFFSET_REGISTER_DI_HEIGHT);
        m_CurrentDisplayMode[0].DisplayInfo.Pitch = width * 4;
        m_CurrentDisplayMode[0].DisplayInfo.ColorFormat = D3DDDIFMT_A8R8G8B8;
        if(m_DeviceId == 0x0001)
        {
            m_CurrentDisplayMode[0].DisplayInfo.PhysicAddress.QuadPart = static_cast<LONGLONG>(m_MemoryManager.MappedBarMap().Region1.Start);
        }
        m_CurrentDisplayMode[0].DisplayInfo.TargetId = 0;
        m_CurrentDisplayMode[0].DisplayInfo.AcpiId = 0;

        LOG_DEBUG("Display 0: %dx%d, Pitch: %d, 0x%I64X\n", m_CurrentDisplayMode[0].DisplayInfo.Width, m_CurrentDisplayMode[0].DisplayInfo.Height, m_CurrentDisplayMode[0].DisplayInfo.Pitch, m_CurrentDisplayMode[0].DisplayInfo.PhysicAddress.QuadPart);
    }

    // Initialize everything for the present queue.
    {
        const NTSTATUS initPresentManagerStatus = m_PresentManager.Init();

        if(!NT_SUCCESS(initPresentManagerStatus))
        {
            return initPresentManagerStatus;
        }
    }

    // Enable Display
    SetDisplayState(0, true);

    m_Flags.IsStarted = true;

    // We'll specify that we have one VidPN source.
    *NumberOfVideoPresentSurfaces = 1;
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/display/child-devices-of-the-display-adapter
    // We have one child, for the display output, but not the display itself.
    *NumberOfChildren = 1;

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::CheckDevice() noexcept
{
    PCI_COMMON_HEADER pciHeader { };
    ULONG bytesRead;
    // Get the device's PCI Vendor and Device ID's.
    const NTSTATUS readDeviceSpaceStatus = m_DxgkInterface.DxgkCbReadDeviceSpace(m_DxgkInterface.DeviceHandle, DXGK_WHICHSPACE_CONFIG, &pciHeader, 0, sizeof(pciHeader), &bytesRead);

    if(!NT_SUCCESS(readDeviceSpaceStatus))
    {
        return readDeviceSpaceStatus;
    }

    if(pciHeader.VendorID != HY_VENDOR_ID)
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    m_DeviceId = pciHeader.DeviceID;

    switch(pciHeader.DeviceID)
    {
        case 0x0001: return STATUS_SUCCESS;
        // If we don't recognize the device ID a different version of our driver is probably handling it.
        default: return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }
}

NTSTATUS HyMiniportDevice::LoadPostDisplayInfo() noexcept
{
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
    // Check if DxgkCbAcquirePostDisplayOwnership exists.
    if(!m_DxgkInterface.DxgkCbAcquirePostDisplayOwnership)
    {
        LOG_ERROR("DxgkCbAcquirePostDisplayOwnership was not provided.\n");
        return STATUS_SUCCESS;
    }
    else
    {
        const NTSTATUS acquirePostDisplayStatus = m_DxgkInterface.DxgkCbAcquirePostDisplayOwnership(m_DxgkInterface.DeviceHandle, &m_CurrentDisplayMode[0].DisplayInfo);

        LOG_DEBUG("Display 0: %dx%d, Pitch: %d, 0x%I64X\n", m_CurrentDisplayMode[0].DisplayInfo.Width, m_CurrentDisplayMode[0].DisplayInfo.Height, m_CurrentDisplayMode[0].DisplayInfo.Pitch, m_CurrentDisplayMode[0].DisplayInfo.PhysicAddress.QuadPart);

        // If we failed to acquire the POST display we're probably not running a POST device, or we're pre WDDM 1.2.
        if(!NT_SUCCESS(acquirePostDisplayStatus))
        {
            LOG_ERROR("DxgkCbAcquirePostDisplayOwnership returned status 0x%08X, Width: %d.\n", acquirePostDisplayStatus, m_CurrentDisplayMode[0].DisplayInfo.Width);
            return acquirePostDisplayStatus;
        }
    }
#endif

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::CheckDevicePrefix(bool* const gpuTypeFound) noexcept
{
    NTSTATUS getEnumeratorStatus;
    wchar_t* enumerator;
    wchar_t staticEnumeratorBuffer[16];

    // Keep iterating until success if the failure is caused by too small of a buffer.
    do
    {
        LOG_DEBUG("Attempting to get the DevicePropertyEnumeratorName.\n");
        // Get the length of the enumerator string.
        ULONG bufferLength;
        const NTSTATUS getEnumeratorLengthStatus = IoGetDeviceProperty(m_PhysicalDeviceObject, DevicePropertyEnumeratorName, 0, NULL, &bufferLength);

        // If we fail propagate errors, unless it is a positional argument error.
        if(!NT_SUCCESS(getEnumeratorLengthStatus) && getEnumeratorLengthStatus != STATUS_BUFFER_TOO_SMALL)
        {
            LOG_ERROR("Failed to get the length of DevicePropertyEnumeratorName: 0x%08X.\n", getEnumeratorLengthStatus);
            // This seems to be causing a BugCheck (BSOD).
            // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

            if(getEnumeratorLengthStatus == STATUS_INVALID_PARAMETER_2)
            {
                return STATUS_UNSUCCESSFUL;
            }

            return getEnumeratorLengthStatus;
        }

        LOG_DEBUG("Buffer length: %u\n", bufferLength);

        // If the length is sufficiently small we'll just use static allocation.
        if(bufferLength <= 16)
        {
            LOG_DEBUG("Using static buffer\n");
            enumerator = staticEnumeratorBuffer;
        }
        else
        {
            LOG_DEBUG("Allocating buffer.\n");
            // Allocate the buffer for the enumerator name.
            enumerator = static_cast<wchar_t*>(HyAllocate(NonPagedPoolNx, bufferLength * sizeof(wchar_t), POOL_TAG_DEVICE_CONTEXT));

            // If the allocation fails report that we're out of memory.
            if(!enumerator)
            {
                LOG_ERROR(" Failed to allocate PCI Device String Buffer.\n");
                // This seems to be causing a BugCheck (BSOD).
                // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

                return STATUS_NO_MEMORY;
            }
        }

        // Get the enumerator name.
        getEnumeratorStatus = IoGetDeviceProperty(m_PhysicalDeviceObject, DevicePropertyEnumeratorName, bufferLength, enumerator, &bufferLength);

        // If we're successful we can break out of the loop.
        if(NT_SUCCESS(getEnumeratorStatus))
        {
            break;
        }

        // If we failed for a reason other than the buffer being too small, propagate errors, unless it is a positional argument error.
        if(getEnumeratorStatus != STATUS_BUFFER_TOO_SMALL)
        {
            LOG_ERROR("Failed to get DevicePropertyEnumeratorName: 0x%08X.\n", getEnumeratorStatus);
            // This seems to be causing a BugCheck (BSOD).
            // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

            // Only free if it was dynamically allocated.
            if(enumerator != staticEnumeratorBuffer)
            {
                // Free the buffer containing the enumerator name.
                HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
            }

            if(getEnumeratorStatus == STATUS_INVALID_PARAMETER_2)
            {
                return STATUS_UNSUCCESSFUL;
            }

            return getEnumeratorStatus;
        }

        // Only free if it was dynamically allocated.
        if(enumerator != staticEnumeratorBuffer)
        {
            // Free the buffer containing the enumerator name.
            HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
        }
    } while(getEnumeratorStatus == STATUS_BUFFER_TOO_SMALL);

    // The enumerator name for PCI devices, as opposed to Root Enumerated Devices.
    constexpr wchar_t pciPrefix[] = L"PCI";

    // Check only the first 3 characters.
    if(wcsncmp(pciPrefix, enumerator, wcslen(pciPrefix)) == 0)
    {
        // If it starts with PCI then it is not a software device (which doesn't work).
        // It could still be Simulated in VirtualBox, an FPGA, or a full Microprocessor
        *gpuTypeFound = false;
    }
    else
    {
        m_Flags.GpuType = GPU_TYPE_SOFTWARE;
        *gpuTypeFound = true;
    }

    LOG_DEBUG("Device ID Prefix: %ls\n", enumerator);

    // Only free if it was dynamically allocated.
    if(enumerator != staticEnumeratorBuffer)
    {
        // Free the buffer containing the enumerator name.
        HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
    }

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::StopDevice() noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    if(m_Flags.IsStarted)
    {
        // Disable Display 0
        SetDisplayState(0, false);
    }

    m_Flags.IsStarted = false;

    (void) m_PresentManager.Close();

    (void) m_MemoryManager.Close();

    return STATUS_SUCCESS;
}

#pragma code_seg(push)
#pragma code_seg("_KTEXT")
BOOLEAN HyMiniportDevice::InterruptRoutine(IN_ULONG MessageNumber) noexcept
{
    (void) MessageNumber;

    CHECK_IRQL(HIGH_LEVEL); // HIGH_LEVEL is the best approximation of DIRQL

    volatile UINT* const pMessageType = GetDeviceConfigRegister(REGISTER_INTERRUPT_TYPE);

    const UINT messageType = *pMessageType;
    *pMessageType = 0;

    if(messageType >= MSG_INTERRUPT_VSYNC_DISPLAY_0 && messageType < MSG_INTERRUPT_VSYNC_DISPLAY_0 + 8)
    {
        if(m_CurrentDisplayMode[0].Flags.VSyncEnabled == 0)
        {
            return TRUE;
        }

        DXGKARGCB_NOTIFY_INTERRUPT_DATA interruptData {};
        interruptData.Flags.Value = 0;

        if(m_CurrentDisplayMode[0].Flags.VSyncEnabled == 1)
        {
            interruptData.InterruptType = DXGK_INTERRUPT_CRTC_VSYNC;
            interruptData.CrtcVsync.VidPnTargetId = m_CurrentDisplayMode[0].VidPnTargetId;
            interruptData.CrtcVsync.PhysicalAddress.QuadPart = static_cast<LONGLONG>(reinterpret_cast<UINT64>(m_CurrentDisplayMode[0].FrameBufferPointer));
            interruptData.CrtcVsync.PhysicalAdapterMask = 0;
        }
        else if(m_CurrentDisplayMode[0].Flags.VSyncEnabled == 2)
        {
            if constexpr(false)
            {
                LOG_DEBUG("Handling Display-Only VSync, Target ID: %d\n", m_CurrentDisplayMode[0].VidPnTargetId);
            }

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
            interruptData.InterruptType = DXGK_INTERRUPT_DISPLAYONLY_VSYNC;
            interruptData.DisplayOnlyVsync.VidPnTargetId = m_CurrentDisplayMode[0].VidPnTargetId;
#else
            LOG_WARN("Driver needs to be compiled against DXGKDDI_INTERFACE_VERSION_WIN8 to use VSync Interrupts.\n");
#endif
        }

        m_DxgkInterface.DxgkCbNotifyInterrupt(m_DxgkInterface.DeviceHandle, &interruptData);
    }

    // This is required to be called. I don't fully get why, but it's explained here:
    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkcb_notify_interrupt
    m_DxgkInterface.DxgkCbQueueDpc(m_DxgkInterface.DeviceHandle);

    return TRUE;
}

void HyMiniportDevice::DpcRoutine() noexcept
{
    CHECK_IRQL(DISPATCH_LEVEL);

    m_DxgkInterface.DxgkCbNotifyDpc(m_DxgkInterface.DeviceHandle);
}
#pragma code_seg(pop)

NTSTATUS HyMiniportDevice::QueryChildRelations(PDXGK_CHILD_DESCRIPTOR ChildRelations, ULONG ChildRelationsSize) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    if(2 * sizeof(DXGK_CHILD_DESCRIPTOR) > ChildRelationsSize)
    {
        LOG_ERROR("Invalid Parameter to HyQueryChildRelations: ChildRelationsSize is not large enough for 1 child and 1 empty block.\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    ChildRelations[0].ChildDeviceType = TypeVideoOutput;
    ChildRelations[0].ChildCapabilities.Type.VideoOutput.InterfaceTechnology = D3DKMDT_VOT_OTHER;
    ChildRelations[0].ChildCapabilities.Type.VideoOutput.MonitorOrientationAwareness = D3DKMDT_MOA_NONE;
    ChildRelations[0].ChildCapabilities.Type.VideoOutput.SupportsSdtvModes = FALSE;
    ChildRelations[0].ChildCapabilities.HpdAwareness = HpdAwarenessAlwaysConnected;
    ChildRelations[0].AcpiUid = 0;
    ChildRelations[0].ChildUid = 0;

    (void) RtlZeroMemory(ChildRelations + 1, 1 * sizeof(DXGK_CHILD_DESCRIPTOR));

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::QueryChildStatus(INOUT_PDXGK_CHILD_STATUS ChildStatus, IN_BOOLEAN NonDestructiveOnly) noexcept
{
    (void) NonDestructiveOnly;

    CHECK_IRQL(PASSIVE_LEVEL);

    switch(ChildStatus->Type)
    {
        case StatusConnection:
            // HpdAwarenessInterruptible was reported since HpdAwarenessNone is deprecated.
            // However, wae have no knowledge of HotPlug events, so just always return connected.
            ChildStatus->HotPlug.Connected = TRUE; //static_cast<BOOLEAN>(m_Flags.IsStarted);
            LOG_DEBUG("Device Connected: %d\n", ChildStatus->HotPlug.Connected);
            return STATUS_SUCCESS;
        case StatusRotation:
            // D3DKMDT_MOA_NONE was reported, so this should never be called
            LOG_ERROR("Child status being queryied for StatusRotation even though D3DKMDT_MOA_NONE was reported.");
            return STATUS_INVALID_PARAMETER;
        default:
            LOG_ERROR("Unknown pChildStatus->Type %d requested.", ChildStatus->Type);
            return STATUS_NOT_SUPPORTED;
    }
}

NTSTATUS HyMiniportDevice::QueryDeviceDescriptor(IN_ULONG ChildUid, INOUT_PDXGK_DEVICE_DESCRIPTOR DeviceDescriptor) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG(
        "Child: %u, Offset: 0x%X (%u), Length: 0x%X (%u)\n", 
        ChildUid, 
        DeviceDescriptor->DescriptorOffset,
        DeviceDescriptor->DescriptorOffset, 
        DeviceDescriptor->DescriptorLength, 
        DeviceDescriptor->DescriptorLength
    );

#if 0
    return STATUS_MONITOR_NO_DESCRIPTOR;
#else
    // We're only going to report our single display.
    if(ChildUid > 1)
    {
        return STATUS_GRAPHICS_CHILD_DESCRIPTOR_NOT_SUPPORTED;
    }

    if(DeviceDescriptor->DescriptorOffset >= 128)
    {
        return STATUS_MONITOR_NO_MORE_DESCRIPTOR_DATA;
    }

    const ULONG edidOffset = DeviceDescriptor->DescriptorOffset;
    ULONG edidLength = DeviceDescriptor->DescriptorLength;

    if(edidLength > SIZE_REGISTER_EDID)
    {
        edidLength = SIZE_REGISTER_EDID;
    }

    if(edidOffset + edidLength >= SIZE_REGISTER_EDID)
    {
        edidLength = SIZE_REGISTER_EDID - edidOffset;
    }

    const volatile UINT* const edidDisplay0 = GetDeviceConfigRegister(BASE_REGISTER_EDID + SIZE_REGISTER_EDID * ChildUid + edidOffset);

    RegisterMemCopyNV32(DeviceDescriptor->DescriptorBuffer, edidDisplay0, static_cast<int>(edidLength));

    if(DeviceDescriptor->DescriptorOffset + DeviceDescriptor->DescriptorLength >= 128)
    {
        return STATUS_MONITOR_NO_MORE_DESCRIPTOR_DATA;
    }

    return STATUS_SUCCESS;
#endif
}

NTSTATUS HyMiniportDevice::SetPowerState(IN_ULONG DeviceUid, IN_DEVICE_POWER_STATE DevicePowerState, IN_POWER_ACTION ActionType) noexcept
{
    (void) ActionType;

    CHECK_IRQL(PASSIVE_LEVEL);

    if(DeviceUid == DISPLAY_ADAPTER_HW_ID)
    {
        if(DevicePowerState == PowerDeviceD0)
        {
            if(m_AdapterPowerState == PowerDeviceD3)
            {
                DXGKARG_SETVIDPNSOURCEVISIBILITY visibility;
                visibility.VidPnSourceId = D3DDDI_ID_ALL;
                visibility.Visible = FALSE;
                SetVidPnSourceVisibility(&visibility);
            }
        }

        m_AdapterPowerState = DevicePowerState;
    }
    else // Some child device, currently this does nothing.
    {

    }

    return STATUS_SUCCESS;
}

#pragma code_seg(push)
#pragma code_seg("_KTEXT")
void HyMiniportDevice::ResetDevice() noexcept
{
    const volatile UINT* const resetReg = GetDeviceConfigRegister(REGISTER_RESET);

    // We don't actually care about the value, reading the register is enough to reset the device.
    const UINT resetValue = *resetReg;

    (void) resetValue;
}
#pragma code_seg(pop)

NTSTATUS HyMiniportDevice::QueryAdapterInfo(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    switch(pQueryAdapterInfo->Type)
    {
        case DXGKQAITYPE_UMDRIVERPRIVATE: return FillUmDriverPrivate(pQueryAdapterInfo);
        case DXGKQAITYPE_DRIVERCAPS: return FillDriverCaps(pQueryAdapterInfo);
        case DXGKQAITYPE_QUERYSEGMENT: return FillQuerySegment(pQueryAdapterInfo);
        default: return STATUS_NOT_IMPLEMENTED;
    }
}

NTSTATUS HyMiniportDevice::CreateDevice(INOUT_PDXGKARG_CREATEDEVICE pCreateDevice) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);
    LOG_DEBUG("System Device: %d, GDI Device: %d, pInfo: %p\n", pCreateDevice->Flags.SystemDevice, pCreateDevice->Flags.GdiDevice, pCreateDevice->pInfo);

    GsLogicalDevice* logicalDevice = new GsLogicalDevice(
        pCreateDevice->hDevice,            // dxgkHandle
        pCreateDevice->Flags.SystemDevice, // isSystemDevice
        pCreateDevice->Flags.GdiDevice     // isGdiDevice
    );

    if(!logicalDevice)
    {
        LOG_WARN("Failed to allocate Logical Device.\n");
        return STATUS_NO_MEMORY;
    }

    pCreateDevice->hDevice = logicalDevice;
    pCreateDevice->pInfo = &logicalDevice->DeviceInfo();

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::FillUmDriverPrivate(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) const noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("Querying Driver Private Data\n");

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
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: pQueryAdapterInfo->InputDataSize [%zu] != sizeof(HyPrivateDriverData) [%zu]\n", pQueryAdapterInfo->InputDataSize, sizeof(HyPrivateDriverData));
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the output data size matches our internal data type.
    if(pQueryAdapterInfo->OutputDataSize != sizeof(HyPrivateDriverData))
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: pQueryAdapterInfo->OutputDataSize [%zu] != sizeof(HyPrivateDriverData) [%zu]\n", sizeof(HyPrivateDriverData));
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    const HyPrivateDriverData* const inputDriverData = static_cast<const HyPrivateDriverData*>(pQueryAdapterInfo->pInputData);
    HyPrivateDriverData* const outputDriverData = static_cast<HyPrivateDriverData*>(pQueryAdapterInfo->pOutputData);

    // Validate that the input data Magic value matches our Magic value.
    if(inputDriverData->Magic != HyPrivateDriverData::HY_PRIVATE_DRIVER_DATA_MAGIC)
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: inputDriverData->Magic != HY_PRIVATE_DRIVER_DATA_MAGIC [0x%08X]\n", HyPrivateDriverData::HY_PRIVATE_DRIVER_DATA_MAGIC);
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the internally stored size matches our internal data type.
    if(inputDriverData->Size != sizeof(HyPrivateDriverData))
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: inputDriverData->Size != sizeof(HyPrivateDriverData) [%zu]\n", pQueryAdapterInfo->OutputDataSize, sizeof(HyPrivateDriverData));
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Validate that the Miniport Display Driver and the User Mode Driver are using the same version of the private data.
    if(inputDriverData->Version != HyPrivateDriverData::HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION)
    {
        LOG_ERROR("Invalid Parameter to FillUmDriverPrivate: inputDriverData->Version != HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION [0x%08X]\n", HyPrivateDriverData::HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION);
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    // Set the same basic data for output
    outputDriverData->Magic = HyPrivateDriverData::HY_PRIVATE_DRIVER_DATA_MAGIC;
    outputDriverData->Size = sizeof(HyPrivateDriverData);
    outputDriverData->Version = HyPrivateDriverData::HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION;

    // Set the page size to 64KiB.
    outputDriverData->PageSize = 65536;

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::FillDriverCaps(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) const noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("Querying Driver Capabilities\n");

    if(!pQueryAdapterInfo->pOutputData)
    {
        LOG_ERROR("Invalid Parameter to FillDriverCaps: pQueryAdapterInfo->pOutputData\n");
        return STATUS_INVALID_PARAMETER;
    }

    if(pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_DRIVERCAPS))
    {
        LOG_ERROR("Invalid Parameter to FillDriverCaps: pQueryAdapterInfo->OutputDataSize [%zu] != sizeof(DXGK_DRIVERCAPS) [%zu]\n", pQueryAdapterInfo->OutputDataSize, sizeof(DXGK_DRIVERCAPS));
        return STATUS_INVALID_PARAMETER;
    }

    // const HyMiniportDeviceContext* const deviceContext = hAdapter;

    DXGK_DRIVERCAPS* const driverCaps = static_cast<DXGK_DRIVERCAPS*>(pQueryAdapterInfo->pOutputData);
    RtlZeroMemory(driverCaps, sizeof(*driverCaps));

    // driverCaps->HighestAcceptableAddress.QuadPart = (LONGLONG) (((UINT_PTR) deviceContext->VRamPointer) + HyGetDeviceVramSize(deviceContext));
    driverCaps->HighestAcceptableAddress.QuadPart = (1ull << 48) - 1;
    driverCaps->MaxAllocationListSlotId = 64;
    driverCaps->ApertureSegmentCommitLimit = GsMemoryManager::EnableApertureSegment ? m_MemoryManager.EmbeddedSegments()[0].CommitLimit : 0;
    driverCaps->MaxPointerWidth = 256;
    driverCaps->MaxPointerHeight = 256;

    driverCaps->PointerCaps.Value = 0;
    driverCaps->PointerCaps.Monochrome = TRUE;
    driverCaps->PointerCaps.Color = TRUE;
    driverCaps->PointerCaps.MaskedColor = FALSE;
    driverCaps->PointerCaps.Reserved = 0;

    driverCaps->InterruptMessageNumber = 0;
    driverCaps->NumberOfSwizzlingRanges = 128;
    driverCaps->MaxOverlays = 1;
    driverCaps->GammaRampCaps.Gamma_Rgb256x3x16 = 1;

    driverCaps->PresentationCaps.Value = 0;
    driverCaps->PresentationCaps.NoScreenToScreenBlt = 0;
    driverCaps->PresentationCaps.NoOverlapScreenBlt = 0;
    driverCaps->PresentationCaps.SupportKernelModeCommandBuffer = 0;
    driverCaps->PresentationCaps.NoSameBitmapAlphaBlend = 0;
    driverCaps->PresentationCaps.NoSameBitmapStretchBlt = 0;
    driverCaps->PresentationCaps.NoSameBitmapTransparentBlt = 0;
    driverCaps->PresentationCaps.NoSameBitmapOverlappedAlphaBlend = 0;
    driverCaps->PresentationCaps.NoSameBitmapOverlappedStretchBlt = 0;
    driverCaps->PresentationCaps.DriverSupportsCddDwmInterop = 0;
    driverCaps->PresentationCaps.Reserved0 = 0;
    driverCaps->PresentationCaps.AlignmentShift = 2;
    driverCaps->PresentationCaps.MaxTextureWidthShift = 2;
    driverCaps->PresentationCaps.MaxTextureHeightShift = 2;
    driverCaps->PresentationCaps.SupportAllBltRops = 0;
    driverCaps->PresentationCaps.SupportMirrorStretchBlt = 0;
    driverCaps->PresentationCaps.SupportMonoStretchBltModes = 0;
    driverCaps->PresentationCaps.StagingRectStartPitchAligned = 0;
    driverCaps->PresentationCaps.NoSameBitmapBitBlt = 0;
    driverCaps->PresentationCaps.NoSameBitmapOverlappedBitBlt = 1;
    driverCaps->PresentationCaps.Reserved1 = 0;
    driverCaps->PresentationCaps.NoTempSurfaceForClearTypeBlend = 0;
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
    driverCaps->PresentationCaps.SupportSoftwareDeviceBitmaps = 0;
    driverCaps->PresentationCaps.NoCacheCoherentApertureMemory = 0;
    driverCaps->PresentationCaps.SupportLinearHeap = 1;
    driverCaps->PresentationCaps.Reserved = 0;
#else
    driverCaps->PresentationCaps.Reserved = 0;
#endif

    driverCaps->MaxQueuedFlipOnVSync = 1;

    driverCaps->FlipCaps.Value = 0;
    driverCaps->FlipCaps.FlipOnVSyncWithNoWait = 1;
    driverCaps->FlipCaps.FlipOnVSyncMmIo = 1;
    driverCaps->FlipCaps.FlipInterval = 0;
    driverCaps->FlipCaps.FlipImmediateMmIo = 1;
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM1_3)
    driverCaps->FlipCaps.FlipIndependent = 1;
  #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
    driverCaps->FlipCaps.DdiPresentForIFlip = 1;
    driverCaps->FlipCaps.FlipImmediateOnHSync = 0;
    driverCaps->FlipCaps.Reserved = 0;
  #else
    driverCaps->FlipCaps.Reserved = 0;
  #endif
#else
    driverCaps->FlipCaps.Reserved = 0;
#endif

    driverCaps->SchedulingCaps.Value = 0;
    driverCaps->SchedulingCaps.MultiEngineAware = 0;
    driverCaps->SchedulingCaps.VSyncPowerSaveAware = 1;

    driverCaps->MemoryManagementCaps.Value = 0;
    driverCaps->MemoryManagementCaps.OutOfOrderLock = 1;
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN7)
    driverCaps->MemoryManagementCaps.DedicatedPagingEngine = 0;
    driverCaps->MemoryManagementCaps.PagingEngineCanSwizzle = 0;
    driverCaps->MemoryManagementCaps.SectionBackedPrimary = 0;
  #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM1_3)
    driverCaps->MemoryManagementCaps.CrossAdapterResource = 0;
    #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
    driverCaps->MemoryManagementCaps.VirtualAddressingSupported = 1;
    driverCaps->MemoryManagementCaps.GpuMmuSupported = 1;
    driverCaps->MemoryManagementCaps.IoMmuSupported = 0;
    driverCaps->MemoryManagementCaps.ReplicateGdiContent = 0;
    driverCaps->MemoryManagementCaps.NonCpuVisiblePrimary = 0;
    #else
    driverCaps->MemoryManagementCaps.Reserved = 0;
    #endif
  #else
    driverCaps->MemoryManagementCaps.Reserved = 0;
  #endif
#else
    driverCaps->MemoryManagementCaps.Reserved = 0;
#endif

    driverCaps->GpuEngineTopology.NbAsymetricProcessingNodes = 0;

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN7)
    driverCaps->WDDMVersion = DXGKDDI_WDDMv1_2;
#endif

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
    driverCaps->SupportNonVGA = TRUE;
    driverCaps->SupportSmoothRotation = TRUE;
#endif

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::FillQuerySegment(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) const noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("Querying Segments\n");

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
        LOG_ERROR("Invalid Parameter to FillQuerySegment: pQueryAdapterInfo->InputDataSize [%zu] != sizeof(DXGK_QUERYSEGMENTIN) [%zu]\n", pQueryAdapterInfo->InputDataSize, sizeof(DXGK_QUERYSEGMENTIN));
        return STATUS_INVALID_PARAMETER;
    }

    if(pQueryAdapterInfo->OutputDataSize != sizeof(DXGK_QUERYSEGMENTOUT))
    {
        LOG_ERROR("Invalid Parameter to FillQuerySegment: pQueryAdapterInfo->OutputDataSize [%zu] != sizeof(DXGK_QUERYSEGMENTOUT) [%zu]\n", pQueryAdapterInfo->OutputDataSize, sizeof(DXGK_QUERYSEGMENTOUT));
        return STATUS_INVALID_PARAMETER;
    }

    DXGK_QUERYSEGMENTOUT* const querySegment = static_cast<DXGK_QUERYSEGMENTOUT*>(pQueryAdapterInfo->pOutputData);

    querySegment->PagingBufferPrivateDataSize = 0;
    return m_MemoryManager.FillSegments(*querySegment);

//     const UINT64 vramSize = GetDeviceVramSize();
//     const UINT64 cpuInvisibleVramSize = vramSize - m_MemoryManager.MappedBarMap().Region1.Length;
//
//     UINT requiredSegments = cpuInvisibleVramSize != 0 ? 2 : 1;
//
//     if constexpr(EnableApertureSegment)
//     {
//         ++requiredSegments;
//     }
//
//     if(querySegment->pSegmentDescriptor)
//     {
//         if(m_Flags.GpuType == GPU_TYPE_SOFTWARE)
//         {
//             if(querySegment->NbSegment > 1)
//             {
//                 querySegment->NbSegment = 1;
//             }
//
//             querySegment->pSegmentDescriptor[0].BaseAddress.QuadPart = 0;
//             querySegment->pSegmentDescriptor[0].CpuTranslatedAddress.QuadPart = 0;
//             querySegment->pSegmentDescriptor[0].Size = 0;
//
//             querySegment->pSegmentDescriptor[0].Flags.Value = 0;
//
//             querySegment->pSegmentDescriptor[0].Flags.Aperture = FALSE;
//             querySegment->pSegmentDescriptor[0].Flags.Agp = FALSE;
//             querySegment->pSegmentDescriptor[0].Flags.CpuVisible = TRUE;
//             querySegment->pSegmentDescriptor[0].Flags.CacheCoherent = FALSE;
//             querySegment->pSegmentDescriptor[0].Flags.PitchAlignment = FALSE;
//             querySegment->pSegmentDescriptor[0].Flags.PopulatedFromSystemMemory = TRUE;
//
//             // querySegment->pSegmentDescriptor[0].m_Flags.Use64KBPages = FALSE;
//             // querySegment->pSegmentDescriptor[0].m_Flags.ReservedSysMem = FALSE;
//             // querySegment->pSegmentDescriptor[0].m_Flags.SupportsCpuHostAperture = FALSE;
//             // querySegment->pSegmentDescriptor[0].m_Flags.SupportsCachedCpuHostAperture = FALSE;
//         }
//         else
//         {
//             // Need to query the adapter for its memory information.
//
//             // If more than 2 segments, report that we only filled out 2.
//             if(querySegment->NbSegment > requiredSegments)
//             {
//                 querySegment->NbSegment = requiredSegments;
//             }
//
//             int index = 0;
//
//             if constexpr(EnableApertureSegment)
//             {
//                 DXGK_SEGMENTDESCRIPTOR& descriptor = querySegment->pSegmentDescriptor[index++];
//
//                 constexpr SIZE_T size = static_cast<SIZE_T>(32) * 1024u * 1024u;
//
//                 // Aperature Segment
//                 descriptor.BaseAddress.QuadPart = 0;
//                 descriptor.CpuTranslatedAddress.QuadPart = 0;
//                 descriptor.Size = size;
//                 descriptor.NbOfBanks = 0;
//                 descriptor.pBankRangeTable = nullptr;
//                 descriptor.CommitLimit = (size / 16) * 4096;
//
//                 descriptor.Flags.Value = 0;
//                 descriptor.Flags.Aperture = 1;
//                 descriptor.Flags.Agp = 0;
//                 descriptor.Flags.CpuVisible = 0;
//                 descriptor.Flags.UseBanking = 0;
//                 descriptor.Flags.CacheCoherent = 0;
//                 descriptor.Flags.PitchAlignment = 0;
//                 descriptor.Flags.PopulatedFromSystemMemory = 0;
// #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
//                 descriptor.Flags.PreservedDuringStandby = 1;
//                 descriptor.Flags.PreservedDuringHibernate = 1;
//                 descriptor.Flags.DirectFlip = 0;
// #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
//                 querySegment->pSegmentDescriptor[0].Flags.Use64KBPages = 1;
// #endif
// #endif
//             }
//
//             {
//                 DXGK_SEGMENTDESCRIPTOR& descriptor = querySegment->pSegmentDescriptor[index++];
//
//                 // CPU Visible Segment
//                 descriptor.BaseAddress.QuadPart = 0;
//                 descriptor.CpuTranslatedAddress.QuadPart = 0;
//                 descriptor.Size = m_MemoryManager.MappedBarMap().Region1.Length;
//                 descriptor.NbOfBanks = 0;
//                 descriptor.pBankRangeTable = nullptr;
//                 descriptor.CommitLimit = 0;
//
//                 descriptor.Flags.Value = 0;
//                 descriptor.Flags.Aperture = 0;
//                 descriptor.Flags.Agp = 0;
//                 descriptor.Flags.CpuVisible = 1;
//                 descriptor.Flags.UseBanking = 0;
//                 descriptor.Flags.CacheCoherent = 0;
//                 descriptor.Flags.PitchAlignment = 0;
//                 descriptor.Flags.PopulatedFromSystemMemory = 0;
// #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
//                 descriptor.Flags.PreservedDuringStandby = 1;
//                 descriptor.Flags.PreservedDuringHibernate = 1;
//                 descriptor.Flags.DirectFlip = 1;
// #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
//                 querySegment->pSegmentDescriptor[0].Flags.Use64KBPages = 1;
// #endif
// #endif
//             }
//
//             if(cpuInvisibleVramSize != 0)
//             {
//                 DXGK_SEGMENTDESCRIPTOR& descriptor = querySegment->pSegmentDescriptor[index++];
//
//                 // CPU Invisible Segment
//                 descriptor.BaseAddress.QuadPart = 0;
//                 descriptor.CpuTranslatedAddress.QuadPart = 0;
//                 descriptor.Size = cpuInvisibleVramSize;
//                 descriptor.NbOfBanks = 0;
//                 descriptor.pBankRangeTable = nullptr;
//                 descriptor.CommitLimit = 0;
//
//                 descriptor.Flags.Value = 0;
//                 descriptor.Flags.Aperture = 0;
//                 descriptor.Flags.Agp = 0;
//                 descriptor.Flags.CpuVisible = 1;
//                 descriptor.Flags.UseBanking = 0;
//                 descriptor.Flags.CacheCoherent = 0;
//                 descriptor.Flags.PitchAlignment = 0;
//                 descriptor.Flags.PopulatedFromSystemMemory = 0;
// #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
//                 descriptor.Flags.PreservedDuringStandby = 1;
//                 descriptor.Flags.PreservedDuringHibernate = 1;
//                 descriptor.Flags.DirectFlip = 1;
// #if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
//                 querySegment->pSegmentDescriptor[0].Flags.Use64KBPages = 1;
// #endif
// #endif
//             }
//         }
//     }
//     else
//     {
//         if(m_Flags.GpuType == GPU_TYPE_SOFTWARE)
//         {
//             querySegment->NbSegment = 1;
//         }
//         else
//         {
//             querySegment->NbSegment = requiredSegments;
//         }
//     }
//
//     // The paging buffer segment being 0 forces to use write-combined memory. Not sure what to put here yet.
//     querySegment->PagingBufferSegmentId = 0;
//     // Not sure what to put here yet
//     querySegment->PagingBufferSize = 65536; 
}

NTSTATUS HyMiniportDevice::CollectDbgInfo(IN_CONST_PDXGKARG_COLLECTDBGINFO pCollectDbgInfo) noexcept
{
    // This function should collect debug information for various failures and can be
    // called at any time and at high IRQL (that is, the IRQL that DxgkDdiCollectDbgInfo
    // runs at is generally undefined).
    // https://learn.microsoft.com/en-us/windows-hardware/drivers/display/threading-and-synchronization-zero-level
    CHECK_IRQL(HIGH_LEVEL);

    if(pCollectDbgInfo->Reason == VIDEO_TDR_TIMEOUT_DETECTED || pCollectDbgInfo->Reason == VIDEO_ENGINE_TIMEOUT_DETECTED)
    {
        //  However, if the Reason member of the DXGKARG_COLLECTDBGINFO structure that
        //  the pCollectDbgInfo parameter points to is set to VIDEO_TDR_TIMEOUT_DETECTED
        //  or VIDEO_ENGINE_TIMEOUT_DETECTED, the driver must ensure that
        //  DxgkDdiCollectDbgInfo is pageable, runs at IRQL = PASSIVE_LEVEL, and supports
        //  synchronization zero level.
        // https://learn.microsoft.com/en-us/windows-hardware/drivers/display/threading-and-synchronization-zero-level
        CHECK_IRQL(PASSIVE_LEVEL);
    }

    LOG_WARN("Reason: 0x%X, Buffer: %p, Buffer Size: %zu (0x%zX), Extension: %p\n", pCollectDbgInfo->Reason, pCollectDbgInfo->pBuffer, pCollectDbgInfo->BufferSize, pCollectDbgInfo->BufferSize, pCollectDbgInfo->pExtension);

    pCollectDbgInfo->pExtension->CurrentDmaBufferOffset = 0;

    if(pCollectDbgInfo->Reason == VIDEO_TDR_TIMEOUT_DETECTED)
    {
        
    }

    return STATUS_SUCCESS;
}

static NTSTATUS AppendLogVidPn(size_t& currentLength, char*& messageBuffer, size_t& currentIndex, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    NTSTATUS status = RtlStringCbVPrintfA(messageBuffer + currentIndex, currentLength - currentIndex, fmt, args);

    if(status == STATUS_BUFFER_OVERFLOW)
    {
        currentLength <<= 1;
        char* newBuffer = static_cast<char*>(HyAllocateZeroed(PagedPool, currentLength * sizeof(char), POOL_TAG_LOGGING));
        (void) memcpy(newBuffer, messageBuffer, currentIndex);
        HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
        messageBuffer = newBuffer;

        va_start(args, fmt);

        status = RtlStringCbVPrintfA(messageBuffer + currentIndex, currentLength - currentIndex, fmt, args);
    }

    va_end(args);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to format vidpn, 0x%08X\n", status);
        HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
        messageBuffer = nullptr;
        return status;
    }

    // Check only the new part of the string.
    currentIndex += strlen(messageBuffer + currentIndex);

    return STATUS_SUCCESS;
}

void HyMiniportDevice::LogVidPn(D3DKMDT_HVIDPN hVidPn) noexcept
{
    const DXGK_VIDPN_INTERFACE* pVidPnInterface;
    NTSTATUS status = m_DxgkInterface.DxgkCbQueryVidPnInterface(hVidPn, DXGK_VIDPN_INTERFACE_VERSION_V1, &pVidPnInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get vidpn interface.\n");
        return;
    }

    const DXGK_MONITOR_INTERFACE* pMonitorInterface;
    status = m_DxgkInterface.DxgkCbQueryMonitorInterface(m_DxgkInterface.DeviceHandle, DXGK_MONITOR_INTERFACE_VERSION_V1, &pMonitorInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get monitor interface, 0x%08X.\n", status);
        pMonitorInterface = nullptr;
        // return;
    }

    D3DKMDT_HVIDPNTOPOLOGY hVidPnTopology;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* pVidPnTopologyInterface;
    status = pVidPnInterface->pfnGetTopology(hVidPn, &hVidPnTopology, &pVidPnTopologyInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get topology.\n");
        return;
    }

    size_t currentLength = 4096;
    char* messageBuffer = static_cast<char*>(HyAllocateZeroed(PagedPool, currentLength * sizeof(char), POOL_TAG_LOGGING));
    size_t currentIndex = 0;

    status = AppendLogVidPn(currentLength, messageBuffer, currentIndex, "V:0x%I64X::;", hVidPn);

    if(!NT_SUCCESS(status))
    {
        return;
    }

    const D3DKMDT_VIDPN_PRESENT_PATH* pPresentPath;
    status = pVidPnTopologyInterface->pfnAcquireFirstPathInfo(hVidPnTopology, &pPresentPath);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to acquire first path.\n");
        HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
        return;
    }

    while(status != STATUS_GRAPHICS_NO_MORE_ELEMENTS_IN_DATASET && pPresentPath)
    {
        status = AppendLogVidPn(currentLength, messageBuffer, currentIndex, "S:0x%08X:0x%I64X:;", pPresentPath->VidPnSourceId, hVidPn);

        if(!NT_SUCCESS(status))
        {
            return;
        }

        {
            D3DKMDT_HVIDPNSOURCEMODESET hSourceModeSet;
            const DXGK_VIDPNSOURCEMODESET_INTERFACE* pSourceModeSetInterface;
            status = pVidPnInterface->pfnAcquireSourceModeSet(hVidPn, pPresentPath->VidPnSourceId, &hSourceModeSet, &pSourceModeSetInterface);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to acquire source mode set.\n");
                HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                return;
            }

            const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode;
            status = pSourceModeSetInterface->pfnAcquireFirstModeInfo(hSourceModeSet, &pSourceMode);

            if(!NT_SUCCESS(status))
            {
                (void) pVidPnInterface->pfnReleaseSourceModeSet(hVidPn, hSourceModeSet);
                hSourceModeSet = nullptr;

                LOG_ERROR("Failed to acquire first source mode.\n");
                HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                return;
            }

            while(status != STATUS_GRAPHICS_DATASET_IS_EMPTY && pSourceMode)
            {
                if(pSourceMode->Type == D3DKMDT_RMT_GRAPHICS)
                {
                    status = AppendLogVidPn(
                        currentLength, 
                        messageBuffer, 
                        currentIndex, 
                        "SM:0x%08X:0x%08X:%ux%u,%ux%u,%u,%u,%u,%u;", 
                        pSourceMode->Id, 
                        pPresentPath->VidPnSourceId,
                        pSourceMode->Format.Graphics.PrimSurfSize.cx,
                        pSourceMode->Format.Graphics.PrimSurfSize.cy,
                        pSourceMode->Format.Graphics.VisibleRegionSize.cx,
                        pSourceMode->Format.Graphics.VisibleRegionSize.cy,
                        pSourceMode->Format.Graphics.Stride,
                        pSourceMode->Format.Graphics.PixelFormat,
                        pSourceMode->Format.Graphics.ColorBasis,
                        pSourceMode->Format.Graphics.PixelValueAccessMode
                    );
                }
                else
                {
                    status = AppendLogVidPn(currentLength, messageBuffer, currentIndex, "SM:0x%08X:0x%08X:unknown_type;", pSourceMode->Id, pPresentPath->VidPnSourceId);
                }

                if(!NT_SUCCESS(status))
                {
                    return;
                }

                const D3DKMDT_VIDPN_SOURCE_MODE* pPrevSourceMode = pSourceMode;
                status = pSourceModeSetInterface->pfnAcquireNextModeInfo(hSourceModeSet, pPrevSourceMode, &pSourceMode);
                (void) pSourceModeSetInterface->pfnReleaseModeInfo(hSourceModeSet, pPrevSourceMode);

                if(!NT_SUCCESS(status))
                {
                    (void) pVidPnInterface->pfnReleaseSourceModeSet(hVidPn, hSourceModeSet);
                    hSourceModeSet = nullptr;

                    LOG_ERROR("Failed to acquire next source mode.\n");
                    HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                    return;
                }
            }


            (void) pVidPnInterface->pfnReleaseSourceModeSet(hVidPn, hSourceModeSet);
            hSourceModeSet = nullptr;
        }

        status = AppendLogVidPn(
            currentLength, 
            messageBuffer, 
            currentIndex, 
            "T:0x%08X:0x%08X:%u,%u,0x%08X,%u,0x%08X,%ux%u,%ux%u,%u,%u.%u.%u.%u,%u,%u,%u,0x%08X,%u,0x%I64X;", 
            pPresentPath->VidPnTargetId, 
            pPresentPath->VidPnSourceId,
            pPresentPath->ImportanceOrdinal,
            pPresentPath->ContentTransformation.Scaling,
            pPresentPath->ContentTransformation.ScalingSupport,
            pPresentPath->ContentTransformation.Rotation,
            pPresentPath->ContentTransformation.RotationSupport,
            pPresentPath->VisibleFromActiveTLOffset.cx,
            pPresentPath->VisibleFromActiveTLOffset.cy,
            pPresentPath->VisibleFromActiveBROffset.cx,
            pPresentPath->VisibleFromActiveBROffset.cy,
            pPresentPath->VidPnTargetColorBasis,
            pPresentPath->VidPnTargetColorCoeffDynamicRanges.FirstChannel,
            pPresentPath->VidPnTargetColorCoeffDynamicRanges.SecondChannel,
            pPresentPath->VidPnTargetColorCoeffDynamicRanges.ThirdChannel,
            pPresentPath->VidPnTargetColorCoeffDynamicRanges.FourthChannel,
            pPresentPath->Content,
            pPresentPath->CopyProtection.CopyProtectionType,
            pPresentPath->CopyProtection.APSTriggerBits,
            pPresentPath->CopyProtection.CopyProtectionSupport,
            pPresentPath->GammaRamp.Type,
            pPresentPath->GammaRamp.DataSize
        );

        if(!NT_SUCCESS(status))
        {
            return;
        }

        {
            D3DKMDT_HVIDPNTARGETMODESET hTargetModeSet;
            const DXGK_VIDPNTARGETMODESET_INTERFACE* pTargetModeSetInterface;
            status = pVidPnInterface->pfnAcquireTargetModeSet(hVidPn, pPresentPath->VidPnTargetId, &hTargetModeSet, &pTargetModeSetInterface);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to acquire target mode set.\n");
                HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                return;
            }

            const D3DKMDT_VIDPN_TARGET_MODE* pTargetMode;
            status = pTargetModeSetInterface->pfnAcquireFirstModeInfo(hTargetModeSet, &pTargetMode);

            if(!NT_SUCCESS(status))
            {
                (void) pVidPnInterface->pfnReleaseTargetModeSet(hVidPn, hTargetModeSet);
                hTargetModeSet = nullptr;

                LOG_ERROR("Failed to acquire first target mode.\n");
                HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                return;
            }

            while(status != STATUS_GRAPHICS_DATASET_IS_EMPTY && pTargetMode)
            {
                status = AppendLogVidPn(
                    currentLength, 
                    messageBuffer, 
                    currentIndex, 
                    "TM:0x%08X:0x%08X:%u,%ux%u,%ux%u,%u/%u,%u/%u,%llu,%u,%u;", 
                    pTargetMode->Id, 
                    pPresentPath->VidPnTargetId,
                    pTargetMode->VideoSignalInfo.VideoStandard,
                    pTargetMode->VideoSignalInfo.TotalSize.cx,
                    pTargetMode->VideoSignalInfo.TotalSize.cy,
                    pTargetMode->VideoSignalInfo.ActiveSize.cx,
                    pTargetMode->VideoSignalInfo.ActiveSize.cy,
                    pTargetMode->VideoSignalInfo.VSyncFreq.Numerator,
                    pTargetMode->VideoSignalInfo.VSyncFreq.Denominator,
                    pTargetMode->VideoSignalInfo.HSyncFreq.Numerator,
                    pTargetMode->VideoSignalInfo.HSyncFreq.Denominator,
                    pTargetMode->VideoSignalInfo.PixelRate,
                    pTargetMode->VideoSignalInfo.ScanLineOrdering,
                    pTargetMode->Preference
                );

                if(!NT_SUCCESS(status))
                {
                    return;
                }

                const D3DKMDT_VIDPN_TARGET_MODE* pPrevTargetMode = pTargetMode;
                status = pTargetModeSetInterface->pfnAcquireNextModeInfo(hTargetModeSet, pPrevTargetMode, &pTargetMode);
                (void) pTargetModeSetInterface->pfnReleaseModeInfo(hTargetModeSet, pPrevTargetMode);

                if(!NT_SUCCESS(status))
                {
                    (void) pVidPnInterface->pfnReleaseTargetModeSet(hVidPn, hTargetModeSet);
                    hTargetModeSet = nullptr;

                    LOG_ERROR("Failed to acquire next target mode.\n");
                    HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                    return;
                }
            }

            (void) pVidPnInterface->pfnReleaseTargetModeSet(hVidPn, hTargetModeSet);
            hTargetModeSet = nullptr;
        }

        if(pMonitorInterface)
        {
            D3DKMDT_HMONITORSOURCEMODESET hMonitorSourceModeSet;
            const DXGK_MONITORSOURCEMODESET_INTERFACE* pMonitorSourceModeSetInterface;
            status = pMonitorInterface->pfnAcquireMonitorSourceModeSet(m_DxgkInterface.DeviceHandle, pPresentPath->VidPnTargetId, &hMonitorSourceModeSet, &pMonitorSourceModeSetInterface);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to acquire monitor mode set.\n");
                HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                return;
            }

            const D3DKMDT_MONITOR_SOURCE_MODE* pMonitorSourceMode;
            status = pMonitorSourceModeSetInterface->pfnAcquireFirstModeInfo(hMonitorSourceModeSet, &pMonitorSourceMode);

            if(!NT_SUCCESS(status))
            {
                (void) pMonitorInterface->pfnReleaseMonitorSourceModeSet(m_DxgkInterface.DeviceHandle, hMonitorSourceModeSet);
                hMonitorSourceModeSet = nullptr;

                LOG_ERROR("Failed to acquire first monitor source mode.\n");
                HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                return;
            }

            while(status != STATUS_GRAPHICS_DATASET_IS_EMPTY && pMonitorSourceMode)
            {
                status = AppendLogVidPn(
                    currentLength,
                    messageBuffer,
                    currentIndex,
                    "MM:0x%08X:0x%08X:%u,%ux%u,%ux%u,%u/%u,%u/%u,%llu,%u,%u,%u.%u.%u.%u,%u,%u;",
                    pMonitorSourceMode->Id,
                    pPresentPath->VidPnTargetId,
                    pMonitorSourceMode->VideoSignalInfo.VideoStandard,
                    pMonitorSourceMode->VideoSignalInfo.TotalSize.cx,
                    pMonitorSourceMode->VideoSignalInfo.TotalSize.cy,
                    pMonitorSourceMode->VideoSignalInfo.ActiveSize.cx,
                    pMonitorSourceMode->VideoSignalInfo.ActiveSize.cy,
                    pMonitorSourceMode->VideoSignalInfo.VSyncFreq.Numerator,
                    pMonitorSourceMode->VideoSignalInfo.VSyncFreq.Denominator,
                    pMonitorSourceMode->VideoSignalInfo.HSyncFreq.Numerator,
                    pMonitorSourceMode->VideoSignalInfo.HSyncFreq.Denominator,
                    pMonitorSourceMode->VideoSignalInfo.PixelRate,
                    pMonitorSourceMode->VideoSignalInfo.ScanLineOrdering,
                    pMonitorSourceMode->ColorBasis,
                    pMonitorSourceMode->ColorCoeffDynamicRanges.FirstChannel,
                    pMonitorSourceMode->ColorCoeffDynamicRanges.SecondChannel,
                    pMonitorSourceMode->ColorCoeffDynamicRanges.ThirdChannel,
                    pMonitorSourceMode->ColorCoeffDynamicRanges.FourthChannel,
                    pMonitorSourceMode->Origin,
                    pMonitorSourceMode->Preference
                );

                if(!NT_SUCCESS(status))
                {
                    return;
                }

                const D3DKMDT_MONITOR_SOURCE_MODE* pPrevMonitorSourceMode = pMonitorSourceMode;
                status = pMonitorSourceModeSetInterface->pfnAcquireNextModeInfo(hMonitorSourceModeSet, pPrevMonitorSourceMode, &pMonitorSourceMode);
                (void) pMonitorSourceModeSetInterface->pfnReleaseModeInfo(hMonitorSourceModeSet, pPrevMonitorSourceMode);

                if(!NT_SUCCESS(status))
                {
                    (void) pMonitorInterface->pfnReleaseMonitorSourceModeSet(m_DxgkInterface.DeviceHandle, hMonitorSourceModeSet);
                    hMonitorSourceModeSet = nullptr;

                    LOG_ERROR("Failed to acquire next monitor source mode.\n");
                    HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
                    return;
                }
            }

            (void) pMonitorInterface->pfnReleaseMonitorSourceModeSet(m_DxgkInterface.DeviceHandle, hMonitorSourceModeSet);
            hMonitorSourceModeSet = nullptr;
        }

        const D3DKMDT_VIDPN_PRESENT_PATH* pPrevPresentPath = pPresentPath;
        status = pVidPnTopologyInterface->pfnAcquireNextPathInfo(hVidPnTopology, pPrevPresentPath, &pPresentPath);
        (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pPrevPresentPath);

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to acquire next path.\n");
            HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
            return;
        }
    }

    LOG_DEBUG("VidPn: %s\n", messageBuffer);
    // DebugLog(messageBuffer, currentIndex, false);
    // DebugLog("\n", 1, false);

    HyDeallocate(messageBuffer, POOL_TAG_LOGGING);
}

// Largely sourced from https://github.com/OpenXT/xc-windows/blob/master/xengfx/wddm/miniport/vidpn.c#L133
static bool IsSupportedVpnPnPath(const D3DKMDT_VIDPN_PRESENT_PATH* const pVidPnPresentPath) noexcept
{
    // Bare minimum support to start with. OK with any of the uncommited states for transformations.
    if(pVidPnPresentPath->ContentTransformation.Scaling != D3DKMDT_VPPS_UNINITIALIZED &&
       pVidPnPresentPath->ContentTransformation.Scaling != D3DKMDT_VPPS_IDENTITY &&
       pVidPnPresentPath->ContentTransformation.Scaling != D3DKMDT_VPPS_CENTERED &&
       pVidPnPresentPath->ContentTransformation.Scaling != D3DKMDT_VPPS_UNPINNED &&
       pVidPnPresentPath->ContentTransformation.Scaling != D3DKMDT_VPPS_NOTSPECIFIED) {
        LOG_DEBUG("Unsupported Scaling value: %d\n", pVidPnPresentPath->ContentTransformation.Scaling);
        return false;
    }

    if(/*pVidPnPresentPath->ContentTransformation.ScalingSupport.Centered != 0 || */
       pVidPnPresentPath->ContentTransformation.ScalingSupport.Stretched != 0) {
        LOG_DEBUG("Unsupported ScalingSupport value: %d\n", *reinterpret_cast<const UINT*>(&pVidPnPresentPath->ContentTransformation.ScalingSupport));
        return false;
    }

    if(pVidPnPresentPath->ContentTransformation.Rotation != D3DKMDT_VPPR_UNINITIALIZED &&
       pVidPnPresentPath->ContentTransformation.Rotation != D3DKMDT_VPPR_IDENTITY &&
       pVidPnPresentPath->ContentTransformation.Rotation != D3DKMDT_VPPR_ROTATE90 &&
       pVidPnPresentPath->ContentTransformation.Rotation != D3DKMDT_VPPR_UNPINNED &&
       pVidPnPresentPath->ContentTransformation.Rotation != D3DKMDT_VPPR_NOTSPECIFIED) {
        LOG_DEBUG("Unsupported Rotation value: %d\n", pVidPnPresentPath->ContentTransformation.Rotation);
        return false;
    }

    if(/*pVidPnPresentPath->ContentTransformation.RotationSupport.Rotate90 != 0 ||*/
       pVidPnPresentPath->ContentTransformation.RotationSupport.Rotate180 != 0 ||
       pVidPnPresentPath->ContentTransformation.RotationSupport.Rotate270 != 0) {
        LOG_DEBUG("Unsupported RotationSupport value: %d\n", *reinterpret_cast<const UINT*>(&pVidPnPresentPath->ContentTransformation.RotationSupport));
        return false;
    }

    if(pVidPnPresentPath->VisibleFromActiveTLOffset.cx != 0 ||
       pVidPnPresentPath->VisibleFromActiveTLOffset.cy != 0 ||
       pVidPnPresentPath->VisibleFromActiveBROffset.cx != 0 ||
       pVidPnPresentPath->VisibleFromActiveBROffset.cy != 0) {
        LOG_DEBUG("TL/BR offsets are not supported.\n");
        return false;
    }

    if(pVidPnPresentPath->VidPnTargetColorBasis != D3DKMDT_CB_SRGB &&
       pVidPnPresentPath->VidPnTargetColorBasis != D3DKMDT_CB_SCRGB &&
       pVidPnPresentPath->VidPnTargetColorBasis != D3DKMDT_CB_UNINITIALIZED) {
        LOG_DEBUG("Unsupported ColorBasis: %d.\n", pVidPnPresentPath->VidPnTargetColorBasis);
        return false;
    }

    if(pVidPnPresentPath->Content != D3DKMDT_VPPC_UNINITIALIZED &&
       pVidPnPresentPath->Content != D3DKMDT_VPPC_GRAPHICS &&
       pVidPnPresentPath->Content != D3DKMDT_VPPC_VIDEO &&
       pVidPnPresentPath->Content != D3DKMDT_VPPC_NOTSPECIFIED) {
        LOG_DEBUG("Unsupported Content: %d.\n", pVidPnPresentPath->Content);
        return false;
    }

    if(pVidPnPresentPath->CopyProtection.CopyProtectionType != D3DKMDT_VPPMT_NOPROTECTION &&
       pVidPnPresentPath->CopyProtection.CopyProtectionType != D3DKMDT_VPPMT_UNINITIALIZED) {
        LOG_DEBUG("CopyProtection is not supported.\n");
        return false;
    }

    if(pVidPnPresentPath->GammaRamp.Type != D3DDDI_GAMMARAMP_DEFAULT &&
       pVidPnPresentPath->GammaRamp.Type != D3DDDI_GAMMARAMP_UNINITIALIZED) {
        LOG_DEBUG("Non-default gamma ramp is not supported.\n");
        return false;
    }

    return true;
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L22
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::IsSupportedVidPn(INOUT_PDXGKARG_ISSUPPORTEDVIDPN pIsSupportedVidPn) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    constexpr bool VerboseLogging = true;

    if constexpr(VerboseLogging)
    {
        LogVidPn(pIsSupportedVidPn->hDesiredVidPn);
    }

    if(pIsSupportedVidPn->hDesiredVidPn == nullptr)
    {
        if constexpr(VerboseLogging)
        {
            LOG_DEBUG("Desired video present network was null. This is always supported\n");
        }

        // A null desired VidPn is supported
        pIsSupportedVidPn->IsVidPnSupported = TRUE;
        return STATUS_SUCCESS;
    }

    // Default to not supported, until shown it is supported
    pIsSupportedVidPn->IsVidPnSupported = FALSE;

    const DXGK_VIDPN_INTERFACE* pVidPnInterface;
    NTSTATUS status = m_DxgkInterface.DxgkCbQueryVidPnInterface(pIsSupportedVidPn->hDesiredVidPn, DXGK_VIDPN_INTERFACE_VERSION_V1, &pVidPnInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get VidPn Interface: 0x%08X, hDesiredVidPn: 0x%I64X\n", status, pIsSupportedVidPn->hDesiredVidPn);
        return VAGUE_STATUS(status, STATUS_NO_MEMORY);
    }

    D3DKMDT_HVIDPNTOPOLOGY hVidPnTopology;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* pVidPnTopologyInterface;
    // Get the VidPn Topology interface so we can enumerate all paths.
    status = pVidPnInterface->pfnGetTopology(pIsSupportedVidPn->hDesiredVidPn, &hVidPnTopology, &pVidPnTopologyInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get VidPnTopology Interface: 0x%08X, hDesiredVidPn: 0x%I64X\n", status, pIsSupportedVidPn->hDesiredVidPn);
        return VAGUE_STATUS(status, STATUS_NO_MEMORY);
    }

    SIZE_T numPaths;
    status = pVidPnTopologyInterface->pfnGetNumPaths(hVidPnTopology, &numPaths);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to query number of paths in VidPN. hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
        return VAGUE_STATUS(status, STATUS_NO_MEMORY);
    }

    if constexpr(VerboseLogging)
    {
        LOG_DEBUG("MaxViews: %u, NumPaths: %zu\n", MaxViews, numPaths);
    }

    if(numPaths > static_cast<SIZE_T>(MaxViews))
    {
        LOG_ERROR("VidPN had more paths than we can handle. MaxViews: %u, NumPaths: %zu\n", MaxViews, numPaths);
        // This VidPn is not supported, which has already been set as the default
        return STATUS_GRAPHICS_INVALID_VIDPN_TOPOLOGY;
    }

    // For every source in this topology, make sure they don't have more paths than there are targets
    for(D3DDDI_VIDEO_PRESENT_SOURCE_ID sourceId = 0; sourceId < numPaths; ++sourceId)
    {
        SIZE_T numPathsFromSource;
        status = pVidPnTopologyInterface->pfnGetNumPathsFromSource(hVidPnTopology, sourceId, &numPathsFromSource);

        if(status == STATUS_GRAPHICS_SOURCE_NOT_IN_TOPOLOGY)
        {
            if constexpr(VerboseLogging)
            {
                LOG_DEBUG("Source not in topology. SourceId: %u\n", sourceId);
            }

            continue;
        }

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to get number of paths from source: 0x%08X, hDesiredVidPn: 0x%I64X, SourceId: 0x%X\n", status, pIsSupportedVidPn->hDesiredVidPn, sourceId);
            return VAGUE_STATUS(status, STATUS_NO_MEMORY);
        }

        if constexpr(VerboseLogging)
        {
            LOG_DEBUG("Number of Paths from Source: %zu\n", numPathsFromSource);
        }

        if(numPathsFromSource > MaxChildren)
        {
            LOG_DEBUG("VidPN is not supported.\n");
            // This VidPn is not supported, which has already been set as the default
            // return STATUS_GRAPHICS_INVALID_VIDPN_TOPOLOGY;
            return STATUS_SUCCESS;
        }
    }

    const D3DKMDT_VIDPN_PRESENT_PATH* pCurrVidPnPresentPathInfo;
    status = pVidPnTopologyInterface->pfnAcquireFirstPathInfo(hVidPnTopology, &pCurrVidPnPresentPathInfo);

    if(status == STATUS_GRAPHICS_DATASET_IS_EMPTY)
    {
        if constexpr(VerboseLogging)
        {
            LOG_DEBUG("No paths in topology.\n");
        }

        pIsSupportedVidPn->IsVidPnSupported = TRUE;
        return STATUS_SUCCESS;
    }

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed acquire first path info: 0x%08X hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
        return VAGUE_STATUS(status, STATUS_NO_MEMORY);
    }

    if constexpr(VerboseLogging)
    {
        LOG_DEBUG("Acquired first topology path. SourceId: %u, TargetId: %u\n", pCurrVidPnPresentPathInfo->VidPnSourceId, pCurrVidPnPresentPathInfo->VidPnTargetId);
    }

    while(true)
    {
        if(pCurrVidPnPresentPathInfo->VidPnSourceId > MaxViews)
        {
            LOG_ERROR("SourceId %u was greater than the MaxViews %u.\n", pCurrVidPnPresentPathInfo->VidPnSourceId, MaxViews);

            (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pCurrVidPnPresentPathInfo);
            return STATUS_GRAPHICS_INVALID_VIDPN_TOPOLOGY;
        }

        if(!IsSupportedVpnPnPath(pCurrVidPnPresentPathInfo))
        {
            (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pCurrVidPnPresentPathInfo);
            return STATUS_GRAPHICS_INVALID_VIDPN_TOPOLOGY;
        }

        if(pCurrVidPnPresentPathInfo->VidPnTargetId > MaxChildren)
        {
            LOG_ERROR("TargetId %u was greater than the MaxChildren %u.\n", pCurrVidPnPresentPathInfo->VidPnTargetId, MaxChildren);

            (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pCurrVidPnPresentPathInfo);
            return STATUS_GRAPHICS_INVALID_VIDPN_TOPOLOGY;
        }

        {
            D3DKMDT_HVIDPNTARGETMODESET hVidPnTargetModeSet;
            const DXGK_VIDPNTARGETMODESET_INTERFACE* pVidPnTargetModeSetInterface;
            status = pVidPnInterface->pfnAcquireTargetModeSet(
                pIsSupportedVidPn->hDesiredVidPn,
                pCurrVidPnPresentPathInfo->VidPnTargetId,
                &hVidPnTargetModeSet,
                &pVidPnTargetModeSetInterface
            );

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed acquire target mode set: 0x%08X TargetMode: %u\n", status, pCurrVidPnPresentPathInfo->VidPnTargetId);
                (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pCurrVidPnPresentPathInfo);
                return VAGUE_STATUS(status, STATUS_NO_MEMORY);
            }

            (void) pVidPnInterface->pfnReleaseTargetModeSet(pIsSupportedVidPn->hDesiredVidPn, hVidPnTargetModeSet);
        }

        {
            D3DKMDT_HVIDPNSOURCEMODESET hVidPnSourceModeSet;
            const DXGK_VIDPNSOURCEMODESET_INTERFACE* pVidPnSourceModeSetInterface;
            status = pVidPnInterface->pfnAcquireSourceModeSet(
                pIsSupportedVidPn->hDesiredVidPn,
                pCurrVidPnPresentPathInfo->VidPnTargetId,
                &hVidPnSourceModeSet,
                &pVidPnSourceModeSetInterface
            );

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed acquire source mode set: 0x%08X TargetMode: %u\n", status, pCurrVidPnPresentPathInfo->VidPnTargetId);
                (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pCurrVidPnPresentPathInfo);
                return VAGUE_STATUS(status, STATUS_NO_MEMORY);
            }

            (void) pVidPnInterface->pfnReleaseSourceModeSet(pIsSupportedVidPn->hDesiredVidPn, hVidPnSourceModeSet);
        }

        {
            const D3DKMDT_VIDPN_PRESENT_PATH* const prevPath = pCurrVidPnPresentPathInfo;
            status = pVidPnTopologyInterface->pfnAcquireNextPathInfo(hVidPnTopology, prevPath, &pCurrVidPnPresentPathInfo);

            (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, prevPath);
        }

        if(status == STATUS_GRAPHICS_NO_MORE_ELEMENTS_IN_DATASET)
        {
            pCurrVidPnPresentPathInfo = nullptr;
            break;
        }

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed acquire next path info: 0x%08X hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
            return VAGUE_STATUS(status, STATUS_NO_MEMORY);
        }

        if constexpr(VerboseLogging)
        {
            LOG_DEBUG("Acquired next topology path. SourceId: %u, TargetId: %u\n", pCurrVidPnPresentPathInfo->VidPnSourceId, pCurrVidPnPresentPathInfo->VidPnTargetId);
        }
    }

    if(pCurrVidPnPresentPathInfo)
    {
        (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pCurrVidPnPresentPathInfo);
    }

    // All sources succeeded so this VidPn is supported
    pIsSupportedVidPn->IsVidPnSupported = TRUE;

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::RecommendFunctionalVidPn(IN_CONST_PDXGKARG_RECOMMENDFUNCTIONALVIDPN_CONST pRecommendFunctionalVidPn) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    (void) pRecommendFunctionalVidPn;

    return STATUS_GRAPHICS_NO_RECOMMENDED_FUNCTIONAL_VIDPN;
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L111
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::EnumVidPnCofuncModality(IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    constexpr bool VerboseLogging = true;

    if constexpr(VerboseLogging)
    {
        LogVidPn(pEnumCofuncModality->hConstrainingVidPn);
    }

    const DXGK_VIDPN_INTERFACE* pVidPnInterface;
    // Get the VidPn Interface so we can get the 'Source Mode Set', 'Target Mode Set' and 'VidPn Topology' interfaces.
    NTSTATUS status = m_DxgkInterface.DxgkCbQueryVidPnInterface(pEnumCofuncModality->hConstrainingVidPn, DXGK_VIDPN_INTERFACE_VERSION_V1, &pVidPnInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get VidPn Interface: 0x%08X, hConstrainingVidPn: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn);
        return status;
    }

    D3DKMDT_HVIDPNTOPOLOGY hVidPnTopology;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* pVidPnTopologyInterface;
    // Get the VidPn Topology interface so we can enumerate all paths.
    status = pVidPnInterface->pfnGetTopology(pEnumCofuncModality->hConstrainingVidPn, &hVidPnTopology, &pVidPnTopologyInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get VidPnTopology Interface: 0x%08X, hConstrainingVidPn: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn);
        return status;
    }

    const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPresentPath;
    // Get the first path before we start looping through them.
    status = pVidPnTopologyInterface->pfnAcquireFirstPathInfo(hVidPnTopology, &pVidPnPresentPath);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get first present path: 0x%08X, hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
        return status;
    }

    D3DKMDT_HVIDPNSOURCEMODESET hVidPnSourceModeSet = nullptr;
    const DXGK_VIDPNSOURCEMODESET_INTERFACE* pVidPnSourceModeSetInterface = nullptr;
    const D3DKMDT_VIDPN_SOURCE_MODE* pVidPnPinnedSourceModeInfo = nullptr;
    D3DKMDT_HVIDPNTARGETMODESET hVidPnTargetModeSet = nullptr;
    const DXGK_VIDPNTARGETMODESET_INTERFACE* pVidPnTargetModeSetInterface = nullptr;
    const D3DKMDT_VIDPN_TARGET_MODE* pVidPnPinnedTargetModeInfo = nullptr;
    const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPresentPathTemp = nullptr;

    // Loop through all available paths.
    while(status != STATUS_GRAPHICS_NO_MORE_ELEMENTS_IN_DATASET)
    {
        if constexpr(VerboseLogging)
        {
            LOG_DEBUG("Looping\n");
        }

        status = pVidPnInterface->pfnAcquireSourceModeSet(
            pEnumCofuncModality->hConstrainingVidPn,
            pVidPnPresentPath->VidPnSourceId,
            &hVidPnSourceModeSet,
            &pVidPnSourceModeSetInterface
        );

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to acquire Source Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, SourceId: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnSourceId);
            break;
        }

        status = pVidPnSourceModeSetInterface->pfnAcquirePinnedModeInfo(hVidPnSourceModeSet, &pVidPnPinnedSourceModeInfo);

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to acquire pinned Source Mode Info: 0x%08X, hSourceModeSet: 0x%I64X\n", status, hVidPnSourceModeSet);
            break;
        }

        // If this source mode isn't the pivot point, do work on the source mode set
        if(pEnumCofuncModality->EnumPivotType != D3DKMDT_EPT_VIDPNSOURCE || pEnumCofuncModality->EnumPivot.VidPnSourceId != pVidPnPresentPath->VidPnSourceId)
        {
            // If there's no pinned source add possible modes (otherwise they've already been added)
            if(!pVidPnPinnedSourceModeInfo)
            {
                if constexpr(VerboseLogging)
                {
                    LOG_DEBUG("Adding modes to unpinned source.\n");
                }

                // Release the acquired source mode set, since going to create a new one to put all modes in
                status = pVidPnInterface->pfnReleaseSourceModeSet(pEnumCofuncModality->hConstrainingVidPn, hVidPnSourceModeSet);

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to release Source Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, hVidPnSourceModeSet: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, hVidPnSourceModeSet);
                    break;
                }
                // We successfully released it.
                hVidPnSourceModeSet = nullptr;

                status = pVidPnInterface->pfnCreateNewSourceModeSet(
                    pEnumCofuncModality->hConstrainingVidPn,
                    pVidPnPresentPath->VidPnSourceId,
                    &hVidPnSourceModeSet,
                    &pVidPnSourceModeSetInterface
                );

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to create new Source Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, SourceId: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnSourceId);
                    break;
                }

                // Add the appropriate modes to the source mode set
                status = AddSingleSourceMode(pVidPnSourceModeSetInterface, hVidPnSourceModeSet, pVidPnPresentPath->VidPnSourceId);

                if(!NT_SUCCESS(status))
                {
                    break;
                }

                // Assign the source modes we just created.
                status = pVidPnInterface->pfnAssignSourceModeSet(pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnSourceId, hVidPnSourceModeSet);

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to assign new Source Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, SourceId: 0x%I64X, hVidPnSourceModeSet: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnSourceId, hVidPnSourceModeSet);
                    break;
                }

                // We successfully assigned.
                hVidPnSourceModeSet = nullptr;
            }
        }

        // If this target mode isn't the pivot point, do work on the target mode set
        if(pEnumCofuncModality->EnumPivotType != D3DKMDT_EPT_VIDPNTARGET || pEnumCofuncModality->EnumPivot.VidPnTargetId != pVidPnPresentPath->VidPnTargetId)
        {
            status = pVidPnInterface->pfnAcquireTargetModeSet(
                pEnumCofuncModality->hConstrainingVidPn,
                pVidPnPresentPath->VidPnTargetId,
                &hVidPnTargetModeSet,
                &pVidPnTargetModeSetInterface
            );

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to acquire Target Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, TargetId: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnTargetId);
                break;
            }

            status = pVidPnTargetModeSetInterface->pfnAcquirePinnedModeInfo(hVidPnTargetModeSet, &pVidPnPinnedTargetModeInfo);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to acquire pinned Target Mode Set: 0x%08X, hVidPnTargetModeSet: 0x%I64X\n", status, hVidPnTargetModeSet);
                break;
            }

            // If there's no pinned target add possible modes, otherwise they've already been added.
            if(!pVidPnPinnedTargetModeInfo)
            {
                if constexpr(VerboseLogging)
                {
                    LOG_DEBUG("Adding modes to unpinned target.\n");
                }

                // Release the acquired target mode set, since going to create a new one to put all modes in
                status = pVidPnInterface->pfnReleaseTargetModeSet(pEnumCofuncModality->hConstrainingVidPn, hVidPnTargetModeSet);

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to release Target Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, hVidPnTargetModeSet: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, hVidPnTargetModeSet);
                    break;
                }

                // We successfully released it.
                hVidPnTargetModeSet = nullptr;

                // Create a new target mode set which will be added to the constraining VidPn with all the possible modes
                status = pVidPnInterface->pfnCreateNewTargetModeSet(
                    pEnumCofuncModality->hConstrainingVidPn,
                    pVidPnPresentPath->VidPnTargetId,
                    &hVidPnTargetModeSet,
                    &pVidPnTargetModeSetInterface
                );

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to create new Target Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, TargetId: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnTargetId);
                    break;
                }

                status = AddSingleTargetMode(pVidPnTargetModeSetInterface, hVidPnTargetModeSet, pVidPnPinnedSourceModeInfo, pVidPnPresentPath->VidPnSourceId);

                if(!NT_SUCCESS(status))
                {
                    break;
                }

                // Assign the source modes we just created.
                status = pVidPnInterface->pfnAssignTargetModeSet(pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnTargetId, hVidPnTargetModeSet);

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to assign new Target Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, TargetId: 0x%I64X, hVidPnTargetModeSet: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, pVidPnPresentPath->VidPnTargetId, hVidPnTargetModeSet);
                    break;
                }

                // We successfully assigned it.
                hVidPnTargetModeSet = nullptr;
            }
            else
            {
                if constexpr(VerboseLogging)
                {
                    LOG_DEBUG("No modes added to unpinned source.\n");
                }

                // Release the pinned target as there's no other work to do
                status = pVidPnTargetModeSetInterface->pfnReleaseModeInfo(hVidPnTargetModeSet, pVidPnPinnedTargetModeInfo);

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to release Target Mode Info: 0x%08X, hVidPnTargetModeSet: 0x%I64X, pVidPnPinnedTargetModeInfo: 0x%I64X\n", status, hVidPnTargetModeSet, pVidPnPinnedTargetModeInfo);
                    break;
                }

                // We successfully released it.
                pVidPnPinnedTargetModeInfo = nullptr;

                // Release the acquired target mode set, since it is no longer needed
                status = pVidPnInterface->pfnReleaseTargetModeSet(pEnumCofuncModality->hConstrainingVidPn, hVidPnTargetModeSet);

                if(!NT_SUCCESS(status))
                {
                    LOG_ERROR("Failed to release Target Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, hVidPnTargetModeSet: 0x%I64X\n", status, pVidPnPresentPath->VidPnTargetId, hVidPnTargetModeSet);
                    break;
                }

                // We successfully released it.
                hVidPnTargetModeSet = nullptr;
            }
        }

        // Nothing else needs the pinned source mode so release it
        if(pVidPnPinnedSourceModeInfo)
        {
            if constexpr(VerboseLogging)
            {
                LOG_DEBUG("Nothing else pinned.\n");
            }

            status = pVidPnSourceModeSetInterface->pfnReleaseModeInfo(hVidPnSourceModeSet, pVidPnPinnedSourceModeInfo);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to release Source Mode Info: 0x%08X, hVidPnSourceModeSet: 0x%I64X, pVidPnPinnedSourceModeInfo: 0x%I64X\n", status, hVidPnSourceModeSet, pVidPnPinnedSourceModeInfo);
                break;
            }

            // We successfully released it.
            pVidPnPinnedSourceModeInfo = nullptr;
        }

        // With the pinned source mode now released, if the source mode set hasn't been released, release that as well
        if(hVidPnSourceModeSet != nullptr)
        {
            status = pVidPnInterface->pfnReleaseSourceModeSet(pEnumCofuncModality->hConstrainingVidPn, hVidPnSourceModeSet);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to release Source Mode Set: 0x%08X, hConstrainingVidPn: 0x%I64X, hVidPnSourceModeSet: 0x%I64X\n", status, pEnumCofuncModality->hConstrainingVidPn, hVidPnSourceModeSet);
                break;
            }

            // We successfully released it.
            hVidPnSourceModeSet = nullptr;
        }

        // If modifying support fields, need to modify a local version of a path structure since the retrieved one is const
        D3DKMDT_VIDPN_PRESENT_PATH localVidPnPresentPath = *pVidPnPresentPath;
        bool supportFieldsModified = false;

        // SCALING: If this path's scaling isn't the pivot point, do work on the scaling support
        if(pEnumCofuncModality->EnumPivotType != D3DKMDT_EPT_SCALING || 
           pEnumCofuncModality->EnumPivot.VidPnSourceId != pVidPnPresentPath->VidPnSourceId || 
           pEnumCofuncModality->EnumPivot.VidPnTargetId != pVidPnPresentPath->VidPnTargetId)
        {
            // If the scaling is unpinned, then modify the scaling support field
            if(pVidPnPresentPath->ContentTransformation.Scaling == D3DKMDT_VPPS_UNPINNED)
            {
                if constexpr(VerboseLogging)
                {
                    LOG_DEBUG("VPPS_UNPINNED\n");
                }

                // Identity and centered scaling are supported, but not any stretch modes
                RtlZeroMemory(&localVidPnPresentPath.ContentTransformation.ScalingSupport, sizeof(D3DKMDT_VIDPN_PRESENT_PATH_SCALING_SUPPORT));
                localVidPnPresentPath.ContentTransformation.ScalingSupport.Identity = 1;
                localVidPnPresentPath.ContentTransformation.ScalingSupport.Centered = 1;
                supportFieldsModified = true;
            }
        }

        // ROTATION: If this path's rotation isn't the pivot point, do work on the rotation support
        if(pEnumCofuncModality->EnumPivotType != D3DKMDT_EPT_ROTATION || 
           pEnumCofuncModality->EnumPivot.VidPnSourceId != pVidPnPresentPath->VidPnSourceId || 
           pEnumCofuncModality->EnumPivot.VidPnTargetId != pVidPnPresentPath->VidPnTargetId)
        {
            // If the rotation is unpinned, then modify the rotation support field
            if(pVidPnPresentPath->ContentTransformation.Rotation == D3DKMDT_VPPR_UNPINNED)
            {
                if constexpr(VerboseLogging)
                {
                    LOG_DEBUG("VPPR_UNPINNED\n");
                }

                localVidPnPresentPath.ContentTransformation.RotationSupport.Identity = 1;
                // Sample supports only Rotate90
                localVidPnPresentPath.ContentTransformation.RotationSupport.Rotate90 = 1;
                localVidPnPresentPath.ContentTransformation.RotationSupport.Rotate180 = 0;
                localVidPnPresentPath.ContentTransformation.RotationSupport.Rotate270 = 0;

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM1_3_PATH_INDEPENDENT_ROTATION)
                // Since clone is not supported, should not support path-independent rotations
                localVidPnPresentPath.ContentTransformation.RotationSupport.Offset0 = 1;
                localVidPnPresentPath.ContentTransformation.RotationSupport.Offset90 = 0;
                localVidPnPresentPath.ContentTransformation.RotationSupport.Offset180 = 0;
                localVidPnPresentPath.ContentTransformation.RotationSupport.Offset270 = 0;
#endif

                supportFieldsModified = true;
            }
        }

        if(supportFieldsModified)
        {
            if constexpr(VerboseLogging)
            {
                LOG_DEBUG("Updating support info.\n");
            }

            status = pVidPnTopologyInterface->pfnUpdatePathSupportInfo(hVidPnTopology, &localVidPnPresentPath);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to update Path Support Info : 0x%08X, hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
                break;
            }
        }

        // Get the next path...
        // (NOTE: This is the value of Status that will return STATUS_GRAPHICS_NO_MORE_ELEMENTS_IN_DATASET when it's time to quit the loop)
        pVidPnPresentPathTemp = pVidPnPresentPath;
        status = pVidPnTopologyInterface->pfnAcquireNextPathInfo(hVidPnTopology, pVidPnPresentPathTemp, &pVidPnPresentPath);

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to acquire next Path Info : 0x%08X, hVidPnTopology: 0x%I64X, pVidPnPresentPathTemp: 0x%I64X\n", status, hVidPnTopology, pVidPnPresentPathTemp);
            break;
        }

        // ...and release the last path
        const NTSTATUS tempStatus = pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pVidPnPresentPathTemp);

        if(!NT_SUCCESS(tempStatus))
        {
            LOG_ERROR("Failed to release last Path Info : 0x%08X, hVidPnTopology: 0x%I64X, pVidPnPresentPathTemp: 0x%I64X\n", status, hVidPnTopology, pVidPnPresentPathTemp);
            status = tempStatus;
            break;
        }

        pVidPnPresentPathTemp = nullptr; // Successfully released it
    }

    // If quit the while loop normally, set the return value to success
    if(status == STATUS_GRAPHICS_NO_MORE_ELEMENTS_IN_DATASET)
    {
        status = STATUS_SUCCESS;
    }

    // Release any resources hanging around because the loop was quit early.
    // Since in normal execution everything should be released by this point, TempStatus is initialized to a bogus error to be used as an
    //  assertion that if anything had to be released now (TempStatus changing) Status isn't successful.
    NTSTATUS tempStatus = STATUS_SUCCESS;

    if(pVidPnSourceModeSetInterface && pVidPnPinnedSourceModeInfo)
    {
        tempStatus = pVidPnSourceModeSetInterface->pfnReleaseModeInfo(hVidPnSourceModeSet, pVidPnPinnedSourceModeInfo);
        if(!NT_SUCCESS(tempStatus))
        {
            LOG_WARN("HyMiniportDevice::EnumVidPnCofuncModality: TEMP Status was 0x%08X when trying to clean up pVidPnPinnedSourceModeInfo.\n", tempStatus);
        }
    }

    if(pVidPnTargetModeSetInterface && pVidPnPinnedTargetModeInfo)
    {
        tempStatus = pVidPnTargetModeSetInterface->pfnReleaseModeInfo(hVidPnTargetModeSet, pVidPnPinnedTargetModeInfo);
        if(!NT_SUCCESS(tempStatus))
        {
            LOG_WARN("HyMiniportDevice::EnumVidPnCofuncModality: TEMP Status was 0x%08X when trying to clean up pVidPnPinnedTargetModeInfo.\n", tempStatus);
        }
    }

    if(pVidPnPresentPath)
    {
        tempStatus = pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pVidPnPresentPath);
        if(!NT_SUCCESS(tempStatus))
        {
            LOG_WARN("HyMiniportDevice::EnumVidPnCofuncModality: TEMP Status was 0x%08X when trying to clean up pVidPnPresentPath.\n", tempStatus);
        }
    }

    if(pVidPnPresentPathTemp)
    {
        tempStatus = pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pVidPnPresentPathTemp);
        if(!NT_SUCCESS(tempStatus))
        {
            LOG_WARN("HyMiniportDevice::EnumVidPnCofuncModality: TEMP Status was 0x%08X when trying to clean up pVidPnPresentPathTemp.\n", tempStatus);
        }
    }

    if(hVidPnSourceModeSet != nullptr)
    {
        tempStatus = pVidPnInterface->pfnReleaseSourceModeSet(pEnumCofuncModality->hConstrainingVidPn, hVidPnSourceModeSet);
        if(!NT_SUCCESS(tempStatus))
        {
            LOG_WARN("HyMiniportDevice::EnumVidPnCofuncModality: TEMP Status was 0x%08X when trying to clean up hVidPnSourceModeSet.\n", tempStatus);
        }
    }

    if(hVidPnTargetModeSet != nullptr)
    {
        tempStatus = pVidPnInterface->pfnReleaseTargetModeSet(pEnumCofuncModality->hConstrainingVidPn, hVidPnTargetModeSet);
        if(!NT_SUCCESS(tempStatus))
        {
            LOG_WARN("HyMiniportDevice::EnumVidPnCofuncModality: TEMP Status was 0x%08X when trying to clean up hVidPnTargetModeSet.\n", tempStatus);
        }
    }

    if constexpr(VerboseLogging)
    {
        LogVidPn(pEnumCofuncModality->hConstrainingVidPn);
    }

    return status;
}

#pragma code_seg(push)
#pragma code_seg("_KTEXT")
NTSTATUS HyMiniportDevice::SetVidPnSourceAddress(IN_CONST_PDXGKARG_SETVIDPNSOURCEADDRESS pSetVidPnSourceAddress) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG(
        "Setting source address for %u to %u:0x%016llX, Context Count: %u\n", 
        pSetVidPnSourceAddress->VidPnSourceId, 
        pSetVidPnSourceAddress->PrimarySegment, 
        pSetVidPnSourceAddress->PrimaryAddress.QuadPart,
        pSetVidPnSourceAddress->ContextCount
    );

    return STATUS_SUCCESS;
}
#pragma code_seg(pop)

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L469
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::SetVidPnSourceVisibility(IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    const UINT StartVidPnSourceId = (pSetVidPnSourceVisibility->VidPnSourceId == D3DDDI_ID_ALL) ? 0 : pSetVidPnSourceVisibility->VidPnSourceId;
    const UINT MaxVidPnSourceId = (pSetVidPnSourceVisibility->VidPnSourceId == D3DDDI_ID_ALL) ? MaxViews : pSetVidPnSourceVisibility->VidPnSourceId + 1;

    LOG_DEBUG("Setting Displays %d-%d, Visible: %d\n", StartVidPnSourceId, MaxVidPnSourceId, pSetVidPnSourceVisibility->Visible);

    (void) StartVidPnSourceId;
    (void) MaxVidPnSourceId;

    for(UINT sourceId = StartVidPnSourceId; sourceId < MaxVidPnSourceId; ++sourceId)
    {
        if(m_Flags.IsStarted)
        {
            if(pSetVidPnSourceVisibility->Visible)
            {
                // Not sure why we only enable, and not disable here...
                m_CurrentDisplayMode[sourceId].Flags.FullscreenPresent = true;
            }

            // Set Display State
            SetDisplayState(sourceId, pSetVidPnSourceVisibility->Visible);
        }

        // Store current visibility so it can be dealt with during Present call
        m_CurrentDisplayMode[sourceId].Flags.SourceNotVisible = !(pSetVidPnSourceVisibility->Visible);
    }

    return STATUS_SUCCESS;
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L500
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::CommitVidPn(IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn) noexcept
{
    // Check this CommitVidPn is for the mode change notification when monitor is in power off state.
    if(pCommitVidPn->Flags.PathPoweredOff)
    {
        return STATUS_SUCCESS;
    }

    const DXGK_VIDPN_INTERFACE* pVidPnInterface = nullptr;
    D3DKMDT_HVIDPNTOPOLOGY hVidPnTopology = nullptr;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* pVidPnTopologyInterface = nullptr;
    SIZE_T numPaths = 0;
    D3DKMDT_HVIDPNSOURCEMODESET hVidPnSourceModeSet = nullptr;
    const DXGK_VIDPNSOURCEMODESET_INTERFACE* pVidPnSourceModeSetInterface = nullptr;
    const D3DKMDT_VIDPN_SOURCE_MODE* pPinnedVidPnSourceModeInfo = nullptr;
    const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPresentPath = nullptr;
    SIZE_T numPathsFromSource = 0;
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId = D3DDDI_ID_UNINITIALIZED;

    // I don't like capturing lambdas, but it's better than goto label, in some ways.
    const auto cleanupHandler = [&]()
    {
        if(pVidPnSourceModeSetInterface && hVidPnSourceModeSet != nullptr && pPinnedVidPnSourceModeInfo)
        {
            (void) pVidPnSourceModeSetInterface->pfnReleaseModeInfo(hVidPnSourceModeSet, pPinnedVidPnSourceModeInfo);
        }

        if(pVidPnInterface && pCommitVidPn->hFunctionalVidPn != nullptr && hVidPnSourceModeSet != nullptr)
        {
            (void) pVidPnInterface->pfnReleaseSourceModeSet(pCommitVidPn->hFunctionalVidPn, hVidPnSourceModeSet);
        }

        if(pVidPnTopologyInterface && hVidPnTopology != nullptr && pVidPnPresentPath)
        {
            (void) pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pVidPnPresentPath);
        }
    };

    // Get the VidPn Interface so we can get the 'Source Mode Set', 'Target Mode Set' and 'VidPn Topology' interfaces.
    NTSTATUS status = m_DxgkInterface.DxgkCbQueryVidPnInterface(pCommitVidPn->hFunctionalVidPn, DXGK_VIDPN_INTERFACE_VERSION_V1, &pVidPnInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get VidPn Interface: 0x%08X, hFunctionalVidPn: 0x%I64X\n", status, pCommitVidPn->hFunctionalVidPn);
        cleanupHandler();
        return status;
    }

    // Get the VidPn Topology interface so we can enumerate all paths.
    status = pVidPnInterface->pfnGetTopology(pCommitVidPn->hFunctionalVidPn, &hVidPnTopology, &pVidPnTopologyInterface);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get VidPnTopology Interface: 0x%08X, hFunctionalVidPn: 0x%I64X\n", status, pCommitVidPn->hFunctionalVidPn);
        cleanupHandler();
        return status;
    }

    // Find out the number of paths now, if it's 0 don't bother with source mode set and pinned mode, just clear current and then quit
    status = pVidPnTopologyInterface->pfnGetNumPaths(hVidPnTopology, &numPaths);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get number of paths: 0x%08X, hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
        cleanupHandler();
        return status;
    }

    if(numPaths != 0)
    {
        // Get the Source Mode Set interface so we can get the pinned mode
        status = pVidPnInterface->pfnAcquireSourceModeSet(
            pCommitVidPn->hFunctionalVidPn,
            pCommitVidPn->AffectedVidPnSourceId,
            &hVidPnSourceModeSet,
            &pVidPnSourceModeSetInterface
        );

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to acquire Source Mode Set: 0x%08X, hFunctionalVidPn: 0x%I64X, SourceId: 0x%I64X\n", status, pCommitVidPn->hFunctionalVidPn, pCommitVidPn->AffectedVidPnSourceId);
            cleanupHandler();
            return status;
        }

        // Get the mode that is being pinned
        status = pVidPnSourceModeSetInterface->pfnAcquirePinnedModeInfo(hVidPnSourceModeSet, &pPinnedVidPnSourceModeInfo);

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to acquire Pinned Mode Set: 0x%08X, hFunctionalVidPn: 0x%I64X\n", status, pCommitVidPn->hFunctionalVidPn);
            cleanupHandler();
            return status;
        }
    }
    else
    {
        // This will cause the successful quit below
        pPinnedVidPnSourceModeInfo = nullptr;
    }

    if(m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].FrameBufferPointer && !m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].Flags.DoNotMapOrUnmap)
    {
        status = UnmapFrameBuffer(
            m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].FrameBufferPointer, 
            m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].DisplayInfo.Pitch * m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].DisplayInfo.Height
        );

        m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].FrameBufferPointer = nullptr;
        m_CurrentDisplayMode[pCommitVidPn->AffectedVidPnSourceId].Flags.FrameBufferIsActive = false;

        if(!NT_SUCCESS(status))
        {
            cleanupHandler();
            return status;
        }
    }

    if(!pPinnedVidPnSourceModeInfo)
    {
        // There is no mode to pin on this source, any old paths here have already been cleared
        status = STATUS_SUCCESS;
        cleanupHandler();
        return status;
    }

    status = AreVidPnSourceModeFieldsValid(pPinnedVidPnSourceModeInfo);

    if(!NT_SUCCESS(status))
    {
        cleanupHandler();
        return status;
    }

    status = pVidPnTopologyInterface->pfnGetNumPathsFromSource(hVidPnTopology, pCommitVidPn->AffectedVidPnSourceId, &numPathsFromSource);
    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to get number of paths from source: 0x%08X, hVidPnTopology: 0x%I64X\n", status, hVidPnTopology);
        cleanupHandler();
        return status;
    }

    // Loop through all paths to set this mode
    for(SIZE_T pathIndex = 0; pathIndex < numPathsFromSource; ++pathIndex)
    {
        // Get the target id for this path
        status = pVidPnTopologyInterface->pfnEnumPathTargetsFromSource(hVidPnTopology, pCommitVidPn->AffectedVidPnSourceId, pathIndex, &TargetId);
        m_CurrentDisplayMode[0].VidPnTargetId = TargetId;

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("pfnEnumPathTargetsFromSource failed with Status = 0x%I64x, hVidPnTopology = 0x%I64x, SourceId = 0x%I64x, PathIndex = 0x%I64x\n", status, hVidPnTopology, pCommitVidPn->AffectedVidPnSourceId, pathIndex);
            cleanupHandler();
            return status;
        }

        // Get the actual path info
        status = pVidPnTopologyInterface->pfnAcquirePathInfo(hVidPnTopology, pCommitVidPn->AffectedVidPnSourceId, TargetId, &pVidPnPresentPath);
        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("pfnAcquirePathInfo failed with Status = 0x%I64x, hVidPnTopology = 0x%I64x, SourceId = 0x%I64x, TargetId = 0x%I64x\n", status, hVidPnTopology, pCommitVidPn->AffectedVidPnSourceId, TargetId);
            cleanupHandler();
            return status;
        }

        status = AreVidPnPathFieldsValid(pVidPnPresentPath);
        if(!NT_SUCCESS(status))
        {
            cleanupHandler();
            return status;
        }

        status = SetSourceModeAndPath(pPinnedVidPnSourceModeInfo, pVidPnPresentPath);
        if(!NT_SUCCESS(status))
        {
            cleanupHandler();
            return status;
        }

        status = pVidPnTopologyInterface->pfnReleasePathInfo(hVidPnTopology, pVidPnPresentPath);
        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("pfnReleasePathInfo failed with Status = 0x%I64x, hVidPnTopoogy = 0x%I64x, pVidPnPresentPath = 0x%I64x\n", status, hVidPnTopology, pVidPnPresentPath);
            cleanupHandler();
            return status;
        }

        // We successfully released it.
        pVidPnPresentPath = nullptr; 
    }

    cleanupHandler();

    return status;
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L687
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::UpdateActiveVidPnPresentPath(IN_CONST_PDXGKARG_UPDATEACTIVEVIDPNPRESENTPATH_CONST pUpdateActiveVidPnPresentPath) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    const NTSTATUS status = AreVidPnPathFieldsValid(&pUpdateActiveVidPnPresentPath->VidPnPresentPathInfo);

    if(!NT_SUCCESS(status))
    {
        return status;
    }

    m_CurrentDisplayMode[pUpdateActiveVidPnPresentPath->VidPnPresentPathInfo.VidPnSourceId].Flags.FullscreenPresent = true;
    m_CurrentDisplayMode[pUpdateActiveVidPnPresentPath->VidPnPresentPathInfo.VidPnSourceId].Rotation = pUpdateActiveVidPnPresentPath->VidPnPresentPathInfo.ContentTransformation.Rotation;

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::RecommendMonitorModes(IN_CONST_PDXGKARG_RECOMMENDMONITORMODES_CONST pRecommendMonitorModes) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);
    LOG_DEBUG(
        "Target: %u\n",
        pRecommendMonitorModes->VideoPresentTargetId
    );

    SIZE_T numModes;
    NTSTATUS status = pRecommendMonitorModes->pMonitorSourceModeSetInterface->pfnGetNumModes(pRecommendMonitorModes->hMonitorSourceModeSet, &numModes);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to query number of modes. 0x%08X\n", status);
        return status;
    }

    LOG_DEBUG("Num Modes: %zu\n", numModes);

    const D3DKMDT_MONITOR_SOURCE_MODE* sourceMode;
    status = pRecommendMonitorModes->pMonitorSourceModeSetInterface->pfnAcquirePreferredModeInfo(pRecommendMonitorModes->hMonitorSourceModeSet, &sourceMode);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to query preferred mode. 0x%08X\n", status);
        return status;
    }

    if(status != STATUS_GRAPHICS_NO_PREFERRED_MODE)
    {
        LOG_DEBUG("Preferred Mode Found.\n");
    }

    // This doesn't actually cause any real problems if we pass null to ReleaseModeInfo,
    // But it does trigger a WatchDog Error (not a real watchdog, just an error logger for dxgkrnl.sys)
    if(sourceMode)
    {
        status = pRecommendMonitorModes->pMonitorSourceModeSetInterface->pfnReleaseModeInfo(pRecommendMonitorModes->hMonitorSourceModeSet, sourceMode);
    }

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to release preferred mode. 0x%08X\n", status);
        return status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::GetScanLine(INOUT_PDXGKARG_GETSCANLINE pGetScanLine) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    // Presumably this only applies to CRT monitors.
    pGetScanLine->InVerticalBlank = FALSE;
    // Our GPU doesn't really handle scan-lines yet.
    pGetScanLine->ScanLine = 0;

    return STATUS_SUCCESS;
}

#if DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8
// Given pixel format, give back the bits per pixel. Only supports pixel formats expected by BDD
// (i.e. the ones found below in PixelFormatFromBPP or that may come in from FallbackStart)
// This is because these two functions combine to allow BDD to store the bpp of a VBE mode in the
// ColorFormat field of a DispInfo
// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD
// Thus it is subject to the Microsoft Public License.
static UINT BPPFromPixelFormat(D3DDDIFORMAT Format)
{
    switch(Format)
    {
        case D3DDDIFMT_UNKNOWN: return 0;
        case D3DDDIFMT_P8: return 8;
        case D3DDDIFMT_R5G6B5: return 16;
        case D3DDDIFMT_R8G8B8: return 24;
        case D3DDDIFMT_X8R8G8B8: // fall through
        case D3DDDIFMT_A8R8G8B8: return 32;
        default: return 0;
    }
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L444
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::PresentDisplayOnly(IN_CONST_PDXGKARG_PRESENT_DISPLAYONLY pPresentDisplayOnly) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    //if(m_Flags.IsStarted)
    //{
    //    LOG_ERROR("HyMiniportDevice::PresentDisplayOnly called while the device is not started.\n");
    //    return STATUS_UNSUCCESSFUL;
    //}

    if(pPresentDisplayOnly->BytesPerPixel != 4)
    {
        LOG_ERROR("pPresentDisplayOnly->BytesPerPixel != 4.\n");
        return STATUS_INVALID_PARAMETER;
    }

    HyDisplayMode& currentMode = m_CurrentDisplayMode[pPresentDisplayOnly->VidPnSourceId];

    // If it is in monitor off state or source is not supposed to be visible, don't present anything to the screen
    if(currentMode.PowerState > PowerDeviceD0 || currentMode.Flags.SourceNotVisible)
    {
        LOG_DEBUG("Display is in a low power state or source is not visible.\n");
        return STATUS_SUCCESS;
    }

    // Present is only valid if the target is actively connected to this source
    if(!currentMode.Flags.FrameBufferIsActive)
    {
        LOG_DEBUG("Framebuffer is not active.\n");
        return STATUS_SUCCESS;
    }

    // If actual pixels are coming through, will need to completely zero out physical address next time in BlackOutScreen
    currentMode.ZeroedOutStart.QuadPart = 0;
    currentMode.ZeroedOutEnd.QuadPart = 0;

    D3DKMDT_VIDPN_PRESENT_PATH_ROTATION rotationNeededByFb = pPresentDisplayOnly->Flags.Rotate ? currentMode.Rotation : D3DKMDT_VPPR_IDENTITY;
    BYTE* dest = static_cast<BYTE*>(currentMode.FrameBufferPointer);
    UINT destBpp = BPPFromPixelFormat(currentMode.DisplayInfo.ColorFormat);

    if(currentMode.Scaling == D3DKMDT_VPPS_CENTERED)
    {
        UINT centerShift = (currentMode.DisplayInfo.Height - currentMode.SrcModeHeight) * currentMode.DisplayInfo.Pitch;
        centerShift += (currentMode.DisplayInfo.Width - currentMode.SrcModeWidth) * (destBpp / 8);
        dest += centerShift / 2;
    }

    BYTE* srcAddress = static_cast<BYTE*>(pPresentDisplayOnly->pSource);
    PMDL mdl;

    {
        LOG_DEBUG("Getting MDL. Framebuffer: 0x%I64X.\n", dest);

        // Map Source into kernel space, as Blt will be executed by system worker thread
        const UINT sizeToMap = pPresentDisplayOnly->BytesPerPixel * currentMode.SrcModeWidth * currentMode.SrcModeHeight;

        mdl = IoAllocateMdl(pPresentDisplayOnly->pSource, sizeToMap, FALSE, FALSE, nullptr);
        
        if(!mdl)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KPROCESSOR_MODE accessMode = static_cast<KPROCESSOR_MODE>(reinterpret_cast<UINT_PTR>(srcAddress) <= MM_USER_PROBE_ADDRESS ? UserMode : KernelMode);

        __try
        {
            // Probe and lock the pages of this buffer in physical memory.
            // We need only IoReadAccess.
            MmProbeAndLockPages(mdl, accessMode, IoReadAccess);
        }
        #pragma prefast(suppress: __WARNING_EXCEPTIONEXECUTEHANDLER, "try/except is only able to protect against user-mode errors and these are the only errors we try to catch here");
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            IoFreeMdl(mdl);
            return GetExceptionCode();
        }

        // Map the physical pages described by the MDL into system space.
        // Note: double mapping the buffer this way causes lot of system
        // overhead for large size buffers.
        srcAddress = reinterpret_cast<BYTE*>(MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority));

        if(!srcAddress)
        {
            MmUnlockPages(mdl);
            IoFreeMdl(mdl);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    m_PresentManager.InsertPresent(
        pPresentDisplayOnly,
        mdl,
        rotationNeededByFb,
        dest,
        srcAddress,
        destBpp,
        currentMode.DisplayInfo.Pitch,
        currentMode.SrcModeWidth,
        currentMode.SrcModeHeight
    );

    return STATUS_SUCCESS;
}
#endif

NTSTATUS HyMiniportDevice::StopDeviceAndReleasePostDisplayOwnership(IN_CONST_D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId, PDXGK_DISPLAY_INFORMATION DisplayInfo) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    if(m_CurrentDisplayMode[0].PowerState > PowerDeviceD0)
    {
        SetPowerState(TargetId, PowerDeviceD0, PowerActionNone);
    }

    *DisplayInfo = m_CurrentDisplayMode[0].DisplayInfo;

    return StopDevice();
}

NTSTATUS HyMiniportDevice::ControlInterrupt(IN_CONST_DXGK_INTERRUPT_TYPE InterruptType, IN_BOOLEAN EnableInterrupt) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    if(InterruptType == DXGK_INTERRUPT_CRTC_VSYNC
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
        || InterruptType == DXGK_INTERRUPT_DISPLAYONLY_VSYNC
#endif
    )
    {
        if(!EnableInterrupt)
        {
            LOG_DEBUG("VSync Disabled for Display 0.\n");
            m_CurrentDisplayMode[0].Flags.VSyncEnabled = 0;
        }
        else if(InterruptType == DXGK_INTERRUPT_CRTC_VSYNC)
        {
            LOG_DEBUG("CRTC VSync Enabled for Display 0.\n");
            m_CurrentDisplayMode[0].Flags.VSyncEnabled = 1;
        }
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
        else if(InterruptType == DXGK_INTERRUPT_DISPLAYONLY_VSYNC)
        {
            LOG_DEBUG("Display-Only VSync Enabled for Display 0.\n");
            m_CurrentDisplayMode[0].Flags.VSyncEnabled = 2;
        }
#endif

        LOG_DEBUG("%sabling VSync Interrupts for Display 0.\n", EnableInterrupt ? "En" : "Dis");

        volatile UINT* const displayVSyncEnable = GetDeviceConfigRegister(BASE_REGISTER_DI + SIZE_REGISTER_DI * 0 + OFFSET_REGISTER_DI_VSYNC_ENABLE);

        *displayVSyncEnable = EnableInterrupt ? 1 : 0;

        return STATUS_SUCCESS;
    }

    return STATUS_NOT_IMPLEMENTED;
}

#pragma code_seg(push)
#pragma code_seg("_KTEXT")
// This function cannot be paged.
// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/blthw.cxx#L21
// Thus it is subject to the Microsoft Public License.
BOOLEAN HyMiniportDevice::SynchronizeVidSchNotifyInterrupt(PVOID Params) noexcept
{
    GsSynchronizeParams* synchronizeParams = reinterpret_cast<GsSynchronizeParams*>(Params);

    const DXGKRNL_INTERFACE& DxgkInterface = synchronizeParams->Device->m_DxgkInterface;

    // Notify the specified interrupt.
    DxgkInterface.DxgkCbNotifyInterrupt(DxgkInterface.DeviceHandle, &synchronizeParams->InterruptData);

    // Now queue a DPC for this interrupt (to callback schedule at DCP level and let it do more work there)
    // DxgkCbQueueDpc can return FALSE if there is already a DPC queued
    // this is an acceptable condition
    DxgkInterface.DxgkCbQueueDpc(DxgkInterface.DeviceHandle);

    return TRUE;
}
#pragma code_seg(pop)

// Fakes an interrupt for present progress.
// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/blthw.cxx#L395
// Thus it is subject to the Microsoft Public License.
void HyMiniportDevice::ReportPresentProgress(D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId, BOOLEAN CompletedOrFailed) noexcept
{
    (void) VidPnSourceId;
    (void) CompletedOrFailed;

    PAGED_CODE();

    if constexpr(false)
    {
        TRACE_ENTRYPOINT();
    }

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
    GsSynchronizeParams synchronizeParams {};
    synchronizeParams.Device = this;
    synchronizeParams.InterruptData.InterruptType = DXGK_INTERRUPT_DISPLAYONLY_PRESENT_PROGRESS;
    synchronizeParams.InterruptData.DisplayOnlyPresentProgress.VidPnSourceId = VidPnSourceId;
    synchronizeParams.InterruptData.DisplayOnlyPresentProgress.ProgressId =
        CompletedOrFailed ?
        DXGK_PRESENT_DISPLAYONLY_PROGRESS_ID_COMPLETE :
        DXGK_PRESENT_DISPLAYONLY_PROGRESS_ID_FAILED;
    synchronizeParams.InterruptData.Flags.Value = 0;

    BOOLEAN ret = FALSE;
    const NTSTATUS status = m_DxgkInterface.DxgkCbSynchronizeExecution(
        m_DxgkInterface.DeviceHandle,     // DeviceHandle
        SynchronizeVidSchNotifyInterrupt, // SynchronizeRoutine
        &synchronizeParams,               // Context
        0,                                // MessageNumber
        &ret                              // ReturnValue
    );

    if(!ret)
    {
        LOG_WARN("Synchronization Routine returned false.\n");
    }

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to synchronize execution.\n");
    }
#endif
}

bool HyMiniportDevice::ObtainLogLock(const bool ReturnOnFailure) noexcept
{
    volatile UINT* logLockRegister = GetDeviceConfigRegister(REGISTER_DEBUG_LOG_LOCK);

    LONG lockValue = InterlockedIncrement(&m_CurrentLogLockValue);

    // If we've experienced a integer overflow, then we should re-increment.
    if(lockValue == VALUE_DEBUG_LOG_LOCK_UNLOCKED)
    {
        lockValue = InterlockedIncrement(&m_CurrentLogLockValue);

        // If re-incrementing failed, then we should just return.
        if(lockValue == VALUE_DEBUG_LOG_LOCK_UNLOCKED)
        {
            return false;
        }
    }

    do
    {
        *logLockRegister = static_cast<UINT>(lockValue);
        const UINT storedLockValue = *logLockRegister;

        if(storedLockValue == static_cast<UINT>(lockValue))
        {
            return true;
        }
    }
    while(!ReturnOnFailure);

    return false;
}

void HyMiniportDevice::ReleaseLogLock() noexcept
{
    *GetDeviceConfigRegister(REGISTER_DEBUG_LOG_LOCK) = VALUE_DEBUG_LOG_LOCK_UNLOCKED;
}

void HyMiniportDevice::DebugLog(const char* String, const SIZE_T Length, const bool Lock) noexcept
{
    if(Lock && !ObtainLogLock(false))
    {
        return;
    }

    volatile UINT* debugLogMultiRegister = GetDeviceConfigRegister(REGISTER_DEBUG_LOG_MULTI);

    for(SIZE_T i = 0; i < Length; ++i)
    {
        *debugLogMultiRegister = String[i];
    }

    if(Lock)
    {
        ReleaseLogLock();
    }
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L711
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::SetSourceModeAndPath(const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode, const D3DKMDT_VIDPN_PRESENT_PATH* pPath) noexcept
{
    HyDisplayMode* currentMode = &m_CurrentDisplayMode[pPath->VidPnSourceId];
    currentMode->Scaling = pPath->ContentTransformation.Scaling;
    currentMode->SrcModeWidth = pSourceMode->Format.Graphics.PrimSurfSize.cx;
    currentMode->SrcModeHeight = pSourceMode->Format.Graphics.PrimSurfSize.cy;
    currentMode->Rotation = pPath->ContentTransformation.Rotation;
    //currentMode->VidPnTargetId = pPath->VidPnTargetId;

    NTSTATUS status = STATUS_SUCCESS;

    if(!currentMode->Flags.DoNotMapOrUnmap)
    {
        status = MapFrameBuffer(
            currentMode->DisplayInfo.PhysicAddress, 
            currentMode->DisplayInfo.Pitch * currentMode->DisplayInfo.Height,
            &currentMode->FrameBufferPointer
        );
    }

    if(NT_SUCCESS(status))
    {
        currentMode->Flags.FrameBufferIsActive = true;
        SetDisplayState(pPath->VidPnSourceId, false);

        // Mark that the next present should be fullscreen so the screen doesn't go from black to actual pixels one dirty rect at a time.
        currentMode->Flags.FullscreenPresent = true;
    }

    return status;
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L748
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::AreVidPnPathFieldsValid(const D3DKMDT_VIDPN_PRESENT_PATH* pPath) const noexcept
{
    if(pPath->VidPnSourceId >= MaxViews)
    {
        LOG_ERROR("VidPnSourceId is 0x%I64x is too high (MAX_VIEWS is 0x%I64x)", pPath->VidPnSourceId, MaxViews);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE;
    }
    else if(pPath->VidPnTargetId >= MaxChildren)
    {
        LOG_ERROR("VidPnTargetId is 0x%I64x is too high (MAX_CHILDREN is 0x%I64x)", pPath->VidPnTargetId, MaxChildren);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_TARGET;
    }
    else if(pPath->GammaRamp.Type != D3DDDI_GAMMARAMP_DEFAULT)
    {
        LOG_ERROR("pPath contains a gamma ramp (0x%I64x)", pPath->GammaRamp.Type);
        return STATUS_GRAPHICS_GAMMA_RAMP_NOT_SUPPORTED;
    }
    else if(pPath->ContentTransformation.Scaling != D3DKMDT_VPPS_IDENTITY &&
            pPath->ContentTransformation.Scaling != D3DKMDT_VPPS_CENTERED &&
            pPath->ContentTransformation.Scaling != D3DKMDT_VPPS_NOTSPECIFIED &&
            pPath->ContentTransformation.Scaling != D3DKMDT_VPPS_UNINITIALIZED)
    {
        LOG_ERROR("pPath contains a non-identity scaling (0x%I64x)", pPath->ContentTransformation.Scaling);
        return STATUS_GRAPHICS_VIDPN_MODALITY_NOT_SUPPORTED;
    }
    else if(pPath->ContentTransformation.Rotation != D3DKMDT_VPPR_IDENTITY &&
            pPath->ContentTransformation.Rotation != D3DKMDT_VPPR_ROTATE90 &&
            pPath->ContentTransformation.Rotation != D3DKMDT_VPPR_NOTSPECIFIED &&
            pPath->ContentTransformation.Rotation != D3DKMDT_VPPR_UNINITIALIZED)
    {
        LOG_ERROR("pPath contains a not-supported rotation (0x%I64x)", pPath->ContentTransformation.Rotation);
        return STATUS_GRAPHICS_VIDPN_MODALITY_NOT_SUPPORTED;
    }
    else if(pPath->VidPnTargetColorBasis != D3DKMDT_CB_SCRGB && pPath->VidPnTargetColorBasis != D3DKMDT_CB_UNINITIALIZED)
    {
        LOG_ERROR("pPath has a non-linear RGB color basis (0x%I64x)", pPath->VidPnTargetColorBasis);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L797
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::AreVidPnSourceModeFieldsValid(const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode) const noexcept
{
    if(pSourceMode->Type != D3DKMDT_RMT_GRAPHICS)
    {
        LOG_ERROR("pSourceMode is a non-graphics mode (0x%I64x)", pSourceMode->Type);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }
    else if(pSourceMode->Format.Graphics.ColorBasis != D3DKMDT_CB_SCRGB && pSourceMode->Format.Graphics.ColorBasis != D3DKMDT_CB_UNINITIALIZED)
    {
        LOG_ERROR("pSourceMode has a non-linear RGB color basis (0x%I64x)", pSourceMode->Format.Graphics.ColorBasis);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }
    else if(pSourceMode->Format.Graphics.PixelValueAccessMode != D3DKMDT_PVAM_DIRECT)
    {
        LOG_ERROR("pSourceMode has a palettized access mode (0x%I64x)", pSourceMode->Format.Graphics.PixelValueAccessMode);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }
    else
    {
        for(UINT pelFmtIdx = 0; pelFmtIdx < sizeof(gPixelFormats) / sizeof(gPixelFormats[0]); ++pelFmtIdx)
        {
            if(pSourceMode->Format.Graphics.PixelFormat == gPixelFormats[pelFmtIdx])
            {
                return STATUS_SUCCESS;
            }
        }

        LOG_ERROR("pSourceMode has an unknown pixel format (0x%I64x)", pSourceMode->Format.Graphics.PixelFormat);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }
}

struct EdidTiming final
{
    UINT16 Width;
    UINT16 Height;
    UINT16 RefreshRate;
};

EdidTiming mVbeEstablishedEdidTiming[] =
{
  //
  // Established Timing I
  //
  {800, 600, 60},
  {800, 600, 56},
  {640, 480, 75},
  {640, 480, 72},
  {640, 480, 67},
  {640, 480, 60},
  {720, 400, 88},
  {720, 400, 70},
  //
  // Established Timing II
  //
  {1280, 1024, 75},
  {1024,  768, 75},
  {1024,  768, 70},
  {1024,  768, 60},
  {1024,  768, 87},
  {832,   624, 75},
  {800,   600, 75},
  {800,   600, 72},
  //
  // Established Timing III
  //
  {1152, 870, 75}
};

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L845
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::AddSingleSourceMode(const DXGK_VIDPNSOURCEMODESET_INTERFACE* const pVidPnSourceModeSetInterface, D3DKMDT_HVIDPNSOURCEMODESET hVidPnSourceModeSet, D3DDDI_VIDEO_PRESENT_SOURCE_ID SourceId) noexcept
{
    // There is only one source format supported by display-only drivers, but more can be added in a 
    // full WDDM driver if the hardware supports them
    for(UINT pelFmtIndex = 0; pelFmtIndex < sizeof(gPixelFormats) / sizeof(gPixelFormats[0]); ++pelFmtIndex)
    {
        D3DKMDT_VIDPN_SOURCE_MODE* pVidPnSourceModeInfo;
        NTSTATUS status = pVidPnSourceModeSetInterface->pfnCreateNewModeInfo(hVidPnSourceModeSet, &pVidPnSourceModeInfo);

        if(!NT_SUCCESS(status))
        {
            LOG_ERROR("Failed to create new Mode Info: 0x%08X, hVidPnSourceModeSet: 0x%I64X\n", status, hVidPnSourceModeSet);
            return status;
        }

        // Populate mode info with values from current mode and hard-coded values
        // Always report 32 bpp format, this will be color converted during the present if the mode is < 32bpp
        pVidPnSourceModeInfo->Type = D3DKMDT_RMT_GRAPHICS;
        pVidPnSourceModeInfo->Format.Graphics.PrimSurfSize.cx = m_CurrentDisplayMode[SourceId].DisplayInfo.Width;
        pVidPnSourceModeInfo->Format.Graphics.PrimSurfSize.cy = m_CurrentDisplayMode[SourceId].DisplayInfo.Height;
        pVidPnSourceModeInfo->Format.Graphics.VisibleRegionSize = pVidPnSourceModeInfo->Format.Graphics.PrimSurfSize;
        pVidPnSourceModeInfo->Format.Graphics.Stride = m_CurrentDisplayMode[SourceId].DisplayInfo.Pitch;
        pVidPnSourceModeInfo->Format.Graphics.PixelFormat = gPixelFormats[pelFmtIndex];
        pVidPnSourceModeInfo->Format.Graphics.ColorBasis = D3DKMDT_CB_SRGB;
        pVidPnSourceModeInfo->Format.Graphics.PixelValueAccessMode = D3DKMDT_PVAM_DIRECT;

        // Add the mode to the source mode set
        status = pVidPnSourceModeSetInterface->pfnAddMode(hVidPnSourceModeSet, pVidPnSourceModeInfo);

        if(!NT_SUCCESS(status))
        {
            // If adding the mode failed, release the mode, if this doesn't work there is nothing that can be done, some memory will get leaked
            (void) pVidPnSourceModeSetInterface->pfnReleaseModeInfo(hVidPnSourceModeSet, pVidPnSourceModeInfo);

            if(status != STATUS_GRAPHICS_MODE_ALREADY_IN_MODESET)
            {
                LOG_ERROR("Failed to add Mode Info: 0x%08X, hVidPnSourceModeSet: 0x%I64X, pVidPnSourceModeInfo: 0x%I64X\n", status, hVidPnSourceModeSet, pVidPnSourceModeInfo);
                return status;
            }
        }
    }

    const UINT maxWidth = m_CurrentDisplayMode[SourceId].DisplayInfo.Width;
    const UINT maxHeight = m_CurrentDisplayMode[SourceId].DisplayInfo.Height;

    // Add all predefined modes that fit within the bounds of the required (POST) mode
    for(UINT modeIndex = 0; modeIndex < sizeof(mVbeEstablishedEdidTiming) / sizeof(mVbeEstablishedEdidTiming[0]); ++modeIndex)
    {
        if(mVbeEstablishedEdidTiming[modeIndex].Width > maxWidth)
        {
            continue;
        }
        else if(mVbeEstablishedEdidTiming[modeIndex].Height > maxHeight)
        {
            continue;
        }

        // There is only one source format supported by display-only drivers, but more can be added in a 
        // full WDDM driver if the hardware supports them
        for(UINT pelFmtIndex = 0; pelFmtIndex < sizeof(gPixelFormats) / sizeof(gPixelFormats[0]); ++pelFmtIndex)
        {
            D3DKMDT_VIDPN_SOURCE_MODE* pVidPnSourceModeInfo;
            NTSTATUS status = pVidPnSourceModeSetInterface->pfnCreateNewModeInfo(hVidPnSourceModeSet, &pVidPnSourceModeInfo);

            if(!NT_SUCCESS(status))
            {
                LOG_ERROR("Failed to create new Mode Info: 0x%08X, hVidPnSourceModeSet: 0x%I64X\n", status, hVidPnSourceModeSet);
                return status;
            }

            // Populate mode info with values from current mode and hard-coded values
            // Always report 32 bpp format, this will be color converted during the present if the mode is < 32bpp
            pVidPnSourceModeInfo->Type = D3DKMDT_RMT_GRAPHICS;
            pVidPnSourceModeInfo->Format.Graphics.PrimSurfSize.cx = mVbeEstablishedEdidTiming[modeIndex].Width;
            pVidPnSourceModeInfo->Format.Graphics.PrimSurfSize.cy = mVbeEstablishedEdidTiming[modeIndex].Height;
            pVidPnSourceModeInfo->Format.Graphics.VisibleRegionSize = pVidPnSourceModeInfo->Format.Graphics.PrimSurfSize;
            pVidPnSourceModeInfo->Format.Graphics.Stride = 4 * mVbeEstablishedEdidTiming[modeIndex].Width;
            pVidPnSourceModeInfo->Format.Graphics.PixelFormat = gPixelFormats[pelFmtIndex];
            pVidPnSourceModeInfo->Format.Graphics.ColorBasis = D3DKMDT_CB_SRGB;
            pVidPnSourceModeInfo->Format.Graphics.PixelValueAccessMode = D3DKMDT_PVAM_DIRECT;

            // Add the mode to the source mode set
            status = pVidPnSourceModeSetInterface->pfnAddMode(hVidPnSourceModeSet, pVidPnSourceModeInfo);

            if(!NT_SUCCESS(status))
            {
                // If adding the mode failed, release the mode, if this doesn't work there is nothing that can be done, some memory will get leaked, continue to next mode anyway
                (void) pVidPnSourceModeSetInterface->pfnReleaseModeInfo(hVidPnSourceModeSet, pVidPnSourceModeInfo);

                if(status != STATUS_GRAPHICS_MODE_ALREADY_IN_MODESET)
                {
                    LOG_ERROR("Failed to add Mode Info: 0x%08X, hVidPnSourceModeSet: 0x%I64X, pVidPnSourceModeInfo: 0x%I64X\n", status, hVidPnSourceModeSet, pVidPnSourceModeInfo);
                }
            }
        }
    }

    return STATUS_SUCCESS;
}

// This implementation is largely sourced from https://github.com/microsoft/Windows-driver-samples/blob/main/video/KMDOD/bdd_dmm.cxx#L966
// Thus it is subject to the Microsoft Public License.
NTSTATUS HyMiniportDevice::AddSingleTargetMode(const DXGK_VIDPNTARGETMODESET_INTERFACE* const pVidPnTargetModeSetInterface, D3DKMDT_HVIDPNTARGETMODESET hVidPnTargetModeSet, const D3DKMDT_VIDPN_SOURCE_MODE* pVidPnPinnedSourceModeInfo, D3DDDI_VIDEO_PRESENT_SOURCE_ID SourceId) noexcept
{
    (void) pVidPnPinnedSourceModeInfo;

    D3DKMDT_VIDPN_TARGET_MODE* pVidPnTargetModeInfo;
    NTSTATUS status = pVidPnTargetModeSetInterface->pfnCreateNewModeInfo(hVidPnTargetModeSet, &pVidPnTargetModeInfo);

    if(!NT_SUCCESS(status))
    {
        LOG_ERROR("Failed to create new Mode Info: 0x%08X, hVidPnTargetModeSet: 0x%I64X\n", status, hVidPnTargetModeSet);
        return status;
    }

    pVidPnTargetModeInfo->VideoSignalInfo.VideoStandard = D3DKMDT_VSS_VESA_GTF;
    pVidPnTargetModeInfo->VideoSignalInfo.TotalSize.cx = m_CurrentDisplayMode[SourceId].DisplayInfo.Width;
    pVidPnTargetModeInfo->VideoSignalInfo.TotalSize.cy = m_CurrentDisplayMode[SourceId].DisplayInfo.Height;
    pVidPnTargetModeInfo->VideoSignalInfo.ActiveSize = pVidPnTargetModeInfo->VideoSignalInfo.TotalSize;
    pVidPnTargetModeInfo->VideoSignalInfo.TotalSize.cx = 1344;
    pVidPnTargetModeInfo->VideoSignalInfo.TotalSize.cy = 795;
#if HY_KMDOD_ENABLE_VSYNC_INTERRUPTS
    pVidPnTargetModeInfo->VideoSignalInfo.VSyncFreq.Numerator = 60;
    pVidPnTargetModeInfo->VideoSignalInfo.VSyncFreq.Denominator = 1;
    pVidPnTargetModeInfo->VideoSignalInfo.HSyncFreq.Numerator = 476999999;
    pVidPnTargetModeInfo->VideoSignalInfo.HSyncFreq.Denominator = 10000;
    pVidPnTargetModeInfo->VideoSignalInfo.PixelRate = 64108800;
#else
    pVidPnTargetModeInfo->VideoSignalInfo.VSyncFreq.Numerator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    pVidPnTargetModeInfo->VideoSignalInfo.VSyncFreq.Denominator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    pVidPnTargetModeInfo->VideoSignalInfo.HSyncFreq.Numerator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    pVidPnTargetModeInfo->VideoSignalInfo.HSyncFreq.Denominator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    pVidPnTargetModeInfo->VideoSignalInfo.PixelRate = D3DKMDT_FREQUENCY_NOTSPECIFIED;
#endif
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM1_3_M1)
    pVidPnTargetModeInfo->VideoSignalInfo.AdditionalSignalInfo.ScanLineOrdering = D3DDDI_VSSLO_PROGRESSIVE;
    pVidPnTargetModeInfo->VideoSignalInfo.AdditionalSignalInfo.VSyncFreqDivider = 0;
    pVidPnTargetModeInfo->VideoSignalInfo.AdditionalSignalInfo.Reserved = 0;
#else
    pVidPnTargetModeInfo->VideoSignalInfo.ScanLineOrdering = D3DDDI_VSSLO_PROGRESSIVE;
#endif // DXGKDDI_INTERFACE_VERSION_WDDM1_3_M1
    // We add this as PREFERRED since it is the only supported target
    pVidPnTargetModeInfo->Preference = D3DKMDT_MP_PREFERRED;

    status = pVidPnTargetModeSetInterface->pfnAddMode(hVidPnTargetModeSet, pVidPnTargetModeInfo);

    if(!NT_SUCCESS(status))
    {
        if(status != STATUS_GRAPHICS_MODE_ALREADY_IN_MODESET)
        {
            LOG_ERROR("Failed to add new Mode Info: 0x%08X, hVidPnTargetModeSet: 0x%I64X, pVidPnTargetModeInfo = 0x%I64X\n", status, hVidPnTargetModeSet, pVidPnTargetModeInfo);
        }
        else
        {
            status = STATUS_SUCCESS;
        }

        // If adding the mode failed, release the mode, if this doesn't work there is nothing that can be done, some memory will get leaked
        (void) pVidPnTargetModeSetInterface->pfnReleaseModeInfo(hVidPnTargetModeSet, pVidPnTargetModeInfo);

        return status;
    }

    // If AddMode succeeded with something other than STATUS_SUCCESS treat it as such anyway when propagating up
    return STATUS_SUCCESS;
}

static NTSTATUS MapFrameBuffer(PHYSICAL_ADDRESS PhysicalAddress, ULONG Length, void** VirtualAddress) noexcept
{
    TRACE_ENTRYPOINT_ARG("Address: 0x%I64X, Length: 0x%X\n", PhysicalAddress.QuadPart, Length);

    if(PhysicalAddress.QuadPart == 0)
    {
        LOG_ERROR("Invalid Parameter to MapFrameBuffer: PhysicalAddress\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    if(Length == 0)
    {
        LOG_ERROR("Invalid Parameter to MapFrameBuffer: Length\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    if(!VirtualAddress)
    {
        LOG_ERROR("Invalid Parameter to MapFrameBuffer: VirtualAddress\n");
        return STATUS_INVALID_PARAMETER_3;
    }

#if (NTDDI_VERSION >= NTDDI_WINTHRESHOLD)
    *VirtualAddress = MmMapIoSpaceEx(PhysicalAddress, Length, PAGE_READWRITE | PAGE_WRITECOMBINE);
#else
    *VirtualAddress = MmMapIoSpace(PhysicalAddress, Length, MmWriteCombined);
#endif

    if(*VirtualAddress == nullptr)
    {
        // The underlying call to MmMapIoSpace failed. This may be because, MmWriteCombined
        // isn't supported, so try again with MmNonCached

#if (NTDDI_VERSION >= NTDDI_WINTHRESHOLD)
        * VirtualAddress = MmMapIoSpaceEx(PhysicalAddress, Length, PAGE_READWRITE | PAGE_NOCACHE);
#else
        * VirtualAddress = MmMapIoSpace(PhysicalAddress, Length, MmNonCached);
#endif

        if(*VirtualAddress == nullptr)
        {
            LOG_ERROR("MmMapIoSpace failed to allocate a bufffer of 0x%I64X bytes.\n", Length);
            return STATUS_NO_MEMORY;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS HyMiniportDevice::UnmapFrameBuffer(void* const VirtualAddress, const ULONG Length) noexcept
{
    TRACE_ENTRYPOINT();

    if(!VirtualAddress && Length == 0)
    {
        // Allow this function to be called when there's no work to do, and treat as successful
        return STATUS_SUCCESS;
    }

    if(!VirtualAddress)
    {
        LOG_ERROR("Invalid Parameter: VirtualAddress\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    if(Length == 0)
    {
        LOG_ERROR("Invalid Parameter: Length\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    m_PresentManager.FlushPipeline();

    MmUnmapIoSpace(VirtualAddress, Length);

    return STATUS_SUCCESS;
}

#pragma code_seg(pop)
