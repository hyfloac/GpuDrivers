#include <vulkan/vk_icd.h>
#include <ConPrinter.hpp>

#include "VulkanSurface.hpp"
#include "ConfigMacros.hpp"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* const pCreateInfo, const VkAllocationCallbacks* const pAllocator, VkSurfaceKHR* const pSurface) noexcept
{
	UNUSED4(instance, pCreateInfo, pAllocator, pSurface);

#if DRIVER_DEBUG_LOG
	ConPrinter::Print("vkCreateWin32SurfaceKHR\n");
#endif
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR void VKAPI_CALL DriverVkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* const pAllocator) noexcept
{
	UNUSED3(instance, surface, pAllocator);

#if DRIVER_DEBUG_LOG
	ConPrinter::Print("vkDestroySurfaceKHR\n");
#endif
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, const uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* const pSupported) noexcept
{
	UNUSED4(physicalDevice, queueFamilyIndex, surface, pSupported);

#if DRIVER_DEBUG_LOG
	ConPrinter::Print("vkGetPhysicalDeviceSurfaceSupportKHR\n");
#endif
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* const pSurfaceCapabilities) noexcept
{
	UNUSED3(physicalDevice, surface, pSurfaceCapabilities);

#if DRIVER_DEBUG_LOG
	ConPrinter::Print("vkGetPhysicalDeviceSurfaceCapabilitiesKHR\n");
#endif
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* const pSurfaceFormatCount, VkSurfaceFormatKHR* const pSurfaceFormats) noexcept
{
	UNUSED4(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

#if DRIVER_DEBUG_LOG
	ConPrinter::Print("vkGetPhysicalDeviceSurfaceFormatsKHR\n");
#endif
	return VK_ERROR_UNKNOWN;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* const pPresentModeCount, VkPresentModeKHR* const pPresentModes) noexcept
{
	UNUSED4(physicalDevice, surface, pPresentModeCount, pPresentModes);

#if DRIVER_DEBUG_LOG
	ConPrinter::Print("vkGetPhysicalDeviceSurfacePresentModesKHR\n");
#endif
	return VK_ERROR_UNKNOWN;
}

}
