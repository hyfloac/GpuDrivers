#pragma once

#include <vulkan/vk_icd.h>
#include <Objects.hpp>
#include <EnumBitFields.hpp>
#include <ConPrinter.hpp>

#include "WindowsNtPolyfill.hpp"
#include "PhysicalDeviceManager.hpp"
#include "InstanceManager.hpp"
#include "ConfigMacros.hpp"
#include "GdiThunks.hpp"

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
    DELETE_CM(DriverVkLogicalDevice);
public:
    DriverVkLogicalDevice() noexcept
        : LoaderVTable{ 0 }
        , PhysicalDevice(nullptr)
        , Flags(DriverVkLogicalDeviceFlags::None)
        , DeviceHandle(-1)
        , CommandBuffer(nullptr)
        , CommandBufferSize(0)
        , AllocationList(nullptr)
        , AllocationListSize(0)
    {
        // Store the magic value for the loader.
        set_loader_magic_value(this);
    }

    DriverVkLogicalDevice(DriverVkPhysicalDevice* const physicalDevice, const bool isCustomAllocated, const D3DKMT_CREATEDEVICE& createDeviceArgs) noexcept
        : LoaderVTable{ 0 }
        , PhysicalDevice(physicalDevice)
        , Flags(DriverVkLogicalDeviceFlags::None)
        , DeviceHandle(createDeviceArgs.hDevice)
        , CommandBuffer(createDeviceArgs.pCommandBuffer)
        , CommandBufferSize(createDeviceArgs.CommandBufferSize)
        , AllocationList(createDeviceArgs.pAllocationList)
        , AllocationListSize(createDeviceArgs.AllocationListSize)
    {
        // Store the magic value for the loader.
        set_loader_magic_value(this);

        // Set the IsCustomAllocated Flag if applicable.
        if(isCustomAllocated)
        {
            Flags = DriverVkLogicalDeviceFlags::IsCustomAllocated;
        }
    }

    ~DriverVkLogicalDevice() noexcept
    {
        // Destroy the GDI device handle.
        D3DKMT_DESTROYDEVICE destroyDeviceArgs {};
        destroyDeviceArgs.hDevice = DeviceHandle;

        const NTSTATUS destroyDeviceStatus = GDIDestroyDevice(&destroyDeviceArgs);

        // If device destruction fails we'll create a debug message.
#if DRIVER_DEBUG_LOG
        if(!NT_SUCCESS(destroyDeviceStatus))
        {
            ConPrinter::Print("For some reason we failed to destroy the logical device.\n");
        }
#endif
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
    D3DKMT_HANDLE DeviceHandle;
    VOID* CommandBuffer;
    UINT CommandBufferSize;
    D3DDDI_ALLOCATIONLIST* AllocationList;
    UINT AllocationListSize;
};

static_assert(offsetof(DriverVkLogicalDevice, LoaderVTable) == 0, "VK_LOADER_DATA was not at offset 0 for DriverVkLogicalDevice.");

}
