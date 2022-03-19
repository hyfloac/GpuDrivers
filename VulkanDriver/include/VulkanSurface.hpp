#pragma once

#include <vulkan/vk_icd.h>

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) noexcept;
VKAPI_ATTR void     VKAPI_CALL DriverVkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) noexcept;

}
