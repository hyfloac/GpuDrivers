#pragma once

#include <vulkan/vulkan.h>

namespace vk {

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL DriverVkGetDeviceProcAddr(VkDevice device, const char* pName) noexcept;

}
