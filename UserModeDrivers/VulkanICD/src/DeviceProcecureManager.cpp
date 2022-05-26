#include <ConPrinter.hpp>
#include <vulkan/vulkan.h>

#include "DeviceProcedureManager.hpp"
#include "LogicalDeviceManager.hpp"
#include "ConfigMacros.hpp"
#include "ExtensionManager.hpp"

#include "_Resharper.h"

#if DRIVER_DUMMY_PUCK_FUNCTION
// Dummy function used to trick the application into believing all functions exist.
// This let's see every function the Application intends to use.
static VKAPI_ATTR VkResult VKAPI_CALL DriverVkPuckFunction() noexcept
{
	return VK_SUCCESS;
}
#endif

#define STRING_CASE(FUNC_NAME, FUNC) STR_CASE(FUNC_NAME, { return reinterpret_cast<PFN_vkVoidFunction>(FUNC); })

namespace vk {

PFN_vkVoidFunction DriverVkGetDeviceProcAddr(const VkDevice device, const char* const pName) noexcept
{
	// If pName is null behaviour is undefined, we'll just return null.
	if(!pName)
	{
		return nullptr;
	}

	// If device is null behaviour is undefined, we'll just return null.
	if(!device)
	{
		return nullptr;
	}

#if DRIVER_DEBUG_LOG
	ConPrinter::Print(u"Application is attempting to get device function {}.\n", pName);
#endif

	// Containerize the function name so that we use its hash for switch statements.
	const DynString funcName(pName);
	
	// Get our DriverVkInstance class from the passed in instance.
	const DriverVkLogicalDevice* const driverLogicalDevice = DriverVkLogicalDevice::FromVkDevice(device);
	// Cache the API version in a register. Strictly speaking, this is unnecessary, but does reduce some verbosity.
	const uint32_t apiVersion = driverLogicalDevice->PhysicalDevice->Instance->ApiVersion;

	// Get a function from Vulkan 1.3.
	if(apiVersion >= VK_API_VERSION_1_3)
	{

	}

	// Get a function from Vulkan 1.2.
	if(apiVersion >= VK_API_VERSION_1_2)
	{

	}

	// Get a function from Vulkan 1.1.
	if(apiVersion >= VK_API_VERSION_1_1)
	{
		STR_SWITCH(funcName,
		{
		}, { });
	}

	// Get a function from Vulkan 1.0.
	if(apiVersion >= VK_API_VERSION_1_0)
	{
		STR_SWITCH(funcName,
		{
			STRING_CASE("vkEnumerateDeviceExtensionProperties", vk::DriverVkEnumerateDeviceExtensionProperties);
		}, { });
	}

#if DRIVER_DEBUG_LOG
	ConPrinter::Print(u"Failed to find function.\n");
#endif

#if DRIVER_DUMMY_PUCK_FUNCTION
	return reinterpret_cast<PFN_vkVoidFunction>(DriverVkPuckFunction);
#else
	return nullptr;
#endif
}

}
