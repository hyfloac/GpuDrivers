#include <vulkan/vulkan.h>
#include <new>
#include <cstring>
#include <Utils.hpp>
#include <ConPrinter.hpp>
#include <DynArray.hpp>

#include "WindowsNtPolyfill.hpp"
#include "PhysicalDeviceManager.hpp"
#include "InstanceManager.hpp"
#include "ConfigMacros.hpp"
#include "DriverAlignment.hpp"
#include "GdiThunks.hpp"
#include "GdiPhysicalDeviceIds.hpp"

namespace vk {

static VkResult GetGDIAdapters(ULONG adapterCount, D3DKMT_ADAPTERINFO* adapters) noexcept;
static VkResult CheckGDIAdapters(ULONG adapterCount, const D3DKMT_ADAPTERINFO* buffer, uint32_t* pPhysicalDeviceCount) noexcept;
static VkResult CheckGDIAdapter(const D3DKMT_ADAPTERINFO* adapterInfo) noexcept;
static VkResult StoreGDIAdapters(ULONG adapterCount, const D3DKMT_ADAPTERINFO* adapters, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices, VkInstance instance) noexcept;
static VkResult GetAdapterGUID(const D3DKMT_ADAPTERINFO* adapterInfo, GUID* pAdapterGuid) noexcept;

static void FillVkPhysicalDeviceLimits(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits* pLimits) noexcept;
static void FillVkPhysicalDeviceDepthStencilResolveProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceDepthStencilResolveProperties* pProperties) noexcept;
static void FillVkPhysicalDeviceDescriptorIndexingProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceDescriptorIndexingProperties* pProperties) noexcept;

static void FillVkPhysicalDeviceVulkan11Properties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceVulkan11Properties* pProperties) noexcept;

static void FillVkPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) noexcept;

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumeratePhysicalDevices(const VkInstance instance, uint32_t* const pPhysicalDeviceCount, VkPhysicalDevice* const pPhysicalDevices) noexcept
{
#if DRIVER_DEBUG_LOG
    ConPrinter::Print(u"Enumerating {} Physical Devices info.\n", *pPhysicalDeviceCount);
#endif

    if(!pPhysicalDevices)
    {
        // If D3DKMTEnumAdapters2 is supported we'll use that, otherwise we can only query up to 16 devices with D3DKMTEnumAdapters.
        // D3DKMTEnumAdapters2 is always returning 32 for me (at least when on RDP), whereas D3DKMTEnumAdapters correctly returns 3. Until I can figure out exactly what is wrong I'll just use the lessor function.
        if(GDIEnumAdapters2 && false)
        // ReSharper disable once CppUnreachableCode
        {
            // ReSharper disable once CppInitializedValueIsAlwaysRewritten
            D3DKMT_ENUMADAPTERS2 enumAdaptersArgs {};
            enumAdaptersArgs.NumAdapters = 0;
            // We're only querying how many devices there are.
            enumAdaptersArgs.pAdapters = nullptr;

            // Get the number of adapters in the system.
            const NTSTATUS enumAdaptersRet = GDIEnumAdapters2(&enumAdaptersArgs);

            // If GDI returns no memory, inform the application we're out of host memory.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters2 returned STATUS_INVALID_PARAMETER while querying how many adapters there are.\u001B[0m\n");
                return VK_ERROR_UNKNOWN;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet != static_cast<NTSTATUS>(STATUS_BUFFER_TOO_SMALL) && enumAdaptersRet != static_cast<NTSTATUS>(STATUS_SUCCESS))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters2 returned unknown error 0x{X} while querying how many adapters there are.\u001B[0m\n", enumAdaptersRet);
                return VK_ERROR_UNKNOWN;
            }

            if(enumAdaptersArgs.NumAdapters <= 16)
            {
                D3DKMT_ADAPTERINFO adapters[16];

                // Get all of the system adapters.
                const VkResult getAdaptersResult = GetGDIAdapters(enumAdaptersArgs.NumAdapters, adapters);
                
                if(getAdaptersResult != VK_SUCCESS)
                {
                    // Propagate any errors.
                    return getAdaptersResult;
                }

                // Check how many adapters are ours.
                const VkResult checkAdaptersResult = CheckGDIAdapters(enumAdaptersArgs.NumAdapters, adapters, pPhysicalDeviceCount);

                if(checkAdaptersResult != VK_SUCCESS)
                {
                    // Propagate any errors.
                    return checkAdaptersResult;
                }
            }
            else
            {
                D3DKMT_ADAPTERINFO* const adapters = new(::std::nothrow) D3DKMT_ADAPTERINFO[enumAdaptersArgs.NumAdapters];

                // If new[] returns null, inform the application we're out of host memory.
                if(!adapters)
                {
                    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
                }

                // Get all of the system adapters.
                const VkResult getAdaptersResult = GetGDIAdapters(enumAdaptersArgs.NumAdapters, adapters);

                if(getAdaptersResult != VK_SUCCESS)
                {
                    // Delete the adapter array if the was an error.
                    delete[] adapters;
                    // Propagate any errors.
                    return getAdaptersResult;
                }

                // Check how many adapters are ours.
                const VkResult checkAdaptersResult = CheckGDIAdapters(enumAdaptersArgs.NumAdapters, adapters, pPhysicalDeviceCount);

                // The adapter array is no longer needed.
                delete[] adapters;

                if(checkAdaptersResult != VK_SUCCESS)
                {
                    // Propagate any errors.
                    return checkAdaptersResult;
                }
            }
        }
        else
        {
            D3DKMT_ENUMADAPTERS enumAdaptersArgs {};
            enumAdaptersArgs.NumAdapters = 0;

            // Get all of the system adapters.
            const NTSTATUS enumAdaptersRet = GDIEnumAdapters(&enumAdaptersArgs);

            // If GDI returns no memory, inform the application we're out of host memory.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters returned STATUS_INVALID_PARAMETER while querying adapters.\u001B[0m\n");
                return VK_ERROR_UNKNOWN;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet != static_cast<NTSTATUS>(STATUS_BUFFER_TOO_SMALL) && enumAdaptersRet != static_cast<NTSTATUS>(STATUS_SUCCESS))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters returned unknown error 0x{X} while querying adapters.\u001B[0m\n", enumAdaptersRet);
                return VK_ERROR_UNKNOWN;
            }

            // Check how many adapters are ours.
            const VkResult checkAdaptersResult = CheckGDIAdapters(enumAdaptersArgs.NumAdapters, enumAdaptersArgs.Adapters, pPhysicalDeviceCount);

            if(checkAdaptersResult != VK_SUCCESS)
            {
                // Propagate any errors.
                return checkAdaptersResult;
            }
        }

        return VK_SUCCESS;
    }
    else
    {
        // If D3DKMTEnumAdapters2 is supported we'll use that, otherwise we can only query up to 16 devices with D3DKMTEnumAdapters.
        // D3DKMTEnumAdapters2 is always returning 32 for me (at least when on RDP), whereas D3DKMTEnumAdapters correctly returns 3. Until I can figure out exactly what is wrong I'll just use the lessor function.
        if(GDIEnumAdapters2 && false)
        // ReSharper disable once CppUnreachableCode
        {
            // ReSharper disable once CppInitializedValueIsAlwaysRewritten
            D3DKMT_ENUMADAPTERS2 enumAdaptersArgs {};
            enumAdaptersArgs.NumAdapters = 0;
            // We're only querying how many devices there are.
            enumAdaptersArgs.pAdapters = nullptr;

            // Get the number of adapters in the system.
            const NTSTATUS enumAdaptersRet = GDIEnumAdapters2(&enumAdaptersArgs);

            // If GDI returns no memory, inform the application we're out of host memory.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters2 returned STATUS_INVALID_PARAMETER while querying how many adapters there are.\u001B[0m\n");
                return VK_ERROR_UNKNOWN;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet != static_cast<NTSTATUS>(STATUS_BUFFER_TOO_SMALL) && enumAdaptersRet != static_cast<NTSTATUS>(STATUS_SUCCESS))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters2 returned unknown error 0x{X} while querying how many adapters there are.\u001B[0m\n", enumAdaptersRet);
                return VK_ERROR_UNKNOWN;
            }

            if(enumAdaptersArgs.NumAdapters <= 16)
            {
                D3DKMT_ADAPTERINFO adapters[16];

                // Get all of the system adapters.
                const VkResult getAdaptersResult = GetGDIAdapters(enumAdaptersArgs.NumAdapters, adapters);

                if(getAdaptersResult != VK_SUCCESS)
                {
                    // Propagate any errors.
                    return getAdaptersResult;
                }

                // Store our vulkan adapters.
                const VkResult storeAdaptersResult = StoreGDIAdapters(enumAdaptersArgs.NumAdapters, adapters, pPhysicalDeviceCount, pPhysicalDevices, instance);

                if(storeAdaptersResult != VK_SUCCESS)
                {
                    // Propagate any errors.
                    return storeAdaptersResult;
                }
            }
            else
            {
                D3DKMT_ADAPTERINFO* const adapters = new(::std::nothrow) D3DKMT_ADAPTERINFO[enumAdaptersArgs.NumAdapters];

                // If new[] returns null, inform the application we're out of host memory.
                if(!adapters)
                {
                    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
                }

                // Get all of the system adapters.
                const VkResult getAdaptersResult = GetGDIAdapters(enumAdaptersArgs.NumAdapters, adapters);

                if(getAdaptersResult != VK_SUCCESS)
                {
                    // Delete the adapter array if the was an error.
                    delete[] adapters;
                    // Propagate any errors.
                    return getAdaptersResult;
                }

                // Store our vulkan adapters.
                const VkResult storeAdaptersResult = StoreGDIAdapters(enumAdaptersArgs.NumAdapters, adapters, pPhysicalDeviceCount, pPhysicalDevices, instance);

                // The adapter array is no longer needed.
                delete[] adapters;

                if(storeAdaptersResult != VK_SUCCESS)
                {
                    // Propagate any errors.
                    return storeAdaptersResult;
                }
            }
        }
        else
        {
            D3DKMT_ENUMADAPTERS enumAdaptersArgs {};
            enumAdaptersArgs.NumAdapters = 0;

            // Get all of the system adapters.
            const NTSTATUS enumAdaptersRet = GDIEnumAdapters(&enumAdaptersArgs);

            // If GDI returns no memory, inform the application we're out of host memory.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters returned STATUS_INVALID_PARAMETER while querying adapters.\u001B[0m\n");
                return VK_ERROR_UNKNOWN;
            }

            // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
            if(enumAdaptersRet != static_cast<NTSTATUS>(STATUS_BUFFER_TOO_SMALL) && enumAdaptersRet != static_cast<NTSTATUS>(STATUS_SUCCESS))
            {
                ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters returned unknown error 0x{X} while querying adapters.\u001B[0m\n", enumAdaptersRet);
                return VK_ERROR_UNKNOWN;
            }

            // Store our vulkan adapters.
            const VkResult storeAdaptersResult = StoreGDIAdapters(enumAdaptersArgs.NumAdapters, enumAdaptersArgs.Adapters, pPhysicalDeviceCount, pPhysicalDevices, instance);

            if(storeAdaptersResult != VK_SUCCESS)
            {
                // Propagate any errors.
                return storeAdaptersResult;
            }
        }

        return VK_SUCCESS;
    }
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* const pProperties) noexcept
{
    UNUSED(physicalDevice);

    // We're just going to say we support Vulkan 1.3 for now.
    pProperties->apiVersion = VK_API_VERSION_1_3;
    // This doesn't need to be a Vulkan version number, but it's an appropriate macro for creating version integers.
    pProperties->driverVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    // We don't have a vendor ID, so we'll just use a generic 0.
    pProperties->vendorID = 0;
    // We'll just start device ID's from 1 for now.
    pProperties->deviceID = 1;
    // There is no plans to fab this GPU, so we're some weird 'other' type.
    pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    // Store the name "Serafin 1" into the device name.
    const errno_t nameCopyError = ::strncpy_s(pProperties->deviceName, sizeof(pProperties->deviceName), "Serafin 1", sizeof(pProperties->deviceName));

#if DRIVER_DEBUG_LOG
    if(nameCopyError)
    {
        ConPrinter::Print(u"Error storing device name into VkPhysicalDeviceProperties.\n");
    }
#else
    // In release mode the name will just be empty.
    (void) nameCopyError;
#endif

    // Set a null UUID for the unique device ID.
    (void) ::std::memset(&pProperties->pipelineCacheUUID, 0, sizeof(pProperties->pipelineCacheUUID));

    FillVkPhysicalDeviceLimits(physicalDevice, &pProperties->limits);
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties2(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* const pProperties) noexcept
{
    DriverVkGetPhysicalDeviceProperties(physicalDevice, &pProperties->properties);

    VkBaseOutStructure* pNext = reinterpret_cast<VkBaseOutStructure*>(pProperties->pNext);

    while(pNext)
    {
        switch(pNext->sType)
        {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES: FillVkPhysicalDeviceDepthStencilResolveProperties(physicalDevice, reinterpret_cast<VkPhysicalDeviceDepthStencilResolveProperties*>(pNext)); break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES: FillVkPhysicalDeviceDescriptorIndexingProperties(physicalDevice, reinterpret_cast<VkPhysicalDeviceDescriptorIndexingProperties*>(pNext)); break;

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES: FillVkPhysicalDeviceVulkan11Properties(physicalDevice, reinterpret_cast<VkPhysicalDeviceVulkan11Properties*>(pNext)); break;
            default: break;
        }

        pNext = pNext->pNext;
    }
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFeatures(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* const pFeatures) noexcept
{
    FillVkPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFeatures2(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* const pFeatures) noexcept
{
    FillVkPhysicalDeviceFeatures(physicalDevice, &pFeatures->features);
}

static VkResult GetGDIAdapters(const ULONG adapterCount, D3DKMT_ADAPTERINFO* const adapters) noexcept
{
    // ReSharper disable once CppInitializedValueIsAlwaysRewritten
    D3DKMT_ENUMADAPTERS2 enumAdaptersArgs {};
    enumAdaptersArgs.NumAdapters = adapterCount;
    enumAdaptersArgs.pAdapters = adapters;

    // Query the actual adapter list.
    const NTSTATUS enumAdaptersRet = GDIEnumAdapters2(&enumAdaptersArgs);

    // If GDI returns no memory, inform the application we're out of host memory.
    if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
    if(enumAdaptersRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
    {
        ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters2 returned STATUS_INVALID_PARAMETER while querying adapters.\u001B[0m\n");
        return VK_ERROR_UNKNOWN;
    }

    // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
    if(enumAdaptersRet != static_cast<NTSTATUS>(STATUS_SUCCESS) && enumAdaptersRet != static_cast<NTSTATUS>(STATUS_BUFFER_TOO_SMALL))
    {
        ConPrinter::Print(u"\u001B[91mERROR: D3DKMTEnumAdapters2 returned unknown error 0x{X} while querying adapters.\u001B[0m\n", enumAdaptersRet);
        return VK_ERROR_UNKNOWN;
    }

    return VK_SUCCESS;
}

static VkResult CheckGDIAdapters(const ULONG adapterCount, const D3DKMT_ADAPTERINFO* const adapters, uint32_t* const pPhysicalDeviceCount) noexcept
{
    u32 ourAdapterCount = 0;

#if DRIVER_DUMMY_DEVICE
    ++ourAdapterCount;
#endif

    for(uSys i = 0; i < adapterCount; ++i)
    {
        const VkResult checkAdapterResult = CheckGDIAdapter(&adapters[i]);

        // Propagate Out of Memory and Unknown errors.
        if(checkAdapterResult == VK_ERROR_OUT_OF_HOST_MEMORY || checkAdapterResult == VK_ERROR_UNKNOWN)
        {
            return checkAdapterResult;
        }

        // If the adapter was ours, increment the count.
        if(checkAdapterResult == VK_SUCCESS)
        {
            ++ourAdapterCount;
        }

        D3DKMT_CLOSEADAPTER closeAdapterArgs {};
        closeAdapterArgs.hAdapter = adapters[i].hAdapter;

        const NTSTATUS closeAdapterRet = GDICloseAdapter(&closeAdapterArgs);

        // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
        if(closeAdapterRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
        {
            ConPrinter::Print(u"\u001B[91mERROR: D3DKMTCloseAdapter returned STATUS_INVALID_PARAMETER while querying adapters.\u001B[0m\n");
            return VK_ERROR_UNKNOWN;
        }

        // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
        if(closeAdapterRet != static_cast<NTSTATUS>(STATUS_SUCCESS))
        {
            ConPrinter::Print(u"\u001B[91mERROR: D3DKMTCloseAdapter returned unknown error 0x{X} while querying adapters.\u001B[0m\n", closeAdapterRet);
            return VK_ERROR_UNKNOWN;
        }
    }

    *pPhysicalDeviceCount = ourAdapterCount;

    return VK_SUCCESS;
}

static VkResult CheckGDIAdapter(const D3DKMT_ADAPTERINFO* const adapterInfo) noexcept
{
    PhysicalDeviceIds physicalDeviceIds {};

    // ReSharper disable once CppInitializedValueIsAlwaysRewritten
    D3DKMT_QUERYADAPTERINFO queryArgs {};
    queryArgs.hAdapter = adapterInfo->hAdapter;

    // Query the adapter PCI IDs.
    queryArgs.Type = KMTQAITYPE_PHYSICALADAPTERDEVICEIDS;
    // Load the proprietary data structure for storage.
    queryArgs.pPrivateDriverData = &physicalDeviceIds;
    queryArgs.PrivateDriverDataSize = sizeof(physicalDeviceIds);

    const NTSTATUS queryStatus = GDIQueryAdapterInfo(&queryArgs);

    // If GDI returns no memory, inform the application we're out of host memory.
    if(queryStatus == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // If GDI returns that the device got removed, inform the application that the device was lost.
    if(queryStatus == static_cast<NTSTATUS>(STATUS_DEVICE_REMOVED))
    {
        return VK_ERROR_DEVICE_LOST;
    }

    // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
    if(queryStatus == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
    {
        ConPrinter::Print(u"\u001B[91mERROR: D3DQueryAdapterInfo returned STATUS_INVALID_PARAMETER while querying Physical Device ID's.\u001B[0m\n");
        return VK_ERROR_UNKNOWN;
    }

    // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
    if(queryStatus != static_cast<NTSTATUS>(STATUS_SUCCESS))
    {
        ConPrinter::Print(u"\u001B[91mERROR: D3DQueryAdapterInfo returned unknown status code 0x{X} while querying Physical Device ID's.\u001B[0m\n", queryStatus);
        return VK_ERROR_UNKNOWN;
    }

#if DRIVER_DEBUG_LOG
    ConPrinter::Print(u"Adapter Info:\n");
    ConPrinter::Print(u"  Field0:            0x{X}\n", physicalDeviceIds.field0);
    ConPrinter::Print(u"  VendorID:          0x{X}\n", physicalDeviceIds.VendorID);
    ConPrinter::Print(u"  DeviceID:          0x{X}\n", physicalDeviceIds.DeviceID);
    ConPrinter::Print(u"  SubsystemVendorID: 0x{X}\n", physicalDeviceIds.SubsystemVendorID);
    ConPrinter::Print(u"  SubsystemID:       0x{X}\n", physicalDeviceIds.SubsystemID);
    ConPrinter::Print(u"  RevisionNumber:    0x{X}\n", physicalDeviceIds.RevisionNumber);
    ConPrinter::Print(u"  Field6:            0x{X}\n", physicalDeviceIds.field6);
#endif

    // This adapter is not ours. Return VK_TIMEOUT as a signifier that every other check completed successfully.
    if(physicalDeviceIds.VendorID != 0xFFFE)
    {
        return VK_TIMEOUT;
    }

    return VK_SUCCESS;
}

static VkResult StoreGDIAdapters(const ULONG adapterCount, const D3DKMT_ADAPTERINFO* const adapters, uint32_t* const pPhysicalDeviceCount, VkPhysicalDevice* const pPhysicalDevices, const VkInstance instance) noexcept
{
    const u32 maxAdapters = *pPhysicalDeviceCount;

    u32 ourAdapterCount = 0;

    DriverVkInstance* driverInstance = DriverVkInstance::FromVkInstance(instance);

#if DRIVER_DUMMY_DEVICE
    {
        VkPhysicalDevice physicalDevice = nullptr;

        if(driverInstance->PhysicalDevices.find(DUMMY_ADAPTER_LUID) != driverInstance->PhysicalDevices.end())
        {
            // Create a new DriverVkPhysicalDevice aligned to the system word size (not what x86 NASM considers WORD, the max size of the standard register set).
            physicalDevice = reinterpret_cast<VkPhysicalDevice>(new(DriverVkAlignment, ::std::nothrow) DriverVkPhysicalDevice(driverInstance, DUMMY_ADAPTER_GUID, DUMMY_ADAPTER_LUID));
        }
        else
        {
            // Load the DriverVkPhysicalDevice from the cache.
            physicalDevice = driverInstance->PhysicalDevices[DUMMY_ADAPTER_LUID];
        }

        // Store the GPU into the first device slot.
        pPhysicalDevices[ourAdapterCount] = physicalDevice;

        // Increment the adapter count.
        ++ourAdapterCount;
    }
#endif

    for(uSys i = 0; i < adapterCount; ++i)
    {
        const VkResult checkAdapterResult = CheckGDIAdapter(&adapters[i]);

        // Propagate Out of Memory and Unknown errors.
        if(checkAdapterResult == VK_ERROR_OUT_OF_HOST_MEMORY || checkAdapterResult == VK_ERROR_UNKNOWN)
        {
            return checkAdapterResult;
        }

        // If the adapter was ours, increment the count and store it in the device list.
        if(checkAdapterResult == VK_SUCCESS)
        {
            // If we've already filled the pPhysicalDevicesBuffer return continue counting our adapters, but don't store them.
            if(ourAdapterCount == maxAdapters)
            {
                ++ourAdapterCount;
                continue;
            }

            GUID adapterGuid;

            // Get the adapter GUID.
            GetAdapterGUID(&adapters[i], &adapterGuid);

            VkPhysicalDevice physicalDevice = nullptr;

            if(driverInstance->PhysicalDevices.find(adapters[i].AdapterLuid) != driverInstance->PhysicalDevices.end())
            {
                // Create a new DriverVkPhysicalDevice aligned to the system word size (not what x86 NASM considers WORD, the max size of the standard register set).
                physicalDevice = reinterpret_cast<VkPhysicalDevice>(new(DriverVkAlignment, ::std::nothrow) DriverVkPhysicalDevice(driverInstance, DUMMY_ADAPTER_GUID, adapters[i].AdapterLuid));
            }
            else
            {
                // Load the DriverVkPhysicalDevice from the cache.
                physicalDevice = driverInstance->PhysicalDevices[DUMMY_ADAPTER_LUID];
            }

            // Store the GPU into the current device slot.
            pPhysicalDevices[ourAdapterCount] = physicalDevice;

            // Increment the adapter count.
            ++ourAdapterCount;
        }

        D3DKMT_CLOSEADAPTER closeAdapterArgs {};
        closeAdapterArgs.hAdapter = adapters[i].hAdapter;

        const NTSTATUS closeAdapterRet = GDICloseAdapter(&closeAdapterArgs);

        // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
        if(closeAdapterRet == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
        {
            ConPrinter::Print(u"\u001B[91mERROR: D3DKMTCloseAdapter returned STATUS_INVALID_PARAMETER while querying adapters.\u001B[0m\n");
            return VK_ERROR_UNKNOWN;
        }

        // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
        if(closeAdapterRet != static_cast<NTSTATUS>(STATUS_SUCCESS))
        {
            ConPrinter::Print(u"\u001B[91mERROR: D3DKMTCloseAdapter returned unknown error 0x{X} while querying adapters.\u001B[0m\n", closeAdapterRet);
            return VK_ERROR_UNKNOWN;
        }
    }

    *pPhysicalDeviceCount = ourAdapterCount;

    // If there were more adapters than could be stored return VK_INCOMPLETE.
    if(ourAdapterCount > maxAdapters)
    {
        return VK_INCOMPLETE;
    }

    return VK_SUCCESS;
}

static VkResult GetAdapterGUID(const D3DKMT_ADAPTERINFO* const adapterInfo, GUID* const pAdapterGuid) noexcept
{
    // ReSharper disable once CppInitializedValueIsAlwaysRewritten
    D3DKMT_QUERYADAPTERINFO queryArgs {};
    queryArgs.hAdapter = adapterInfo->hAdapter;

    // Query the adapter GUID.
    queryArgs.Type = KMTQAITYPE_ADAPTERGUID;
    queryArgs.pPrivateDriverData = pAdapterGuid;
    queryArgs.PrivateDriverDataSize = sizeof(*pAdapterGuid);

    const NTSTATUS queryStatus = GDIQueryAdapterInfo(&queryArgs);

    // If GDI returns no memory, inform the application we're out of host memory.
    if(queryStatus == static_cast<NTSTATUS>(STATUS_NO_MEMORY))
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // If GDI returns that the device got removed, inform the application that the device was lost.
    if(queryStatus == static_cast<NTSTATUS>(STATUS_DEVICE_REMOVED))
    {
        return VK_ERROR_DEVICE_LOST;
    }

    // This should never happen, if it does inform the application that an unknown error occurred and log the incident.
    if(queryStatus == static_cast<NTSTATUS>(STATUS_INVALID_PARAMETER))
    {
        ConPrinter::Print(u"\u001B[91mD3DQueryAdapterInfo returned STATUS_INVALID_PARAMETER while querying an adapter GUID\u001B[0m.\n");
        return VK_ERROR_UNKNOWN;
    }

    if(queryStatus != static_cast<NTSTATUS>(STATUS_SUCCESS))
    {
        ConPrinter::Print(u"\u001B[91mD3DQueryAdapterInfo returned unknown status code 0x{X} while querying an adapter GUID.\u001B[0m\n", queryStatus);
        return VK_ERROR_UNKNOWN;
    }

    return VK_SUCCESS;
}

static void FillVkPhysicalDeviceLimits(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits* const pLimits) noexcept
{
    UNUSED(physicalDevice);

    // This is just dummy data based mostly on my RTX 3070 Ti.
    pLimits->maxImageDimension1D = 32768;
    pLimits->maxImageDimension2D = 32768;
    pLimits->maxImageDimension3D = 16384;
    pLimits->maxImageDimensionCube = 32768;
    pLimits->maxImageArrayLayers = 2048;
    pLimits->maxTexelBufferElements = 134217728;
    pLimits->maxUniformBufferRange = 65536;
    pLimits->maxStorageBufferRange = IntMaxMin<decltype(pLimits->maxStorageBufferRange)>::Max;
    pLimits->maxPushConstantsSize = 256;
    pLimits->maxMemoryAllocationCount = 4096;
    pLimits->maxSamplerAllocationCount = 4000;
    pLimits->bufferImageGranularity = 1024;
    pLimits->sparseAddressSpaceSize = 1ull << 40ull;
    pLimits->maxBoundDescriptorSets = 32;
}

static void FillVkPhysicalDeviceDepthStencilResolveProperties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceDepthStencilResolveProperties* const pProperties) noexcept
{
    UNUSED(physicalDevice);

    pProperties->supportedDepthResolveModes = VK_RESOLVE_MODE_NONE;
    pProperties->supportedStencilResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    pProperties->independentResolveNone = VK_FALSE;
    pProperties->independentResolve = VK_FALSE;
}

static void FillVkPhysicalDeviceDescriptorIndexingProperties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceDescriptorIndexingProperties* const pProperties) noexcept
{
    UNUSED(physicalDevice);
    UNUSED(pProperties);
    
}

static void FillVkPhysicalDeviceVulkan11Properties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceVulkan11Properties* const pProperties) noexcept
{
    UNUSED(physicalDevice);
    UNUSED(pProperties);

    const DriverVkPhysicalDevice* driverPhysicalDevice = DriverVkPhysicalDevice::FromVkPhysicalDevice(physicalDevice);

    static_assert(VK_UUID_SIZE == sizeof(GUID), "Vulkan UUID size did not match that of a GUID.");
    static_assert(VK_LUID_SIZE == sizeof(LUID), "Vulkan LUID size did not match that of a LUID.");

    ::std::memcpy(pProperties->deviceUUID, &driverPhysicalDevice->DeviceGuid, VK_UUID_SIZE);
    ::std::memset(pProperties->driverUUID, 0, VK_UUID_SIZE);
    ::std::memcpy(pProperties->deviceLUID, &driverPhysicalDevice->DeviceLuid, VK_LUID_SIZE);

}

static void FillVkPhysicalDeviceFeatures(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* const pFeatures) noexcept
{
    UNUSED(physicalDevice);

    pFeatures->robustBufferAccess = VK_TRUE;
    pFeatures->fullDrawIndexUint32 = VK_TRUE;
    pFeatures->imageCubeArray = VK_TRUE;
    pFeatures->independentBlend = VK_TRUE;
    pFeatures->geometryShader = VK_FALSE;
    pFeatures->tessellationShader = VK_FALSE;
    pFeatures->sampleRateShading = VK_FALSE;
    pFeatures->dualSrcBlend = VK_TRUE;
    pFeatures->logicOp = VK_TRUE;
    pFeatures->multiDrawIndirect = VK_TRUE;
    pFeatures->drawIndirectFirstInstance = VK_TRUE;
    pFeatures->depthClamp = VK_FALSE;
    pFeatures->depthBiasClamp = VK_FALSE;
    pFeatures->fillModeNonSolid = VK_TRUE;
    pFeatures->depthBounds = VK_TRUE;
    pFeatures->wideLines = VK_TRUE;
    pFeatures->largePoints = VK_TRUE;
    pFeatures->alphaToOne = VK_TRUE;
    pFeatures->multiViewport = VK_FALSE;
    pFeatures->samplerAnisotropy = VK_TRUE;
    pFeatures->textureCompressionETC2 = VK_FALSE;
    pFeatures->textureCompressionASTC_LDR = VK_FALSE;
    pFeatures->textureCompressionBC = VK_TRUE;
    pFeatures->occlusionQueryPrecise = VK_TRUE;
    pFeatures->pipelineStatisticsQuery = VK_TRUE;
    pFeatures->vertexPipelineStoresAndAtomics = VK_TRUE;
    pFeatures->fragmentStoresAndAtomics = VK_TRUE;
    pFeatures->shaderTessellationAndGeometryPointSize = VK_FALSE;
    pFeatures->shaderImageGatherExtended = VK_FALSE;
    pFeatures->shaderStorageImageExtendedFormats = VK_TRUE;
    pFeatures->shaderStorageImageMultisample = VK_FALSE;
    pFeatures->shaderStorageImageReadWithoutFormat = VK_TRUE;
    pFeatures->shaderStorageImageWriteWithoutFormat = VK_TRUE;
    pFeatures->shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
    pFeatures->shaderSampledImageArrayDynamicIndexing = VK_TRUE;
    pFeatures->shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
    pFeatures->shaderStorageImageArrayDynamicIndexing = VK_TRUE;
    pFeatures->shaderClipDistance = VK_FALSE;
    pFeatures->shaderCullDistance = VK_FALSE;
    pFeatures->shaderFloat64 = VK_TRUE;
    pFeatures->shaderInt64 = VK_TRUE;
    pFeatures->shaderInt16 = VK_TRUE;
    pFeatures->shaderResourceResidency = VK_FALSE;
    pFeatures->shaderResourceMinLod = VK_TRUE;
    pFeatures->sparseBinding = VK_FALSE;
    pFeatures->sparseResidencyBuffer = VK_FALSE;
    pFeatures->sparseResidencyImage2D = VK_FALSE;
    pFeatures->sparseResidencyImage3D = VK_FALSE;
    pFeatures->sparseResidency2Samples = VK_FALSE;
    pFeatures->sparseResidency4Samples = VK_FALSE;
    pFeatures->sparseResidency8Samples = VK_FALSE;
    pFeatures->sparseResidency16Samples = VK_FALSE;
    pFeatures->sparseResidencyAliased = VK_FALSE;
    pFeatures->variableMultisampleRate = VK_FALSE;
    pFeatures->inheritedQueries = VK_FALSE;
}

}
