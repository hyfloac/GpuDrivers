#include <vulkan/vk_icd.h>
#include <cstring>
#include <new>
#include <Windows.h>
#include <ConPrinter.hpp>
#include "InstanceManagement.hpp"

namespace vk {

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumerateInstanceVersion(uint32_t* const pApiVersion) noexcept
{
	// Inform the application that we support Vulkan 1.3.
	// This may need to change in the future to a lower, true level of support.
	*pApiVersion = VK_API_VERSION_1_3;
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL DriverVkCreateInstance(const VkInstanceCreateInfo* const pCreateInfo, const VkAllocationCallbacks* const pAllocator, VkInstance* const pInstance) noexcept
{
#ifdef _DEBUG
	if(pCreateInfo && pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->pApplicationName)
	{
		ConPrinter::Print("Application {} is creating a VkInstance.\n", pCreateInfo->pApplicationInfo->pApplicationName);
	}
	else
	{
		ConPrinter::Print("Application is creating a VkInstance.\n");
	}
#endif

	// The targeted API version.
	uint32_t apiVersion = 0;
	// The length of the application name if set plus a null terminator.
	size_t applicationNameLength = 1;

	// Application info is not necessarily non-null.
	if(pCreateInfo->pApplicationInfo)
	{
		apiVersion = pCreateInfo->pApplicationInfo->apiVersion;
		// Application name is not necessarily non-null.
		if(pCreateInfo->pApplicationInfo->pApplicationName)
		{
			// Get the application name length and add a null terminator.
			applicationNameLength = ::std::strlen(pCreateInfo->pApplicationInfo->pApplicationName) + 1;
		}
	}

	// We're going to keep our own copy of the application name, so let's bundle them together into the same allocation.
	const size_t instanceSize = sizeof(DriverVkInstance) + applicationNameLength * sizeof(char);

	// A block of memory to create our driver instance from.
	void* placement;
	// Whether pAllocator was successfully used, this may be false if pAllocator returns null.
	bool isCustomAllocated = false;

	// pAllocator is not necessarily non-null.
	if(pAllocator)
	{
		// Allocate a block of memory for placement new.
		placement = pAllocator->pfnAllocation(pAllocator->pUserData, instanceSize, DRIVER_VK_INSTANCE_ALIGNMENT, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

		// pAllocator can fail.
		if(!placement)
		{
			// Fallback to using an aligned new. We need to ensure some alignment for pointer tagging, but we'll be generous and uses the system word size (not was NASM calls WORD).
			placement = ::operator new(instanceSize, ::std::align_val_t { DRIVER_VK_INSTANCE_ALIGNMENT }, ::std::nothrow);

			// If new failed then our address space is entirely out of memory. Consider using 64 bit for 48 or 52 bits of address space.
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
	else
	{
		// Call the more explicit new to get a raw void*. We need to ensure some alignment for pointer tagging, but we'll be generous and uses the system word size (not was NASM calls WORD).
		placement = ::operator new(instanceSize, ::std::align_val_t { DRIVER_VK_INSTANCE_ALIGNMENT }, ::std::nothrow);

		// If new failed then our address space is entirely out of memory. Consider using 64 bit for 48 or 52 bits of address space.
		if(!placement)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
	}

	// Our internal version of VkInstance;
	DriverVkInstance* driverInstance = new(placement) DriverVkInstance;

	// Store the magic value for the loader.
	set_loader_magic_value(driverInstance);

	// Store the creation information.
	driverInstance->ApiVersion = apiVersion;

	// If the application name was set, store it, otherwise set it to a null string.
	if(applicationNameLength > 1)
	{
		(void) ::std::memcpy(driverInstance + 1, pCreateInfo->pApplicationInfo->pApplicationName, applicationNameLength * sizeof(char));
	}
	else
	{
		*reinterpret_cast<char*>(driverInstance + 1) = '\0';
	}

	// If were able to successfully use the custom allocator tag the pointer.
	if(isCustomAllocated)
	{
		const uintptr_t tagged = reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(driverInstance)) | 1;
		*pInstance = reinterpret_cast<VkInstance>(reinterpret_cast<void*>(tagged));
	}
	else
	{
		*pInstance = reinterpret_cast<VkInstance>(driverInstance);
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL DriverVkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* const pAllocator) noexcept
{
	// If instance is null fast-exit.
	if(!instance)
	{
		return;
	}

	// If this was custom allocated we need to deallocate with the allocator callbacks.
	if(DriverVkInstance::IsCustomAllocated(instance))
	{
		// We're allowed to assume that pAllocator is not null if instance was allocated with a custom allocator.
		pAllocator->pfnFree(pAllocator->pUserData, instance);

		// The information callbacks are not necessarily non-null.
		if(pAllocator->pfnInternalFree)
		{
			// The information callback requires the allocation size.
			const size_t size = sizeof(DriverVkInstance) + (::std::strlen(DriverVkInstance::GetApplicationName(instance)) + 1) * sizeof(char);
			// Inform the allocator about our freeing of an allocation.
			pAllocator->pfnInternalFree(pAllocator->pUserData, size, VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
		}
	}
	else // Call the C++ delete instead.
	{
		// We have to use the more explicit delete as we had set alignments, and also used the more explicit new.
		::operator delete(instance, ::std::align_val_t { DRIVER_VK_INSTANCE_ALIGNMENT }, ::std::nothrow);
	}
}

}
