#include "Common.h"
#include "GsMemoryManager.hpp"
#include "Logging.h"

NTSTATUS GsMemoryManager::Init(const UINT16 deviceId, const DXGK_DEVICE_INFO& deviceInfo, const DXGKRNL_INTERFACE& dxgkInterface) noexcept
{
    TRACE_ENTRYPOINT();

    // Save the Device ID
    m_DeviceId = deviceId;
    m_DxgkInterface = dxgkInterface;

    // Load the region map for this device.
    const NTSTATUS loadBarMapStatus = LoadRegionMap();

    if(!NT_SUCCESS(loadBarMapStatus))
    {
        LOG_ERROR("Error loading bar map for device 0x%04X: 0x%08X.\n", deviceId, loadBarMapStatus);
        return loadBarMapStatus;
    }

    CM_FULL_RESOURCE_DESCRIPTOR* list = deviceInfo.TranslatedResourceList->List;

    if constexpr(EnableExtensiveLogging)
    {
        LOG_DEBUG("CM_FULL_RESOURCE_DESCRIPTOR: 0x%p.\n", list);
    }

    UINT regionIndex = 0;

    for(UINT i = 0; i < deviceInfo.TranslatedResourceList->Count; ++i)
    {
        if constexpr(EnableExtensiveLogging)
        {
            LOG_DEBUG("Resource List: %u.\n", i);
        }

        for(UINT j = 0; j < list->PartialResourceList.Count; ++j)
        {
            if constexpr(EnableExtensiveLogging)
            {
                LOG_DEBUG("Partial Resource List: %u.\n", j);
            }

            CM_PARTIAL_RESOURCE_DESCRIPTOR* const desc = &list->PartialResourceList.PartialDescriptors[j];

            if constexpr(EnableExtensiveLogging)
            {
                LOG_DEBUG("Partial Resource List Desc: 0x%p, Type: %d, m_Flags: 0x%04X.\n", desc, desc->Type, desc->Flags);
            }

            switch(desc->Type)
            {
                case CmResourceTypeMemory:
                case CmResourceTypeMemoryLarge:
                {
                    if constexpr(EnableExtensiveLogging)
                    {
                        if(desc->Type == CmResourceTypeMemory)
                        {
                            LOG_DEBUG("Handling CmResourceTypeMemory.\n");
                        }
                        else if(desc->Type == CmResourceTypeMemoryLarge)
                        {
                            LOG_DEBUG("Handling CmResourceTypeMemoryLarge.\n");
                        }
                    }

                    const NTSTATUS loadBarStatus = LoadBar(*desc, regionIndex, dxgkInterface);
                    if(!NT_SUCCESS(loadBarStatus))
                    {
                        LOG_ERROR("Error loading Region %d: 0x%08X.\n", regionIndex, loadBarStatus);
                        if(loadBarStatus == STATUS_INVALID_PARAMETER_1 || loadBarStatus == STATUS_INVALID_PARAMETER_2)
                        {
                            return STATUS_INVALID_PARAMETER;
                        }
                        return loadBarStatus;
                    }

                    ++regionIndex;
                    break;
                }
                default:
                    break;
            }
        }

        // Advance to the next full resource descriptor.
        list = reinterpret_cast<CM_FULL_RESOURCE_DESCRIPTOR*>(&list->PartialResourceList.PartialDescriptors[list->PartialResourceList.Count]);
    }

    m_Initialized = true;

    return STATUS_SUCCESS;
}

NTSTATUS GsMemoryManager::InitSegments(ULONGLONG vramSize) noexcept
{
    const ULONGLONG barSize = m_MappedBarMap.Region1.Length;

    const UINT64 cpuVisibleVramSize = min(barSize, vramSize);
    const UINT64 cpuInvisibleVramSize = barSize >= vramSize ? 0 : vramSize - barSize;

    LOG_DEBUG("cpuVisibleVramSize: 0x%016llX, vramSize: 0x%016llX, Region1 Length: 0x%016llX\n", cpuVisibleVramSize, vramSize, barSize);

    m_ActiveSegments = 0;

    if constexpr(EnableApertureSegment)
    {
        m_ApertureSegmentId = m_ActiveSegments++;
        GsSegment& descriptor = m_EmbeddedSegments[m_ApertureSegmentId];

        const SIZE_T size = static_cast<SIZE_T>(4) * vramSize;

        // Aperture Segment
        descriptor.BaseAddress.QuadPart = 0;
        descriptor.CpuTranslatedAddress.QuadPart = 0;
        descriptor.Size = size;
        descriptor.CommitLimit = size;

        descriptor.Flags.Value = 0;
        descriptor.Flags.Bits.Aperture = 1;
        descriptor.Flags.Bits.Agp = 0;
        descriptor.Flags.Bits.CpuVisible = 0;
        descriptor.Flags.Bits.UseBanking = 0;
        descriptor.Flags.Bits.CacheCoherent = 1;
        descriptor.Flags.Bits.PitchAlignment = 0;
        descriptor.Flags.Bits.PopulatedFromSystemMemory = 0;
        descriptor.Flags.Bits.PreservedDuringStandby = 1;
        descriptor.Flags.Bits.PreservedDuringHibernate = 1;
        descriptor.Flags.Bits.DirectFlip = 0;
        descriptor.Flags.Bits.Use64KBPages = 0;
        descriptor.Flags.Bits.SupportsCpuHostAperture = 0;
    }
    else
    {
        m_ApertureSegmentId = InvalidSegmentId;
    }

    if(cpuInvisibleVramSize > 0)
    {
        m_CpuInvisibleSegmentId = m_ActiveSegments++;
        GsSegment& descriptor = m_EmbeddedSegments[m_CpuInvisibleSegmentId];

        // CPU Invisible Segment
        // Start Invisible segment after Visible segment.
        descriptor.BaseAddress.QuadPart = static_cast<LONGLONG>(cpuVisibleVramSize);
        descriptor.CpuTranslatedAddress.QuadPart = 0;
        descriptor.Size = cpuInvisibleVramSize;
        descriptor.CommitLimit = descriptor.Size;

        descriptor.Flags.Value = 0;
        descriptor.Flags.Bits.Aperture = 0;
        descriptor.Flags.Bits.Agp = 0;
        descriptor.Flags.Bits.CpuVisible = 0;
        descriptor.Flags.Bits.UseBanking = 0;
        descriptor.Flags.Bits.CacheCoherent = 1;
        descriptor.Flags.Bits.PitchAlignment = 0;
        descriptor.Flags.Bits.PopulatedFromSystemMemory = 0;
        descriptor.Flags.Bits.PreservedDuringStandby = 1;
        descriptor.Flags.Bits.PreservedDuringHibernate = 1;
        descriptor.Flags.Bits.DirectFlip = 0;
        descriptor.Flags.Bits.Use64KBPages = 0;
        descriptor.Flags.Bits.SupportsCpuHostAperture = 1;
    }
    else
    {
        m_CpuInvisibleSegmentId = InvalidSegmentId;
    }

    if(cpuVisibleVramSize != 0)
    {
        m_CpuVisibleSegmentId = m_ActiveSegments++;
        GsSegment& descriptor = m_EmbeddedSegments[m_CpuVisibleSegmentId];

        // CPU Visible Segment
        descriptor.BaseAddress.QuadPart = 0;
        descriptor.CpuTranslatedAddress.QuadPart = static_cast<LONGLONG>(m_MappedBarMap.Region1.Start);
        descriptor.Size = cpuVisibleVramSize;
        descriptor.CommitLimit = descriptor.Size;

        descriptor.Flags.Value = 0;
        descriptor.Flags.Bits.Aperture = 0;
        descriptor.Flags.Bits.Agp = 0;
        descriptor.Flags.Bits.CpuVisible = 1;
        descriptor.Flags.Bits.UseBanking = 0;
        descriptor.Flags.Bits.CacheCoherent = 1;
        descriptor.Flags.Bits.PitchAlignment = 0;
        descriptor.Flags.Bits.PopulatedFromSystemMemory = 0;
        descriptor.Flags.Bits.PreservedDuringStandby = 1;
        descriptor.Flags.Bits.PreservedDuringHibernate = 1;
        descriptor.Flags.Bits.DirectFlip = 0;
        descriptor.Flags.Bits.Use64KBPages = 0;
        descriptor.Flags.Bits.SupportsCpuHostAperture = 0;
    }
    else
    {
        m_CpuVisibleSegmentId = InvalidSegmentId;
    }

    m_PagingBufferSegmentId = 0;
    m_PagingBufferSize = 65536;

    return STATUS_SUCCESS;
}

NTSTATUS GsMemoryManager::FillSegments(DXGK_QUERYSEGMENTOUT& querySegment) const noexcept
{
    // The paging buffer segment being 0 forces to use write-combined memory. Not sure what to put here yet.
    querySegment.PagingBufferSegmentId = m_PagingBufferSegmentId;
    // Not sure what to put here yet
    querySegment.PagingBufferSize = m_PagingBufferSize;

    if(!querySegment.pSegmentDescriptor)
    {
        querySegment.NbSegment = m_ActiveSegments;
        return STATUS_SUCCESS;
    }

    if(querySegment.NbSegment > m_ActiveSegments)
    {
        querySegment.NbSegment = m_ActiveSegments;
    }

    for(UINT i = 0; i < m_ActiveSegments; ++i)
    {
        DXGK_SEGMENTDESCRIPTOR& descriptorDest = querySegment.pSegmentDescriptor[i];
        const GsSegment& descriptorSrc = m_EmbeddedSegments[i];

        descriptorDest.CpuTranslatedAddress = descriptorSrc.CpuTranslatedAddress;
        descriptorDest.BaseAddress = descriptorSrc.BaseAddress;
        descriptorDest.Size = descriptorSrc.Size;
        descriptorDest.NbOfBanks = 0;
        descriptorDest.pBankRangeTable = nullptr;
        descriptorDest.CommitLimit = descriptorSrc.CommitLimit;
        descriptorDest.Flags.Value = 0;
        descriptorDest.Flags.Aperture = descriptorSrc.Flags.Bits.Aperture;
        descriptorDest.Flags.Agp = descriptorSrc.Flags.Bits.Agp;
        descriptorDest.Flags.CpuVisible = descriptorSrc.Flags.Bits.CpuVisible;
        descriptorDest.Flags.UseBanking = false;
        descriptorDest.Flags.CacheCoherent = descriptorSrc.Flags.Bits.CacheCoherent;
        descriptorDest.Flags.PitchAlignment = descriptorSrc.Flags.Bits.PitchAlignment;
        descriptorDest.Flags.PopulatedFromSystemMemory = descriptorSrc.Flags.Bits.PopulatedFromSystemMemory;
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
        descriptorDest.Flags.PreservedDuringStandby = descriptorSrc.Flags.Bits.PreservedDuringStandby;
        descriptorDest.Flags.PreservedDuringHibernate = descriptorSrc.Flags.Bits.PreservedDuringHibernate;
        descriptorDest.Flags.PartiallyPreservedDuringHibernate = descriptorSrc.Flags.Bits.PartiallyPreservedDuringHibernate;
        descriptorDest.Flags.DirectFlip = descriptorSrc.Flags.Bits.DirectFlip;
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_0)
        descriptorDest.Flags.Use64KBPages = descriptorSrc.Flags.Bits.Use64KBPages;
        descriptorDest.Flags.ReservedSysMem = descriptorSrc.Flags.Bits.ReservedSysMem;
        descriptorDest.Flags.SupportsCpuHostAperture = descriptorSrc.Flags.Bits.SupportsCpuHostAperture;
        descriptorDest.Flags.SupportsCachedCpuHostAperture = descriptorSrc.Flags.Bits.SupportsCachedCpuHostAperture;
        descriptorDest.Flags.ApplicationTarget = descriptorSrc.Flags.Bits.ApplicationTarget;
        descriptorDest.Flags.VprSupported = descriptorSrc.Flags.Bits.VprSupported;
        descriptorDest.Flags.VprPreservedDuringStandby = descriptorSrc.Flags.Bits.VprPreservedDuringStandby;
        descriptorDest.Flags.EncryptedPagingSupported = descriptorSrc.Flags.Bits.EncryptedPagingSupported;
        descriptorDest.Flags.LocalBudgetGroup = descriptorSrc.Flags.Bits.LocalBudgetGroup;
        descriptorDest.Flags.NonLocalBudgetGroup = descriptorSrc.Flags.Bits.NonLocalBudgetGroup;
#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WDDM2_9)
        descriptorDest.Flags.PopulatedByReservedDDRByFirmware = descriptorSrc.Flags.Bits.PopulatedByReservedDDRByFirmware;
#endif
#endif
#endif
    }

    return STATUS_SUCCESS;
}

NTSTATUS GsMemoryManager::Close() noexcept
{
    if(!m_Initialized)
    {
        return STATUS_SUCCESS;
    }

    if(m_DeviceBarMap.Region0.Bar != 0)
    {
        m_DxgkInterface.DxgkCbUnmapMemory(m_DxgkInterface.DeviceHandle, m_MappedBarMap.Region0.VirtualPointer);
    }

    if(m_DeviceBarMap.Region1.Bar != 0)
    {
        m_DxgkInterface.DxgkCbUnmapMemory(m_DxgkInterface.DeviceHandle, m_MappedBarMap.Region1.VirtualPointer);
    }

    if(m_DeviceBarMap.Region2.Bar != 0)
    {
        m_DxgkInterface.DxgkCbUnmapMemory(m_DxgkInterface.DeviceHandle, m_MappedBarMap.Region2.VirtualPointer);
    }

    if(m_DeviceBarMap.Region3.Bar != 0)
    {
        m_DxgkInterface.DxgkCbUnmapMemory(m_DxgkInterface.DeviceHandle, m_MappedBarMap.Region3.VirtualPointer);
    }

    if(m_DeviceBarMap.Region4.Bar != 0)
    {
        m_DxgkInterface.DxgkCbUnmapMemory(m_DxgkInterface.DeviceHandle, m_MappedBarMap.Region4.VirtualPointer);
    }

    if(m_DeviceBarMap.Region5.Bar != 0)
    {
        m_DxgkInterface.DxgkCbUnmapMemory(m_DxgkInterface.DeviceHandle, m_MappedBarMap.Region5.VirtualPointer);
    }

    m_Initialized = false;

    return STATUS_SUCCESS;
}

NTSTATUS GsMemoryManager::LoadRegionMap() noexcept
{
    if(m_DeviceId == 0x0001)
    {
        m_DeviceBarMap = Device0001ManufacturerFFFD;
    }
    else
    {
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS GsMemoryManager::LoadBar(CM_PARTIAL_RESOURCE_DESCRIPTOR& desc, const UINT regionIndex, const DXGKRNL_INTERFACE& dxgkInterface) noexcept
{
    // This function is only for handling CmResourceTypeMemory & CmResourceTypeMemoryLarge
    if(desc.Type != CmResourceTypeMemory && desc.Type != CmResourceTypeMemoryLarge)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // There are only 6 regions.
    if(regionIndex > 5)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // We don't use IO ports.
    if((desc.Flags & CM_RESOURCE_PORT_MEMORY) != CM_RESOURCE_PORT_MEMORY)
    {
        LOG_ERROR("CmResourceTypeMemory must be of type CM_RESOURCE_PORT_MEMORY.\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // The region info for this region index
    GsRegion region {};
    // The mapped region info that we will be setting.
    GsMappedRegion* mappedRegion = nullptr;

    // The number of valid regions we have to skip.
    UINT remainingSkips = regionIndex;

    // One could technically skip regions (say if they wanted to declaration to keep parity with the BARs)
    // So we have to iterate only through the valid regions, as our regionIndex is only relative the BAR info we receive.
    for(UINT i = 0; i < 6; ++i)
    {
        // Get the region info at the current index.
        switch(i)
        {
            case 0:
                region = m_DeviceBarMap.Region0;
                break;
            case 1:
                region = m_DeviceBarMap.Region1;
                break;
            case 2:
                region = m_DeviceBarMap.Region2;
                break;
            case 3:
                region = m_DeviceBarMap.Region3;
                break;
            case 4:
                region = m_DeviceBarMap.Region4;
                break;
            case 5:
                region = m_DeviceBarMap.Region5;
                break;
            default: break;
        }

        // Check if this region is in use.
        if(region.Bar == 0)
        {
            continue;
        }

        // Is this region the one represented by regionIndex.
        if(remainingSkips > 0)
        {
            --remainingSkips;
            continue;
        }

        // Grab a pointer to the mapped region.
        switch(i)
        {
            case 0:
                mappedRegion = &m_MappedBarMap.Region0;
                break;
            case 1:
                mappedRegion = &m_MappedBarMap.Region1;
                break;
            case 2:
                mappedRegion = &m_MappedBarMap.Region2;
                break;
            case 3:
                mappedRegion = &m_MappedBarMap.Region3;
                break;
            case 4:
                mappedRegion = &m_MappedBarMap.Region4;
                break;
            case 5:
                mappedRegion = &m_MappedBarMap.Region5;
                break;
            default: break;
        }

        // We have found the region we're looking for at this point and can exit the loop.
        break;
    }

    // If we failed to find the region.
    if(!mappedRegion)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // Ensure the R/W flag matches with what we expect.
    if(region.IsReadWrite)
    {
        if((desc.Flags & CM_RESOURCE_MEMORY_READ_WRITE) != CM_RESOURCE_MEMORY_READ_WRITE)
        {
            LOG_ERROR("BAR %d was not Read/Write.\n", region.Bar);
            return STATUS_MEMORY_NOT_ALLOCATED;
        }
    }
    else
    {
        if((desc.Flags & CM_RESOURCE_MEMORY_READ_WRITE) == CM_RESOURCE_MEMORY_READ_WRITE)
        {
            LOG_ERROR("BAR %d was Read/Write.\n", region.Bar);
            return STATUS_MEMORY_NOT_ALLOCATED;
        }
    }

    // Decode the memory structure. This is only needed for CmResourceTypeMemoryLarge, but also works for CmResourceTypeMemory
    mappedRegion->Length = RtlCmDecodeMemIoResource(&desc, &mappedRegion->Start);

    // When calling DXGK to map memory we need to pass a PHYSICAL_ADDRESS.
    PHYSICAL_ADDRESS startAddress = {
        .QuadPart = static_cast<LONGLONG>(mappedRegion->Start)
    };

    // Map the memory.
    const NTSTATUS mapStatus = dxgkInterface.DxgkCbMapMemory(
        dxgkInterface.DeviceHandle,               // DeviceHandle
        startAddress,                             // TranslatedAddress
        static_cast<ULONG>(mappedRegion->Length), // Length
        FALSE,                                    // InIoSpace
        FALSE,                                    // MapToUserMode
        region.IsCached ? MmCached : MmNonCached, // CacheType
        &mappedRegion->VirtualPointer             // VirtualAddress
    );

    if constexpr(EnableExtensiveLogging)
    {
        LOG_DEBUG("Memory Start: 0x%016llX, Length: 0x%016llX.\n", mappedRegion->Start, mappedRegion->Length);
    }

    // Report any error with memory mapping.
    if(!NT_SUCCESS(mapStatus))
    {
        LOG_ERROR("Failed to map BAR %d: 0x%08X.\n", region.Bar, mapStatus);
        return mapStatus;
    }

    return STATUS_SUCCESS;
}

