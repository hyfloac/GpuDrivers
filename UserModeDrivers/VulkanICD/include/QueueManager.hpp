#pragma once

#include <vulkan/vk_icd.h>
#include <Objects.hpp>

#include "_Resharper.h"

namespace vk {

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) noexcept;

}
