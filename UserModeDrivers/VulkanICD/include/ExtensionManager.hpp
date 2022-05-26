#pragma once

#include <vulkan/vulkan.h>
#include <NumTypes.hpp>
#include <String.hpp>

namespace vk {

void InitExtensionSet() noexcept;
[[nodiscard]] const DynString& GetInstanceExtension(iSys index) noexcept;
[[nodiscard]] iSys GetInstanceExtension(const DynString& name) noexcept;
[[nodiscard]] const DynString& GetDeviceExtension(iSys index) noexcept;
[[nodiscard]] iSys GetDeviceExtension(const DynString& name) noexcept;

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) noexcept;

}
