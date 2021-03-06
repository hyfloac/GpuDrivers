#pragma once

#include <vulkan/vk_icd.h>
#include <Objects.hpp>
#include <EnumBitFields.hpp>
#include <unordered_map>

#include "DriverAlignment.hpp"
#include "_Resharper.h"
#include "PhysicalDeviceManager.hpp"
#include "WindowsNtPolyfill.hpp"

namespace vk {

enum class DriverVkInstanceFlags : uint32_t
{
    None = 0,
    IsCustomAllocated  = 1 << 0x00
};

}

// Declare bit operations on DriverVkInstanceFlags.
ENUM_FLAGS(vk::DriverVkInstanceFlags);

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceVersion(uint32_t* pApiVersion) noexcept;
VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) noexcept;
VKAPI_ATTR void VKAPI_CALL DriverVkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) noexcept;

#define DRIVER_VK_INSTANCE_TAG_FILTER (DRIVER_VK_INSTANCE_ALIGNMENT - 1)

/**
 * The implementation version of VkInstance.
 *
 *   Alignment should be either 8 or 4, depending on if it is a 64 or 32
 * bit architecture respectively.
 *
 *   The application name is stored as an additional null terminated string
 * beyond the structure. If the application name was not passed with will
 * just be an empty null terminated string.
 */
class DriverVkInstance final
{
    DELETE_CM(DriverVkInstance);
public:
    DriverVkInstance() noexcept
        : LoaderVTable{ 0 }
        , ApiVersion(0)
        , Flags(DriverVkInstanceFlags::None)
        , ExtensionCount(0)
        , ExtensionsSize(0)
        , Extensions(nullptr)
    {
        // Store the magic value for the loader.
        set_loader_magic_value(this);
    }

    DriverVkInstance(const uint32_t apiVersion, const bool isCustomAllocated, const uint32_t extensionCount, const size_t extensionsSize) noexcept
        : LoaderVTable{ 0 }
        , ApiVersion(apiVersion)
        , Flags(DriverVkInstanceFlags::None)
        , ExtensionCount(extensionCount)
        , ExtensionsSize(extensionsSize)
        , Extensions(nullptr)
    {
        // Store the magic value for the loader.
        set_loader_magic_value(this);

        // Set the IsCustomAllocated Flag if applicable.
        if(isCustomAllocated)
        {
            Flags = DriverVkInstanceFlags::IsCustomAllocated;
        }
    }

    ~DriverVkInstance() noexcept
    {
        for(auto it : PhysicalDevices)
        {
            // Invoke the destructor
            DriverVkPhysicalDevice::FromVkPhysicalDevice(it.second)->~DriverVkPhysicalDevice();
            // Free using the aligned delete operator.
            ::operator delete(it.second, DriverVkAlignment, ::std::nothrow);
        }
    }

    /**
     * Checks the flags to see if this was allocated with a user allocator.
     */
    [[nodiscard]] bool IsCustomAllocated() const noexcept
    {
        return HasFlag(Flags, DriverVkInstanceFlags::IsCustomAllocated);
    }
public:
    VK_LOADER_DATA LoaderVTable;
    uint32_t ApiVersion;
    DriverVkInstanceFlags Flags;
    uint32_t ExtensionCount;
    size_t ExtensionsSize;
    const char* const* Extensions;
    ::std::unordered_map<LUID, VkPhysicalDevice> PhysicalDevices;
public:
    /**
     * Reinterprets as a DriverVkInstance*.
     */
    [[nodiscard]] static DriverVkInstance* FromVkInstance(const VkInstance instance) noexcept
    {
        //   VkInstance is akin to a type-safe-ish void*, so we tell the
        // application that they have a VkInstance, when in reality it is a
        // DriverVkInstance, so we can just do a simple reinterpret cast.
        return reinterpret_cast<DriverVkInstance*>(instance);
    }
    
    /**
     * Checks the flags to see if this was allocated with a user allocator.
     */
    [[nodiscard]] static bool IsCustomAllocated(const VkInstance instance) noexcept
    {
        return FromVkInstance(instance)->IsCustomAllocated();
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

// Test that VK_LOADER_DATA is at the offset that the Vulkan Loader expects.
static_assert(offsetof(DriverVkInstance, LoaderVTable) == 0, "VK_LOADER_DATA was not at offset 0 for DriverVkInstance.");

}
