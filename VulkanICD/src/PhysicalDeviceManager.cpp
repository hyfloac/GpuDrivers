#include <vulkan/vulkan.h>
#include <new>
#include <cstring>
#include <Utils.hpp>
#include <ConPrinter.hpp>

#include "PhysicalDeviceManager.hpp"
#include "InstanceManager.hpp"
#include "ConfigMacros.hpp"

namespace vk {

static void FillVkPhysicalDeviceLimits(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits* pLimits) noexcept;
static void FillVkPhysicalDeviceDepthStencilResolveProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceDepthStencilResolveProperties* pProperties) noexcept;
static void FillVkPhysicalDeviceDescriptorIndexingProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceDescriptorIndexingProperties* pProperties) noexcept;

static void FillVkPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) noexcept;

VKAPI_ATTR VkResult VKAPI_CALL DriverVkEnumeratePhysicalDevices(const VkInstance instance, uint32_t* const pPhysicalDeviceCount, VkPhysicalDevice* const pPhysicalDevices) noexcept
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
		// Create a new DriverPhysicalDevice aligned to the system word size (not what x86 NASM considers WORD, the max size of the standard register set).
		DriverVkPhysicalDevice* physicalDevice = new(std::align_val_t { sizeof(size_t) }, ::std::nothrow) DriverVkPhysicalDevice;

		// Store the magic value for the loader.
		set_loader_magic_value(physicalDevice);

		// Save the instance that owns this PhysicalDevice.
		physicalDevice->Instance = DriverVkInstance::FromVkInstance(instance);

		// Set the UUID to null as we don't actually have any physical devices at this point in time in our dev-cycle.
		(void) ::std::memset(&physicalDevice->DeviceUuid, 0, sizeof(physicalDevice->DeviceUuid));

		// Store the GPU into the first device slot.
		pPhysicalDevices[0] = reinterpret_cast<VkPhysicalDevice>(physicalDevice);
		return VK_SUCCESS;
	}
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* const pProperties) noexcept
{
	UNUSED(physicalDevice);

	// We're just going to say we support Vulkan 1.3 for now.
	pProperties->apiVersion = VK_API_VERSION_1_3;
	// This doesn't need to be a Vulkan version number, but it's an appropriate macro for creating version integers.
	pProperties->driverVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	// We don't have a vendor ID, so we'll just use a generic 0.
	pProperties->vendorID = 0;
	// We'll just start device ID's from 1 for now.
	pProperties->deviceID = 1;
	// There is no plans to fab this GPU, so we're some weird 'other' type.
	pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
	// Store the name "Serafin 1" into the device name.
	const errno_t nameCopyError = ::strncpy_s(pProperties->deviceName, sizeof(pProperties->deviceName), "Serafin 1", sizeof(pProperties->deviceName));

#if DRIVER_DEBUG_LOG
	if(nameCopyError)
	{
		ConPrinter::Print(u"Error storing device name into VkPhysicalDeviceProperties.\n");
	}
#else
	// In release mode the name will just be empty.
	(void) nameCopyError;
#endif

	// Set a null UUID for the unique device ID.
	(void) ::std::memset(&pProperties->pipelineCacheUUID, 0, sizeof(pProperties->pipelineCacheUUID));

	FillVkPhysicalDeviceLimits(physicalDevice, &pProperties->limits);
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceProperties2(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* const pProperties) noexcept
{
	DriverVkGetPhysicalDeviceProperties(physicalDevice, &pProperties->properties);

	VkBaseOutStructure* pNext = reinterpret_cast<VkBaseOutStructure*>(pProperties->pNext);

	while(pNext)
	{
		switch(pNext->sType)
		{
			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES: FillVkPhysicalDeviceDepthStencilResolveProperties(physicalDevice, reinterpret_cast<VkPhysicalDeviceDepthStencilResolveProperties*>(pNext)); break;
			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES: FillVkPhysicalDeviceDescriptorIndexingProperties(physicalDevice, reinterpret_cast<VkPhysicalDeviceDescriptorIndexingProperties*>(pNext)); break;
			default: break;
		}

		pNext = pNext->pNext;
	}
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFeatures(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* const pFeatures) noexcept
{
	FillVkPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL DriverVkGetPhysicalDeviceFeatures2(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* const pFeatures) noexcept
{
	FillVkPhysicalDeviceFeatures(physicalDevice, &pFeatures->features);
}

static void FillVkPhysicalDeviceLimits(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits* const pLimits) noexcept
{
	UNUSED(physicalDevice);

	// This is just dummy data based mostly on my RTX 3070 Ti.
	pLimits->maxImageDimension1D = 32768;
	pLimits->maxImageDimension2D = 32768;
	pLimits->maxImageDimension3D = 16384;
	pLimits->maxImageDimensionCube = 32768;
	pLimits->maxImageArrayLayers = 2048;
	pLimits->maxTexelBufferElements = 134217728;
	pLimits->maxUniformBufferRange = 65536;
	pLimits->maxStorageBufferRange = IntMaxMin<decltype(pLimits->maxStorageBufferRange)>::Max;
	pLimits->maxPushConstantsSize = 256;
	pLimits->maxMemoryAllocationCount = 4096;
	pLimits->maxSamplerAllocationCount = 4000;
	pLimits->bufferImageGranularity = 1024;
	pLimits->sparseAddressSpaceSize = 1ull << 40ull;
	pLimits->maxBoundDescriptorSets = 32;
}

static void FillVkPhysicalDeviceDepthStencilResolveProperties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceDepthStencilResolveProperties* const pProperties) noexcept
{
	UNUSED(physicalDevice);

	pProperties->supportedDepthResolveModes = VK_RESOLVE_MODE_NONE;
	pProperties->supportedStencilResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
	pProperties->independentResolveNone = VK_FALSE;
	pProperties->independentResolve = VK_FALSE;
}

static void FillVkPhysicalDeviceDescriptorIndexingProperties(const VkPhysicalDevice physicalDevice, VkPhysicalDeviceDescriptorIndexingProperties* const pProperties) noexcept
{
	UNUSED(physicalDevice);
	UNUSED(pProperties);
	
}

static void FillVkPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) noexcept
{
	UNUSED(physicalDevice);

	pFeatures->robustBufferAccess = VK_TRUE;
	pFeatures->fullDrawIndexUint32 = VK_TRUE;
	pFeatures->imageCubeArray = VK_TRUE;
	pFeatures->independentBlend = VK_TRUE;
	pFeatures->geometryShader = VK_FALSE;
	pFeatures->tessellationShader = VK_FALSE;
	pFeatures->sampleRateShading = VK_FALSE;
	pFeatures->dualSrcBlend = VK_TRUE;
	pFeatures->logicOp = VK_TRUE;
	pFeatures->multiDrawIndirect = VK_TRUE;
	pFeatures->drawIndirectFirstInstance = VK_TRUE;
	pFeatures->depthClamp = VK_FALSE;
	pFeatures->depthBiasClamp = VK_FALSE;
	pFeatures->fillModeNonSolid = VK_TRUE;
	pFeatures->depthBounds = VK_TRUE;
	pFeatures->wideLines = VK_TRUE;
	pFeatures->largePoints = VK_TRUE;
	pFeatures->alphaToOne = VK_TRUE;
	pFeatures->multiViewport = VK_FALSE;
	pFeatures->samplerAnisotropy = VK_TRUE;
	pFeatures->textureCompressionETC2 = VK_FALSE;
	pFeatures->textureCompressionASTC_LDR = VK_FALSE;
	pFeatures->textureCompressionBC = VK_TRUE;
	pFeatures->occlusionQueryPrecise = VK_TRUE;
	pFeatures->pipelineStatisticsQuery = VK_TRUE;
	pFeatures->vertexPipelineStoresAndAtomics = VK_TRUE;
	pFeatures->fragmentStoresAndAtomics = VK_TRUE;
	pFeatures->shaderTessellationAndGeometryPointSize = VK_FALSE;
	pFeatures->shaderImageGatherExtended = VK_FALSE;
	pFeatures->shaderStorageImageExtendedFormats = VK_TRUE;
	pFeatures->shaderStorageImageMultisample = VK_FALSE;
	pFeatures->shaderStorageImageReadWithoutFormat = VK_TRUE;
	pFeatures->shaderStorageImageWriteWithoutFormat = VK_TRUE;
	pFeatures->shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
	pFeatures->shaderSampledImageArrayDynamicIndexing = VK_TRUE;
	pFeatures->shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
	pFeatures->shaderStorageImageArrayDynamicIndexing = VK_TRUE;
	pFeatures->shaderClipDistance = VK_FALSE;
	pFeatures->shaderCullDistance = VK_FALSE;
	pFeatures->shaderFloat64 = VK_TRUE;
	pFeatures->shaderInt64 = VK_TRUE;
	pFeatures->shaderInt16 = VK_TRUE;
	pFeatures->shaderResourceResidency = VK_FALSE;
	pFeatures->shaderResourceMinLod = VK_TRUE;
	pFeatures->sparseBinding = VK_FALSE;
	pFeatures->sparseResidencyBuffer = VK_FALSE;
	pFeatures->sparseResidencyImage2D = VK_FALSE;
	pFeatures->sparseResidencyImage3D = VK_FALSE;
	pFeatures->sparseResidency2Samples = VK_FALSE;
	pFeatures->sparseResidency4Samples = VK_FALSE;
	pFeatures->sparseResidency8Samples = VK_FALSE;
	pFeatures->sparseResidency16Samples = VK_FALSE;
	pFeatures->sparseResidencyAliased = VK_FALSE;
	pFeatures->variableMultisampleRate = VK_FALSE;
	pFeatures->inheritedQueries = VK_FALSE;
}

}
