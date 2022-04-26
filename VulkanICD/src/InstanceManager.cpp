#include <vulkan/vk_icd.h>
#include <cstring>
#include <new>
#include <Windows.h>
#include <ConPrinter.hpp>

#include "InstanceManager.hpp"
#include "ConfigMacros.hpp"
#include "DriverAlignment.hpp"

#include "_Resharper.h"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceVersion(uint32_t* const pApiVersion) noexcept
{
#if DRIVER_DEBUG_LOG
    ConPrinter::Print(u"Informing Loader/Application that we support Vulkan 1.3.\n");
#endif

    // Inform the application that we support Vulkan 1.3.
    // This may need to change in the future to a lower, true level of support.
    *pApiVersion = VK_API_VERSION_1_3;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateInstance(const VkInstanceCreateInfo* const pCreateInfo, const VkAllocationCallbacks* const pAllocator, VkInstance* const pInstance) noexcept
{
#if DRIVER_DEBUG_LOG
    if(pCreateInfo && pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->pApplicationName)
    {
        ConPrinter::Print("Application {} is creating a VkInstance.\n", pCreateInfo->pApplicationInfo->pApplicationName);
    }
    else
    {
        ConPrinter::Print("Application is creating a VkInstance.\n");
    }

    if(pCreateInfo && pCreateInfo->enabledExtensionCount != 0 && pCreateInfo->ppEnabledExtensionNames)
    {
        ConPrinter::Print("Application is requesting {} extensions:\n", pCreateInfo->enabledExtensionCount);
        for(uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i)
        {
            ConPrinter::Print("  {}\n", pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }
#endif

    // The targeted API version.
    uint32_t apiVersion = 0;
    // The length of the application name if set plus a null terminator.
    size_t applicationNameLength = 1;

    // Application info is not necessarily non-null.
    if(pCreateInfo && pCreateInfo->pApplicationInfo)
    {
        apiVersion = pCreateInfo->pApplicationInfo->apiVersion;
        // Application name is not necessarily non-null.
        if(pCreateInfo->pApplicationInfo->pApplicationName)
        {
            // Get the application name length and add a null terminator.
            applicationNameLength = ::std::strlen(pCreateInfo->pApplicationInfo->pApplicationName) + 1;
        }
    }

    //   The size of our extensions allocation, this at a minimum is the
    // size of a 0 length string (with the null terminator) and a pointer
    // to that string.
    size_t extensionsSize = 9;

    // If there are any requested extensions compute the size required to store them for ourselves.
    if(pCreateInfo && pCreateInfo->enabledExtensionCount != 0)
    {
        // Set extensions size to be the size of all the required pointers.
        extensionsSize = pCreateInfo->enabledExtensionCount * sizeof(const char*);

        for(uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i)
        {
            // Add the length of each null terminated extension name to the extensions size.
            extensionsSize += (::std::strlen(pCreateInfo->ppEnabledExtensionNames[i]) + 1) * sizeof(char);
        }
    }

    // We're going to keep our own copy of the application name and enabled extensions, so let's bundle them together into the same allocation.
    const size_t instanceSize = sizeof(DriverVkInstance) + applicationNameLength * sizeof(char) + extensionsSize;

    // A block of memory to create our driver instance from.
    void* placement;
    // Whether pAllocator was successfully used, this may be false if pAllocator returns null.
    bool isCustomAllocated = false;

    // pAllocator is not necessarily non-null. If it is not, we'll use the user supplied allocator.
    if(pAllocator)
    {
        // Allocate a block of memory for placement new.
        placement = pAllocator->pfnAllocation(pAllocator->pUserData, instanceSize, DriverVkAlignmentSz, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

        // pAllocator can fail.
        if(!placement)
        {
            // Fallback to using an aligned new. We'll ensure alignment to the system word size (not was NASM calls WORD) because why not.
            placement = ::operator new(instanceSize, DriverVkAlignment, ::std::nothrow);

            // If new failed then our virtual address space is entirely out of memory. Consider using 64-bit application for 48 or 52 bits of address space.
            if(!placement)
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }
        }
        else
        {
            // Custom allocation succeeded.
            isCustomAllocated = true;

            // The information callbacks are not necessarily non-null.
            if(pAllocator->pfnInternalAllocation)
            {
                // Inform the allocator about our allocation.
                pAllocator->pfnInternalAllocation(pAllocator->pUserData, instanceSize, VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
            }
        }
    }
    else // Use the C++ allocator.
    {
        // Call the more explicit new to get a raw void*. We'll ensure alignment to the system word size (not was NASM calls WORD) because why not.
        placement = ::operator new(instanceSize, DriverVkAlignment, ::std::nothrow);

        // If new failed then our virtual address space is entirely out of memory. Consider using 64 bit for 48 or 52 bits of address space.
        if(!placement)
        {
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }
    }

    // Create and initialize our internal version of VkInstance;
    DriverVkInstance* const driverInstance = new(placement) DriverVkInstance(apiVersion, isCustomAllocated, pCreateInfo->enabledExtensionCount, extensionsSize);
    
    // If the application name was set, store it, otherwise set it to a null string.
    if(applicationNameLength > 1)
    {
        (void) ::std::memcpy(driverInstance + 1, pCreateInfo->pApplicationInfo->pApplicationName, applicationNameLength * sizeof(char));
    }
    else
    {
        *reinterpret_cast<char*>(driverInstance + 1) = '\0';
    }
    
    // The pointer to our extensions
    void* const extensionPointer = reinterpret_cast<char*>(driverInstance + 1) + applicationNameLength;
    // Cast the extensionPointer to an array of pointers to strings.
    char** const extensionPointerTable = reinterpret_cast<char**>(extensionPointer);

    // If we have any enabled extensions copy them.
    if(pCreateInfo && pCreateInfo->enabledExtensionCount != 0)
    {
        // Compute the extension head as being after the pointer table.
        char* currentExtensionHead = reinterpret_cast<char*>(extensionPointerTable + pCreateInfo->enabledExtensionCount);

        // Iterate over each of the extensions for copying.
        for(uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i)
        {
            // Set the pointer to the string.
            extensionPointerTable[i] = currentExtensionHead;
            // Compute the extension name length including the null terminator.
            const size_t extensionNameLength = ::std::strlen(pCreateInfo->ppEnabledExtensionNames[i]) + 1;
            // Copy the string.
            (void) ::std::memcpy(currentExtensionHead, pCreateInfo->ppEnabledExtensionNames[i], extensionNameLength);
            // Offset the currentExtensionHead by the extension name length.
            currentExtensionHead += extensionNameLength;
        }
    }
    else // If there are no enabled extensions initialize a pointer to a null string.
    {
        extensionPointerTable[0] = reinterpret_cast<char*>(extensionPointerTable + 1);
        extensionPointerTable[0][0] = '\0';
    }

    *pInstance = reinterpret_cast<VkInstance>(driverInstance);

    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL DriverVkDestroyInstance(const VkInstance instance, const VkAllocationCallbacks* const pAllocator) noexcept
{
    // If instance is null fast-exit.
    if(!instance)
    {
        return;
    }

#if DRIVER_DEBUG_LOG
    if(*DriverVkInstance::GetApplicationName(instance))
    {
        ConPrinter::Print(u"Freeing VkInstance from application {}.\n", DriverVkInstance::GetApplicationName(instance));
    }
    else
    {
        ConPrinter::Print(u"Freeing VkInstance from unnamed application.\n");
    }
#endif

    // Invoke the destructor as we're essentially going to free a void*.
    DriverVkInstance::FromVkInstance(instance)->~DriverVkInstance();

    // If this was custom allocated we need to deallocate with the allocator callbacks.
    if(DriverVkInstance::IsCustomAllocated(instance))
    {
        // We're allowed to assume that pAllocator is not null if instance was allocated with a custom allocator.
        pAllocator->pfnFree(pAllocator->pUserData, instance);

        // The information callbacks are not necessarily non-null.
        if(pAllocator->pfnInternalFree)
        {
            // The information callback requires the allocation size.
            const size_t size = sizeof(DriverVkInstance) + (::std::strlen(DriverVkInstance::GetApplicationName(instance)) + 1) * sizeof(char) + DriverVkInstance::FromVkInstance(instance)->ExtensionsSize;
            // Inform the allocator about our freeing of an allocation.
            pAllocator->pfnInternalFree(pAllocator->pUserData, size, VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        }
    }
    else // Call the C++ aligned delete instead.
    {
        // We have to use the more explicit delete as we had set alignments, and also used the more explicit new.
        ::operator delete(instance, DriverVkAlignment, ::std::nothrow);
    }
}

}
