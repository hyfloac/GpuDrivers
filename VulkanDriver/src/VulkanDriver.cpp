#define VK_NO_PROTOTYPES
#include <vulkan/vk_icd.h>
#include <cstring>
#include <Windows.h>
#include "InstanceManagement.hpp"
#include "VulkanSurface.hpp"

#ifdef _DEBUG
static void TestThatFunctionDeclarationsMatch();
#endif

template<size_t Len>
inline consteval size_t FindHashCodeConst(const char(&str)[Len]) noexcept
{
	size_t hash = 0;
	for(size_t i = 0; str[i]; ++i)
	{
		hash = 31u * hash + static_cast<size_t>(str[i]);
	}
	return hash;
}

inline size_t FindHashCode(const char* str) noexcept;

uint32_t g_VulkanLoaderVersion = 0;

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* const pVersion)
{
#ifdef _DEBUG
	// Dummy to ignore the unused warning. This should do literally nothing.
	TestThatFunctionDeclarationsMatch();
	OutputDebugStringA("Negotiating ICD Interface\n");
#endif

	// Check that the pointer is not null. This should never actually enter.
	if(!pVersion)
	{
		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}

	// Cache the version. This should be unnecessary with optimizations.
	const uint32_t version = *pVersion;

	// Cache the loader version in global memory.
	g_VulkanLoaderVersion = version;

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
		g_VulkanLoaderVersion = 5;
		return VK_SUCCESS;
	}

	// This should never be encountered.
	return VK_ERROR_INCOMPATIBLE_DRIVER;
}

#define STRING_CASE(FUNC_NAME, FUNC) \
	case FindHashCodeConst(FUNC_NAME): \
		if(::std::strcmp(FUNC_NAME, pName) == 0) { return reinterpret_cast<PFN_vkVoidFunction>(FUNC); } else { break; }

__declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* const pName)
{
	(void)instance;
	(void)pName;

	// If pName is null behaviour is undefined, we'll just return null.
	if(!pName)
	{
		return nullptr;
	}
	
	OutputDebugStringA("Loader is attempting to get function ");
	OutputDebugStringA(pName);
	OutputDebugStringA("\n");

	// Get global functions
	if(!instance)
	{
		switch(FindHashCode(pName))
		{
			STRING_CASE("vkCreateInstance", DriverVkCreateInstance)
			STRING_CASE("vkDestroyInstance", DriverVkDestroyInstance)
			STRING_CASE("vkEnumerateInstanceExtensionProperties", DriverVkEnumerateInstanceExtensionProperties)
			default: break;
		}
	}
	else
	{
		const DriverVkInstance* const driverInstance = DriverVkInstance::FromVkInstance(instance);

		const uint32_t apiVersion = driverInstance->ApiVersion;

		if(apiVersion >= VK_API_VERSION_1_2)
		{
			
		}

		if(apiVersion >= VK_API_VERSION_1_1)
		{
			
		}

		if(apiVersion >= VK_API_VERSION_1_0)
		{
			switch(FindHashCode(pName))
			{
				STRING_CASE("vkGetPhysicalDeviceProperties", DriverVkEnumerateInstanceExtensionProperties)
				STRING_CASE("vkCreateWin32SurfaceKHR", vk::DriverVkCreateWin32SurfaceKHR);
				default: break;
			}
		}
	}

	OutputDebugStringA("Failed to find function.\n");

	return nullptr;
}

__declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetPhysicalDeviceProcAddr(VkInstance instance, const char* const pName)
{
	(void) instance;
	(void) pName;

	OutputDebugStringA("Loader is getting physical device procedure ");
	OutputDebugStringA(pName);
	OutputDebugStringA("\n");

	return nullptr;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef _DEBUG

#define FUNC_DECL_TESTER(TYPEDEF, FUNC) { const TYPEDEF func = FUNC; (void) func; } 

static void TestThatFunctionDeclarationsMatch()
{
	FUNC_DECL_TESTER(PFN_vkCreateInstance, DriverVkCreateInstance);
	FUNC_DECL_TESTER(PFN_vkDestroyInstance, DriverVkDestroyInstance);
	FUNC_DECL_TESTER(PFN_vkEnumerateInstanceExtensionProperties, DriverVkEnumerateInstanceExtensionProperties);

	FUNC_DECL_TESTER(PFN_vkCreateWin32SurfaceKHR, vk::DriverVkCreateWin32SurfaceKHR);
	FUNC_DECL_TESTER(PFN_vkDestroySurfaceKHR, vk::DriverVkDestroySurfaceKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfaceSupportKHR, vk::DriverVkGetPhysicalDeviceSurfaceSupportKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vk::DriverVkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, vk::DriverVkGetPhysicalDeviceSurfaceFormatsKHR);
	FUNC_DECL_TESTER(PFN_vkGetPhysicalDeviceSurfacePresentModesKHR, vk::DriverVkGetPhysicalDeviceSurfacePresentModesKHR);
}
#endif

inline size_t FindHashCode(const char* str) noexcept
{
	size_t hash = 0;
	for(size_t i = 0; str[i]; ++i)
	{
		hash = 31u * hash + static_cast<size_t>(str[i]);
	}
	return hash;
}
