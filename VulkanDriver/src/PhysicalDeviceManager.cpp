#include <vulkan/vulkan.h>
#include <Utils.hpp>
#include <new>
#include <cstring>

#include "PhysicalDeviceManager.hpp"
#include "InstanceManagement.hpp"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumeratePhysicalDevices(VkInstance instance, uint32_t* const pPhysicalDeviceCount, VkPhysicalDevice* const pPhysicalDevices) noexcept
{
	UNUSED(instance);

	if(!pPhysicalDevices)
	{
		*pPhysicalDeviceCount = 1;
		return VK_SUCCESS;
	}

	// Is the application requesting all of the devices.
	if(*pPhysicalDeviceCount < 1)
	{
		return VK_INCOMPLETE;
	}
	else
	{
		DriverVkPhysicalDevice* physicalDevice = new(std::align_val_t { sizeof(size_t) }, ::std::nothrow) DriverVkPhysicalDevice;

		// Store the magic value for the loader.
		set_loader_magic_value(physicalDevice);

		// Save the instance that owns this PhysicalDevice.
		physicalDevice->Instance = DriverVkInstance::FromVkInstance(instance);

		// Set the UUID to null as we don't actually have any physical devices at this point in time in our dev-cycle.
		(void) ::std::memset(&physicalDevice->DeviceUuid, 0, sizeof(physicalDevice->DeviceUuid));

		pPhysicalDevices[0] = reinterpret_cast<VkPhysicalDevice>(physicalDevice);
		return VK_SUCCESS;
	}
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* const pProperties) noexcept
{
	UNUSED2(physicalDevice, pProperties);
}

}
