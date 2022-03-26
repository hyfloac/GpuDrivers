#include <vulkan/vk_icd.h>
#include <ConPrinter.hpp>
#include <Utils.hpp>

#include "ExtensionManager.hpp"
#include "ConfigMacros.hpp"

namespace vk {

static constexpr VkExtensionProperties GlobalExtensions[] = {
	{
		"VK_KHR_surface",
		25
	},
	{
		"VK_KHR_win32_surface",
		6
	}
};

static constexpr uint32_t ExtensionCount = ::std::size(GlobalExtensions);

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceExtensionProperties(const char* const pLayerName, uint32_t* const pPropertyCount, VkExtensionProperties* const pProperties) noexcept
{
	UNUSED(pLayerName);

#if DRIVER_DEBUG_LOG
	if(pLayerName)
	{
		ConPrinter::Print(u"Application is attempting to query {} extensions from layer {}.\n", *pPropertyCount, pLayerName);
	}
	else
	{
		ConPrinter::Print(u"Application is attempting to query {} extensions.\n", *pPropertyCount);
	}
#endif

	// Only return the number of available extensions.
	if(!pProperties)
	{
		*pPropertyCount = ExtensionCount;
		// For some reason vkcube fails if we return VK_INCOMPLETE here.
		return VK_SUCCESS;
	}
	
	// Has the application requested all of the extensions?
	if(*pPropertyCount < ExtensionCount)
	{
		// Loop through *pPropertyCount extensions.
		for(uint32_t i = 0, count = *pPropertyCount; i < count; ++i)
		{
			// Copy the extension name. Using a constant length is often faster than trying to only do the minimum size due to memcpy being an intrinsic function.
			(void) ::std::memcpy(pProperties[i].extensionName, GlobalExtensions[i].extensionName, sizeof(pProperties[i].extensionName));
			// Copy the extension version.
			pProperties[i].specVersion = GlobalExtensions[i].specVersion;
		}

		// We've filled up pProperties, but there are still more extensions.
		return VK_INCOMPLETE;
	}
	else
	{
		// Loop through ExtensionCount extensions. Due to being a constant value this may be faster.
		for(uint32_t i = 0; i < ExtensionCount; ++i)
		{
			// Copy the extension name. Using a constant length is often faster than trying to only do the minimum size due to memcpy being an intrinsic function.
			(void) ::std::memcpy(pProperties[i].extensionName, GlobalExtensions[i].extensionName, sizeof(pProperties[i].extensionName));
			// Copy the extension version.
			pProperties[i].specVersion = GlobalExtensions[i].specVersion;
		}

		// We've stored all of the extensions into the Application's buffer.
		return VK_SUCCESS;
	}
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateDeviceExtensionProperties(const VkPhysicalDevice physicalDevice, const char* const pLayerName, uint32_t* const pPropertyCount, VkExtensionProperties* const pProperties) noexcept
{
	UNUSED2(physicalDevice, pLayerName);

#if DRIVER_DEBUG_LOG
	if(pLayerName)
	{
		ConPrinter::Print(u"Application is attempting to query {} device extensions from layer {}.\n", *pPropertyCount, pLayerName);
	}
	else
	{
		ConPrinter::Print(u"Application is attempting to query {} device extensions.\n", *pPropertyCount);
	}
#endif

	if(!pProperties)
	{
		*pPropertyCount = 0;
		return VK_SUCCESS;
	}

	return VK_SUCCESS;
}

}
