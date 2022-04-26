#pragma once

#include <vulkan/vk_icd.h>
#include <Objects.hpp>
#include <EnumBitFields.hpp>

#include "PhysicalDeviceManager.hpp"
#include "InstanceManager.hpp"

#include "_Resharper.h"

namespace vk {

enum class DriverVkLogicalDeviceFlags : uint32_t
{
    None = 0,
    IsCustomAllocated = 1 << 0x00
};

}

// Declare bit operations on DriverVkInstanceFlags.
ENUM_FLAGS(vk::DriverVkLogicalDeviceFlags);

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) noexcept;

class DriverVkLogicalDevice final
{
    DEFAULT_DESTRUCT(DriverVkLogicalDevice);
    DEFAULT_CM_PU(DriverVkLogicalDevice);
public:
    DriverVkLogicalDevice() noexcept
        : LoaderVTable{ 0 }
        , PhysicalDevice(nullptr)
        , Flags(DriverVkLogicalDeviceFlags::None)
    {
        // Store the magic value for the loader.
        set_loader_magic_value(this);
    }

    DriverVkLogicalDevice(DriverVkPhysicalDevice* const physicalDevice, const bool isCustomAllocated) noexcept
        : LoaderVTable{ 0 }
        , PhysicalDevice(physicalDevice)
        , Flags(DriverVkLogicalDeviceFlags::None)
    {
        // Store the magic value for the loader.
        set_loader_magic_value(this);

        // Set the IsCustomAllocated Flag if applicable.
        if(isCustomAllocated)
        {
            Flags = DriverVkLogicalDeviceFlags::IsCustomAllocated;
        }
    }

    /**
     * Checks the flags to see if this was allocated with a user allocator.
     */
    [[nodiscard]] bool IsCustomAllocated() const noexcept
    {
        return HasFlag(Flags, DriverVkLogicalDeviceFlags::IsCustomAllocated);
    }
public:
    /**
     * Reinterprets as a DriverVkLogicalDevice*.
     */
    [[nodiscard]] static DriverVkLogicalDevice* FromVkDevice(const VkDevice device) noexcept
    {
        //   VkDevice is akin to a type-safe-ish void*, so we tell the
        // application that they have a VkDevice, when in reality it is a
        // DriverVkLogicalDevice, so we can just do a simple reinterpret cast.
        return reinterpret_cast<DriverVkLogicalDevice*>(device);
    }

    /**
     * Checks the flags to see if this was allocated with a user allocator.
     */
    [[nodiscard]] static bool IsCustomAllocated(const VkDevice device) noexcept
    {
        return FromVkDevice(device)->IsCustomAllocated();
    }
public:
    VK_LOADER_DATA LoaderVTable;
    DriverVkPhysicalDevice* PhysicalDevice;
    DriverVkLogicalDeviceFlags Flags;
};

}
