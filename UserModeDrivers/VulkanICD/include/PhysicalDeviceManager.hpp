#pragma once

#include <vulkan/vk_icd.h>
#include <utility>

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) noexcept;

struct DriverVkInstance;

class DriverVkPhysicalDevice final
{
	DEFAULT_DESTRUCT(DriverVkPhysicalDevice);
	DEFAULT_CM_PU(DriverVkPhysicalDevice);
public:
	DriverVkPhysicalDevice() noexcept
		: LoaderVTable{ 0 }
	    , Instance(nullptr)
	    , DeviceUuid{ }
	{
		// Store the magic value for the loader.
		set_loader_magic_value(this);
	}

	DriverVkPhysicalDevice(DriverVkInstance* const instance, const GUID& deviceUuid) noexcept
		: LoaderVTable { 0 }
		, Instance(instance)
		, DeviceUuid(deviceUuid)
	{
		// Store the magic value for the loader.
		set_loader_magic_value(this);
	}

	DriverVkPhysicalDevice(DriverVkInstance* const instance, GUID&& deviceUuid) noexcept
		: LoaderVTable { 0 }
		, Instance(instance)
		, DeviceUuid(::std::move(deviceUuid))
	{
		// Store the magic value for the loader.
		set_loader_magic_value(this);
	}
public:
	VK_LOADER_DATA LoaderVTable;
	DriverVkInstance* Instance;
	GUID DeviceUuid;
public:
	[[nodiscard]] static DriverVkPhysicalDevice* FromVkPhysicalDevice(const VkPhysicalDevice physicalDevice) noexcept
	{
		return reinterpret_cast<DriverVkPhysicalDevice*>(physicalDevice);
	}
};

static_assert(offsetof(DriverVkPhysicalDevice, LoaderVTable) == 0, "VK_LOADER_DATA was not at offset 0 for DriverVkPhysicalDevice.");

}
