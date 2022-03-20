#pragma once

#include <vulkan/vk_icd.h>

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) noexcept;

struct DriverVkInstance;

struct DriverVkPhysicalDevice final
{
	VK_LOADER_DATA LoaderVTable;
	DriverVkInstance* Instance;
	GUID DeviceUuid;

	[[nodiscard]] static DriverVkPhysicalDevice* FromVkPhysicalDevice(VkPhysicalDevice physicalDevice) noexcept
	{
		return reinterpret_cast<DriverVkPhysicalDevice*>(physicalDevice);
	}
};

static_assert(offsetof(DriverVkPhysicalDevice, LoaderVTable) == 0, "VK_LOADER_DATA was not at offset 0 for DriverVkPhysicalDevice.");

}
