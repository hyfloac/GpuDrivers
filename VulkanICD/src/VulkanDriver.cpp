// See https://vulkan.lunarg.com/doc/view/1.3.204.1/mac/LoaderDriverInterface.html
#define VK_NO_PROTOTYPES
#include <vulkan/vk_icd.h>
#include <cstring>
#include <Windows.h>
#include <ConPrinter.hpp>
#include <String.hpp>

#include "ExtensionManager.hpp"
#include "InstanceManager.hpp"
#include "PhysicalDeviceManager.hpp"
#include "VulkanSurface.hpp"
#include "DeviceProcedureManager.hpp"
#include "LogicalDeviceManager.hpp"
#include "ConfigMacros.hpp"

#include "_Resharper.h"

#define STRING_CASE(FUNC_NAME, FUNC) STR_CASE(FUNC_NAME, { return reinterpret_cast<PFN_vkVoidFunction>(FUNC); })

#ifdef _DEBUG
[[maybe_unused]] static void TestThatFunctionDeclarationsMatch() noexcept;
#endif

namespace vk {
uint32_t g_VulkanLoaderVersion = 0;
}

#if DRIVER_DUMMY_PUCK_FUNCTION
// Dummy function used to trick the application into believing all functions exist.
// This let's see every function the Application intends to use.
static VKAPI_ATTR VkResult VKAPI_CALL DriverVkPuckFunction() noexcept
{
	return VK_SUCCESS;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* const pVersion) noexcept
{
#if _DEBUG
	// Dummy to ignore the unused warning. This should do literally nothing.
	TestThatFunctionDeclarationsMatch();
#endif

#if DRIVER_DEBUG_LOG
	ConPrinter::Print(u"Vulkan Loader is negotiating for loader version {}.\n", *pVersion);
#endif

	// Check that the pointer is not null. This should never actually enter.
	if(!pVersion)
	{
		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}

	// Cache the version. This should be unnecessary with optimizations.
	const uint32_t version = *pVersion;

	// Cache the loader version in global memory.
    vk::g_VulkanLoaderVersion = version;

	// We support Loader V5, if less than 5 we fail.
	if(version <= 4)
	{
		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}
	else if(version == 5) // Versions match. Succeed.
	{
		return VK_SUCCESS;
	}
	else if(version > 5) // Loader is newer than our driver, succeed, but inform the loader that we're still on V5.
	{
		// Tell the loader use V5.
		*pVersion = 5;
		// Update the global cache to be V5.
		vk::g_VulkanLoaderVersion = 5;
		return VK_SUCCESS;
	}

	// This should never be encountered.
	return VK_ERROR_INCOMPATIBLE_DRIVER;
}

__declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(const VkInstance instance, const char* const pName) noexcept
{
	// If pName is null behaviour is undefined, we'll just return null.
	if(!pName)
	{
		return nullptr;
	}

#if DRIVER_DEBUG_LOG
	ConPrinter::Print(u"Loader is attempting to get function {}.\n", pName);
#endif

	// Containerize the function name so that we use its hash for switch statements.
	const DynString funcName(pName);

	// Get a global function.
	if(!instance)
	{
		STR_SWITCH(funcName, 
		{
			STRING_CASE("vkEnumerateInstanceVersion", vk::DriverVkEnumerateInstanceVersion);
		    STRING_CASE("vkCreateInstance", vk::DriverVkCreateInstance);
			STRING_CASE("vkEnumerateInstanceExtensionProperties", vk::DriverVkEnumerateInstanceExtensionProperties);
		}, { });
	}
	else // Get an instance function.
	{
		// Get our DriverVkInstance class from the passed in instance.
		const vk::DriverVkInstance* const driverInstance = vk::DriverVkInstance::FromVkInstance(instance);
		// Cache the API version in a register. Strictly speaking, this is unnecessary, but does reduce some verbosity.
		const uint32_t apiVersion = driverInstance->ApiVersion;

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
				STRING_CASE("vkGetPhysicalDeviceProperties2", vk::DriverVkGetPhysicalDeviceProperties2);
				STRING_CASE("vkGetPhysicalDeviceFeatures2", vk::DriverVkGetPhysicalDeviceFeatures2);
				STRING_CASE("vkGetPhysicalDeviceFormatProperties2", vk::DriverVkGetPhysicalDeviceFormatProperties2);
			}, { });
		}

		// Get a function from Vulkan 1.0.
		if(apiVersion >= VK_API_VERSION_1_0)
		{
			STR_SWITCH(funcName,
			{
				STRING_CASE("vkDestroyInstance", vk::DriverVkDestroyInstance);
				STRING_CASE("vkEnumeratePhysicalDevices", vk::DriverVkEnumeratePhysicalDevices);
				STRING_CASE("vkGetPhysicalDeviceProperties", vk::DriverVkGetPhysicalDeviceProperties);
				STRING_CASE("vkGetPhysicalDeviceFeatures", vk::DriverVkGetPhysicalDeviceFeatures);
				STRING_CASE("vkGetPhysicalDeviceFormatProperties", vk::DriverVkGetPhysicalDeviceFormatProperties);
				STRING_CASE("vkCreateWin32SurfaceKHR", vk::DriverVkCreateWin32SurfaceKHR);
				STRING_CASE("vkGetDeviceProcAddr", vk::DriverVkGetDeviceProcAddr);
				STRING_CASE("vkCreateDevice", vk::DriverVkCreateDevice);
				STRING_CASE("vkDestroyDevice", vk::DriverVkDestroyDevice);
			    STRING_CASE("vkEnumerateDeviceExtensionProperties", vk::DriverVkEnumerateDeviceExtensionProperties);
			}, { });
		}
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

__declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetPhysicalDeviceProcAddr(const VkInstance instance, const char* const pName) noexcept
{
	// If pName is null behaviour is undefined, we'll just return null.
	if(!pName)
	{
		return nullptr;
	}

#if DRIVER_DEBUG_LOG
	ConPrinter::Print(u"Loader is attempting to get physical device function {}.\n", pName);
#endif

	// Get a global function. This should never occur in this function.
	if(!instance)
	{
		return nullptr;
	}
	else // Get an instance function.
	{
	    // Containerize the function name so that we use its hash for switch statements.
		const DynString funcName(pName);
		// Get our DriverVkInstance class from the passed in instance.
		const vk::DriverVkInstance* const driverInstance = vk::DriverVkInstance::FromVkInstance(instance);
		// Cache the API version in a register. Strictly speaking, this is unnecessary, but does reduce some verbosity.
		const uint32_t apiVersion = driverInstance->ApiVersion;

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
				STRING_CASE("vkGetPhysicalDeviceProperties2", vk::DriverVkGetPhysicalDeviceProperties2);
				STRING_CASE("vkGetPhysicalDeviceFeatures2", vk::DriverVkGetPhysicalDeviceFeatures2);
				STRING_CASE("vkGetPhysicalDeviceFormatProperties2", vk::DriverVkGetPhysicalDeviceFormatProperties2);
			}, { });
		}

		// Get a function from Vulkan 1.1.
		if(apiVersion >= VK_API_VERSION_1_0)
		{
			STR_SWITCH(funcName,
			{
				STRING_CASE("vkGetPhysicalDeviceProperties", vk::DriverVkGetPhysicalDeviceProperties);
				STRING_CASE("vkGetPhysicalDeviceFeatures", vk::DriverVkGetPhysicalDeviceFeatures);
				STRING_CASE("vkGetPhysicalDeviceFormatProperties", vk::DriverVkGetPhysicalDeviceFormatProperties);
				STRING_CASE("vkEnumerateDeviceExtensionProperties", vk::DriverVkEnumerateDeviceExtensionProperties);
			}, { });
		}
	}

#ifdef _DEBUG
	ConPrinter::Print(u"Failed to find function {}.\n", pName);
#endif

#if DRIVER_DUMMY_PUCK_FUNCTION
	return reinterpret_cast<PFN_vkVoidFunction>(DriverVkPuckFunction);
#else
	return nullptr;
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef _DEBUG
[[maybe_unused]] static void TestThatFunctionDeclarationsMatch() noexcept
{
#define FUNC_DECL_TESTER(TYPEDEF, FUNC) { const TYPEDEF func = FUNC; (void) func; }
	FUNC_DECL_TESTER(PFN_vkCreateInstance, vk::DriverVkCreateInstance);
	FUNC_DECL_TESTER(PFN_vkDestroyInstance, vk::DriverVkDestroyInstance);
	FUNC_DECL_TESTER(PFN_vkEnumerateInstanceExtensionProperties, vk::DriverVkEnumerateInstanceExtensionProperties);
	FUNC_DECL_TESTER(PFN_vkEnumerateDeviceExtensionProperties, vk::DriverVkEnumerateDeviceExtensionProperties);

	FUNC_DECL_TESTER(PFN_vkCreateWin32SurfaceKHR, vk::DriverVkCreateWin32SurfaceKHR);
	FUNC_DECL_TESTER(PFN_vkDestroySurfaceKHR, vk::DriverVkDestroySurfaceKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfaceSupportKHR, vk::DriverVkGetPhysicalDeviceSurfaceSupportKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vk::DriverVkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, vk::DriverVkGetPhysicalDeviceSurfaceFormatsKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfacePresentModesKHR, vk::DriverVkGetPhysicalDeviceSurfacePresentModesKHR);

	FUNC_DECL_TESTER(PFN_vkEnumeratePhysicalDevices, vk::DriverVkEnumeratePhysicalDevices);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceProperties, vk::DriverVkGetPhysicalDeviceProperties);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceProperties2, vk::DriverVkGetPhysicalDeviceProperties2);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceFeatures, vk::DriverVkGetPhysicalDeviceFeatures);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceFeatures2, vk::DriverVkGetPhysicalDeviceFeatures2);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceFormatProperties, vk::DriverVkGetPhysicalDeviceFormatProperties);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceFormatProperties2, vk::DriverVkGetPhysicalDeviceFormatProperties2);
#undef FUNC_DECL_TESTER
}
#endif
