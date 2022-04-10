#include <vulkan/vk_icd.h>
#include <ConPrinter.hpp>

#include "VulkanSurface.hpp"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* const pCreateInfo, const VkAllocationCallbacks* const pAllocator, VkSurfaceKHR* const pSurface) noexcept
{
	UNUSED4(instance, pCreateInfo, pAllocator, pSurface);

	OutputDebugStringA("vkCreateWin32SurfaceKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR void VKAPI_CALL DriverVkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* const pAllocator) noexcept
{
	UNUSED3(instance, surface, pAllocator);

	OutputDebugStringA("vkDestroySurfaceKHR\n");
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, const uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* const pSupported) noexcept
{
	UNUSED4(physicalDevice, queueFamilyIndex, surface, pSupported);

	OutputDebugStringA("vkGetPhysicalDeviceSurfaceSupportKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* const pSurfaceCapabilities) noexcept
{
	UNUSED3(physicalDevice, surface, pSurfaceCapabilities);

	OutputDebugStringA("vkGetPhysicalDeviceSurfaceCapabilitiesKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* const pSurfaceFormatCount, VkSurfaceFormatKHR* const pSurfaceFormats) noexcept
{
	UNUSED4(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

	OutputDebugStringA("vkGetPhysicalDeviceSurfaceFormatsKHR\n");
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* const pPresentModeCount, VkPresentModeKHR* const pPresentModes) noexcept
{
	UNUSED4(physicalDevice, surface, pPresentModeCount, pPresentModes);

	OutputDebugStringA("vkGetPhysicalDeviceSurfacePresentModesKHR\n");
	return VK_ERROR_UNKNOWN;
}

}
