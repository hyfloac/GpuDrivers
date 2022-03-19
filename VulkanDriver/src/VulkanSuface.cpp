#include <vulkan/vk_icd.h>
#include <Windows.h>

#include "VulkanSurface.hpp"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* const pCreateInfo, const VkAllocationCallbacks* const pAllocator, VkSurfaceKHR* const pSurface) noexcept
{
	OutputDebugStringA("vkCreateWin32SurfaceKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR void VKAPI_CALL DriverVkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) noexcept
{
	OutputDebugStringA("vkDestroySurfaceKHR\n");
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* const pSupported) noexcept
{
	OutputDebugStringA("vkGetPhysicalDeviceSurfaceSupportKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* const pSurfaceCapabilities) noexcept
{
	OutputDebugStringA("vkGetPhysicalDeviceSurfaceCapabilitiesKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* const pSurfaceFormatCount, VkSurfaceFormatKHR* const pSurfaceFormats) noexcept
{
	OutputDebugStringA("vkGetPhysicalDeviceSurfaceFormatsKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* const pPresentModeCount, VkPresentModeKHR* const pPresentModes) noexcept
{
	OutputDebugStringA("vkGetPhysicalDeviceSurfacePresentModesKHR\n");
	return VK_ERROR_UNKNOWN;
}

}
