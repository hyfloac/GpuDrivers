#pragma once

#include <vulkan/vk_icd.h>

// #include "_Resharper.h"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceVersion(uint32_t* pApiVersion) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) noexcept;

#ifdef WIN32
#define DRIVER_VK_INSTANCE_ALIGNMENT (4)
#else
#define DRIVER_VK_INSTANCE_ALIGNMENT (8)
#endif

#define DRIVER_VK_INSTANCE_TAG_FILTER (DRIVER_VK_INSTANCE_ALIGNMENT - 1)

/**
 * The implementation version of VkInstance.
 *
 *   If this was allocated using a user allocator the LSB of the pointer
 * will be tagged 1, otherwise 0. Alignment should be either 8 or 4,
 * depending on if it is a 64 or 32 bit architecture respectively.
 *
 *   The application name is stored as an additional null terminated string
 * beyond the structure. If the application name was not passed with will
 * just be an empty null terminated string.
 */
struct DriverVkInstance final
{
	VK_LOADER_DATA LoaderVTable;
	uint32_t ApiVersion;

	/**
	 * Untags the pointer and reinterprets as a DriverVkInstance*.
	 */
	[[nodiscard]] static DriverVkInstance* FromVkInstance(const VkInstance instance) noexcept
	{
		// Offset by one byte to untag the pointer instead of performing bitwise and.
		// This protects provenance information: https://reviews.llvm.org/D91055
		return reinterpret_cast<DriverVkInstance*>(reinterpret_cast<char*>(instance) - 1);
	}


	/**
	 *   Checks the pointer tag to see if this was allocated with a user
	 * allocator.
	 */
	[[nodiscard]] static bool IsCustomAllocated(const VkInstance instance) noexcept
	{
		return reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(instance)) & 0x01;
	}

	/**
	 * Gets the application name from just beyond the instance.
	 */
	[[nodiscard]] static const char* GetApplicationName(const DriverVkInstance* const instance) noexcept
	{
		return reinterpret_cast<const char*>(instance + 1);
	}

	/**
	 * Gets the application name from just beyond the instance.
	 *
	 *   A simple wrapper that will first convert the VkInstance to a
	 * DriverVkInstance.
	 */
	[[nodiscard]] static const char* GetApplicationName(const VkInstance instance) noexcept
	{
		return GetApplicationName(FromVkInstance(instance));
	}
};

static_assert(offsetof(DriverVkInstance, LoaderVTable) == 0, "VK_LOADER_DATA was not at offset 0 for DriverVkInstance.");

}
