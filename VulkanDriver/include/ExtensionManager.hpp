#pragma once

#include <vulkan/vulkan.h>

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) noexcept;

}
