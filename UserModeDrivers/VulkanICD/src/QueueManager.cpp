#include <vulkan/vulkan.h>
#include <new>
#include <cstring>
#include <Utils.hpp>
#include <ConPrinter.hpp>
#include <DynArray.hpp>

#include "InstanceManager.hpp"
#include "ConfigMacros.hpp"
#include "DriverAlignment.hpp"
#include "GdiThunks.hpp"
#include "QueueManager.hpp"

namespace vk {

static void FillVkQueueFamilyProperties(VkPhysicalDevice physicalDevice, VkQueueFamilyProperties* properties, u32 index) noexcept;

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceQueueFamilyProperties(const VkPhysicalDevice physicalDevice, uint32_t* const pQueueFamilyPropertyCount, VkQueueFamilyProperties* const pQueueFamilyProperties) noexcept
{
#if DRIVER_DEBUG_LOG
    ConPrinter::Print("Application is querying device queues (gen 1).\n");
#endif

    // We have a graphics queue, compute queue, and a transfer queue.

    if(!pQueueFamilyProperties)
    {
        // Inform the application we have 3 queues.
        *pQueueFamilyPropertyCount = 3;
    }
    else
    {
        // Limit to only 3 queues.
        if(*pQueueFamilyPropertyCount > 3)
        {
            *pQueueFamilyPropertyCount = 3;
        }

        // Cache the queueFamilyCount;
        const uint32_t queueFamilyCount = *pQueueFamilyPropertyCount;

        // Fill in each of the queue properties.
        for(uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            FillVkQueueFamilyProperties(physicalDevice, &pQueueFamilyProperties[i], i);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceQueueFamilyProperties2(const VkPhysicalDevice physicalDevice, uint32_t* const pQueueFamilyPropertyCount, VkQueueFamilyProperties2* const pQueueFamilyProperties) noexcept
{
#if DRIVER_DEBUG_LOG
    ConPrinter::Print("Application is querying device queues (gen 2).\n");
#endif

    // We have a graphics queue, compute queue, and a transfer queue.

    if(!pQueueFamilyProperties)
    {
        // Inform the application we have 3 queues.
        *pQueueFamilyPropertyCount = 3;
    }
    else
    {
        // Limit to only 3 queues.
        if(*pQueueFamilyPropertyCount > 3)
        {
            *pQueueFamilyPropertyCount = 3;
        }

        // Cache the queueFamilyCount;
        const uint32_t queueFamilyCount = *pQueueFamilyPropertyCount;
        
        // Fill in each of the queue properties.
        // There are currently no next structures.
        for(uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            FillVkQueueFamilyProperties(physicalDevice, &pQueueFamilyProperties[i].queueFamilyProperties, i);
        }
    }
}

static void FillVkQueueFamilyProperties(const VkPhysicalDevice physicalDevice, VkQueueFamilyProperties* const properties, const u32 index) noexcept
{
    UNUSED(physicalDevice);

    switch(index)
    {
        case 0:
            properties->queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            properties->queueCount = 1;
            properties->timestampValidBits = 48;
            properties->minImageTransferGranularity.width = 1;
            properties->minImageTransferGranularity.height = 1;
            properties->minImageTransferGranularity.depth = 1;
            break;
        case 1:
            properties->queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            properties->queueCount = 1;
            properties->timestampValidBits = 48;
            properties->minImageTransferGranularity.width = 1;
            properties->minImageTransferGranularity.height = 1;
            properties->minImageTransferGranularity.depth = 1;
            break;
        case 2:
            properties->queueFlags = VK_QUEUE_TRANSFER_BIT;
            properties->queueCount = 1;
            properties->timestampValidBits = 48;
            properties->minImageTransferGranularity.width = 1;
            properties->minImageTransferGranularity.height = 1;
            properties->minImageTransferGranularity.depth = 1;
            break;
        default: break;
    }
}

}
