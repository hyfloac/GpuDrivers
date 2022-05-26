#include <vulkan/vk_icd.h>
#include <ConPrinter.hpp>
#include <Utils.hpp>
#include <String.hpp>
#include <unordered_set>

#include "ExtensionManager.hpp"
#include "ConfigMacros.hpp"

#include "_Resharper.h"

namespace vk {

static constexpr VkExtensionProperties InstanceExtensions[] = {
    {
        "VK_KHR_surface",
        25
    },
    {
        "VK_KHR_win32_surface",
        6
    }
};

static constexpr uint32_t InstanceExtensionCount = ::std::size(InstanceExtensions);

static constexpr VkExtensionProperties DeviceExtensions[] = {
    {
        "VK_KHR_swapchain",
        70
    }
};

static constexpr uint32_t DeviceExtensionCount = ::std::size(DeviceExtensions);

/**
 *	 This acts as a set of all available instance extensions by this
 * driver. The intent is to lookup extensions by name, then use the
 * distance of the returned iterator as a unique key to decrease memory
 * and search time within the enabled extension list of the VkInstance.
 */
static ::std::unordered_set<DynString> InstanceExtensionSet;
/**
 *	 This acts as a set of all available device extensions by this
 * driver. The intent is to lookup extensions by name, then use the
 * distance of the returned iterator as a unique key to decrease memory
 * and search time within the enabled extension list of the VkInstance.
 */
static ::std::unordered_set<DynString> DeviceExtensionSet;

void InitExtensionSet() noexcept
{
    // Stores whether this function has already been called.
    static bool initialized = false;

    // Ensure this function can't be run more than once.
    if(!initialized)
    {
        initialized = true;

        // Iterate through all supported instance extensions.
        for(uSys i = 0; i < InstanceExtensionCount; ++i)
        {
            // Insert the instance extension name into the set.
            //   We reinterpret_cast to a const char* because it is stored as a fixed
            // size char[] which would impact how it is represented in the DynStringT.
            InstanceExtensionSet.emplace(reinterpret_cast<const char*>(InstanceExtensions[i].extensionName));
        }

        // Iterate through all supported device extensions.
        for(uSys i = 0; i < DeviceExtensionCount; ++i)
        {
            // Insert the device extension name into the set.
            //   We reinterpret_cast to a const char* because it is stored as a fixed
            // size char[] which would impact how it is represented in the DynStringT.
            DeviceExtensionSet.emplace(reinterpret_cast<const char*>(DeviceExtensions[i].extensionName));
        }
    }
}

/**
 *   Gets the name of an extension from the index of either an instance
 * or device extension.
 */
[[nodiscard]] static const DynString& GetExtension(const iSys index, const ::std::unordered_set<DynString>& extensions) noexcept
{
    // Used to return a const& if the extension could not be found.
    static DynString defaultReturn;

    // Check that the index is in bounds.
    if(index >= static_cast<iSys>(extensions.size()) || index < 0)
    {
        // If the index is not in bounds return a null string.
        return defaultReturn;
    }

    // Get a const iterator to the beginning of the set.
    auto iter = extensions.cbegin();
    // Offset to by index to get the proper iterator.
    ::std::advance(iter, index);

    if(iter == extensions.cend())
    {
        // If the index somehow exceeded the iterator return a null string.
        return defaultReturn;
    }

    // Return the string.
    return *iter;
}

/**
 *   Gets the index of an extension from the name of either an instance
 * or device extension.
 */
[[nodiscard]] static iSys GetExtension(const DynString& name, const ::std::unordered_set<DynString>& extensions) noexcept
{
    // Get a const iterator to the extension in the set.
    const auto iter = extensions.find(name);

    // Check that the extension was found.
    if(iter != extensions.cend())
    {
        // Convert the iterator to a index from the beginning of the set iterator.
        return static_cast<iSys>(::std::distance(extensions.cbegin(), iter));
    }

    // If the extension was not found in the set return -1.
    return -1;
}

[[nodiscard]] const DynString& GetInstanceExtension(const iSys index) noexcept
{
    return GetExtension(index, InstanceExtensionSet);
}

[[nodiscard]] iSys GetInstanceExtension(const DynString& name) noexcept
{
    return GetExtension(name, InstanceExtensionSet);
}

[[nodiscard]] const DynString& GetDeviceExtension(const iSys index) noexcept
{
    return GetExtension(index, DeviceExtensionSet);
}

[[nodiscard]] iSys GetDeviceExtension(const DynString& name) noexcept
{
    return GetExtension(name, DeviceExtensionSet);
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceExtensionProperties(const char* const pLayerName, uint32_t* const pPropertyCount, VkExtensionProperties* const pProperties) noexcept
{
    UNUSED(pLayerName);

#if DRIVER_DEBUG_LOG
    if(pLayerName)
    {
        ConPrinter::Print(u"Application is attempting to query {} extensions from layer {}.\n", *pPropertyCount, pLayerName);
    }
    else
    {
        ConPrinter::Print(u"Application is attempting to query {} extensions.\n", *pPropertyCount);
    }
#endif

    // Only return the number of available extensions.
    if(!pProperties)
    {
        *pPropertyCount = InstanceExtensionCount;
        // For some reason vkcube fails if we return VK_INCOMPLETE here.
        return VK_SUCCESS;
    }
    
    // Has the application requested all of the instance extensions?
    if(*pPropertyCount < InstanceExtensionCount)
    {
        // Loop through *pPropertyCount extensions.
        for(uint32_t i = 0, count = *pPropertyCount; i < count; ++i)
        {
            // Copy the extension name. Using a constant length is often faster than trying to only do the minimum size due to memcpy being an intrinsic function.
            (void) ::std::memcpy(pProperties[i].extensionName, InstanceExtensions[i].extensionName, sizeof(pProperties[i].extensionName));
            // Copy the extension version.
            pProperties[i].specVersion = InstanceExtensions[i].specVersion;
        }

        // We've filled up pProperties, but there are still more extensions.
        return VK_INCOMPLETE;
    }
    else
    {
        // Loop through ExtensionCount extensions. Due to being a constant value this may be faster.
        for(uint32_t i = 0; i < InstanceExtensionCount; ++i)
        {
            // Copy the extension name. Using a constant length is often faster than trying to only do the minimum size due to memcpy being an intrinsic function.
            (void) ::std::memcpy(pProperties[i].extensionName, InstanceExtensions[i].extensionName, sizeof(pProperties[i].extensionName));
            // Copy the extension version.
            pProperties[i].specVersion = InstanceExtensions[i].specVersion;
        }

        // We've stored all of the extensions into the Application's buffer.
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateDeviceExtensionProperties(const VkPhysicalDevice physicalDevice, const char* const pLayerName, uint32_t* const pPropertyCount, VkExtensionProperties* const pProperties) noexcept
{
    UNUSED2(physicalDevice, pLayerName);

#if DRIVER_DEBUG_LOG
    if(pLayerName)
    {
        ConPrinter::Print(u"Application is attempting to query {} device extensions from layer {}.\n", *pPropertyCount, pLayerName);
    }
    else
    {
        ConPrinter::Print(u"Application is attempting to query {} device extensions.\n", *pPropertyCount);
    }
#endif

    // Only return the number of available extensions.
    if(!pProperties)
    {
        *pPropertyCount = DeviceExtensionCount;
        // For some reason vkcube fails if we return VK_INCOMPLETE here.
        return VK_SUCCESS;
    }

    // Has the application requested all of the device extensions?
    if(*pPropertyCount < DeviceExtensionCount)
    {
        // Loop through *pPropertyCount extensions.
        for(uint32_t i = 0, count = *pPropertyCount; i < count; ++i)
        {
            // Copy the extension name. Using a constant length is often faster than trying to only do the minimum size due to memcpy being an intrinsic function.
            (void) ::std::memcpy(pProperties[i].extensionName, DeviceExtensions[i].extensionName, sizeof(pProperties[i].extensionName));
            // Copy the extension version.
            pProperties[i].specVersion = DeviceExtensions[i].specVersion;
        }

        // We've filled up pProperties, but there are still more extensions.
        return VK_INCOMPLETE;
    }
    else
    {
        // Loop through ExtensionCount extensions. Due to being a constant value this may be faster.
        for(uint32_t i = 0; i < DeviceExtensionCount; ++i)
        {
            // Copy the extension name. Using a constant length is often faster than trying to only do the minimum size due to memcpy being an intrinsic function.
            (void) ::std::memcpy(pProperties[i].extensionName, DeviceExtensions[i].extensionName, sizeof(pProperties[i].extensionName));
            // Copy the extension version.
            pProperties[i].specVersion = DeviceExtensions[i].specVersion;
        }

        // We've stored all of the extensions into the Application's buffer.
        return VK_SUCCESS;
    }
}

}
