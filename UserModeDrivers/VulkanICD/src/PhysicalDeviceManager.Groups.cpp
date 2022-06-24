#include <vulkan/vulkan.h>
#include <new>
#include <cstring>
#include <Utils.hpp>
#include <ConPrinter.hpp>

#include "PhysicalDeviceManager.hpp"
#include "ConfigMacros.hpp"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumeratePhysicalDeviceGroups(const VkInstance instance, uint32_t* const pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* const pPhysicalDeviceGroupProperties) noexcept
{
    UNUSED(instance);

#if DRIVER_DEBUG_LOG
    ConPrinter::Print(u"Enumerating {} Physical Device Groups.\n", *pPhysicalDeviceGroupCount);
#endif

    // If pPhysicalDeviceGroupProperties is null simply query the device count.
    if(!pPhysicalDeviceGroupProperties)
    {
        return DriverVkEnumeratePhysicalDevices(instance, pPhysicalDeviceGroupCount, nullptr);
    }

    // Query the number of physical devices under our domain, each will be in their own group.
    uint32_t physicalDeviceCount;
    const VkResult queryCountResult = DriverVkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    // Propagate any errors querying the physical device count.
    if(queryCountResult != VK_SUCCESS)
    {
        return queryCountResult;
    }

    // If the application is querying fewer physical devices than actually exists we'll only populate that many devices.
    if(*pPhysicalDeviceGroupCount < physicalDeviceCount)
    {
        // If the query count is less than or equal to 16 we can use stack allocation.
        if(*pPhysicalDeviceGroupCount <= 16)
        {
            // Only load at most pPhysicalDeviceGroupCount devices.
            physicalDeviceCount = *pPhysicalDeviceGroupCount;

            // Setup a stack allocated buffer of 16 devices.
            VkPhysicalDevice devices[16];
            // Load the devices.
            const VkResult queryDevicesResults = DriverVkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices);

            // Propagate any errors querying the physical devices.
            if(queryDevicesResults != VK_SUCCESS && queryDevicesResults != VK_INCOMPLETE)
            {
                return queryCountResult;
            }

            // Store each device into their own group.
            for(uint32_t i = 0; i < physicalDeviceCount; ++i)
            {
                pPhysicalDeviceGroupProperties[i].physicalDeviceCount = 1;
                pPhysicalDeviceGroupProperties[i].physicalDevices[0] = devices[i];
                pPhysicalDeviceGroupProperties[i].subsetAllocation = VK_FALSE;
            }

            // Store the number of devices written.
            *pPhysicalDeviceGroupCount = physicalDeviceCount;

            // Propagate the return result of enumerating the devices. vkEnumeratePhysicalDevices can change between calls, so it may return VK_SUCCESS.
            return queryDevicesResults;
        }
        else
        {
            // Only load at most pPhysicalDeviceGroupCount devices.
            physicalDeviceCount = *pPhysicalDeviceGroupCount;

            // Setup a heap allocated buffer of pPhysicalDeviceGroupCount devices.
            VkPhysicalDevice* devices = new(::std::nothrow) VkPhysicalDevice[physicalDeviceCount];
            // Load the devices.
            const VkResult queryDevicesResults = DriverVkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices);

            // Propagate any errors querying the physical devices.
            if(queryDevicesResults != VK_SUCCESS && queryDevicesResults != VK_INCOMPLETE)
            {
                // Cleanup devices.
                delete[] devices;

                return queryCountResult;
            }

            // Store each device into their own group.
            for(uint32_t i = 0; i < physicalDeviceCount; ++i)
            {
                pPhysicalDeviceGroupProperties[i].physicalDeviceCount = 1;
                pPhysicalDeviceGroupProperties[i].physicalDevices[0] = devices[i];
                pPhysicalDeviceGroupProperties[i].subsetAllocation = VK_FALSE;
            }

            // Store the number of devices written.
            *pPhysicalDeviceGroupCount = physicalDeviceCount;

            // Cleanup devices.
            delete[] devices;

            // Propagate the return result of enumerating the devices. vkEnumeratePhysicalDevices can change between calls, so it may return VK_SUCCESS.
            return queryDevicesResults;
        }
    }
    else
    {
        // If the device count is less than or equal to 16 we can use stack allocation.
        if(physicalDeviceCount <= 16)
        {
            // Setup a stack allocated buffer of 16 devices.
            VkPhysicalDevice devices[16];
            // Load the devices.
            const VkResult queryDevicesResults = DriverVkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices);

            // Propagate any errors querying the physical devices.
            if(queryDevicesResults != VK_SUCCESS && queryDevicesResults != VK_INCOMPLETE)
            {
                return queryCountResult;
            }

            // Store each device into their own group.
            for(uint32_t i = 0; i < physicalDeviceCount; ++i)
            {
                pPhysicalDeviceGroupProperties[i].physicalDeviceCount = 1;
                pPhysicalDeviceGroupProperties[i].physicalDevices[0] = devices[i];
                pPhysicalDeviceGroupProperties[i].subsetAllocation = VK_FALSE;
            }

            // Store the number of devices written.
            *pPhysicalDeviceGroupCount = physicalDeviceCount;

            // Propagate the return result of enumerating the devices. vkEnumeratePhysicalDevices can change between calls, so it may return VK_INCOMPLETE.
            return queryDevicesResults;
        }
        else
        {
            // Setup a heap allocated buffer of pPhysicalDeviceGroupCount devices.
            VkPhysicalDevice* devices = new(::std::nothrow) VkPhysicalDevice[physicalDeviceCount];
            // Load the devices.
            const VkResult queryDevicesResults = DriverVkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices);

            // Propagate any errors querying the physical devices.
            if(queryDevicesResults != VK_SUCCESS && queryDevicesResults != VK_INCOMPLETE)
            {
                // Cleanup devices.
                delete[] devices;

                return queryCountResult;
            }

            // Store each device into their own group.
            for(uint32_t i = 0; i < physicalDeviceCount; ++i)
            {
                pPhysicalDeviceGroupProperties[i].physicalDeviceCount = 1;
                pPhysicalDeviceGroupProperties[i].physicalDevices[0] = devices[i];
                pPhysicalDeviceGroupProperties[i].subsetAllocation = VK_FALSE;
            }

            // Store the number of devices written.
            *pPhysicalDeviceGroupCount = physicalDeviceCount;

            // Cleanup devices.
            delete[] devices;

            // Propagate the return result of enumerating the devices. vkEnumeratePhysicalDevices can change between calls, so it may return VK_INCOMPLETE.
            return queryDevicesResults;
        }
    }
}

}
