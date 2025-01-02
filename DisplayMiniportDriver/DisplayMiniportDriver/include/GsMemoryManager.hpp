#pragma once

#include "Common.h"
#include "Objects.hpp"
#include "Devices.hpp"

struct GsMappedRegion final
{
    DEFAULT_CONSTRUCT_PU(GsMappedRegion);
    DEFAULT_DESTRUCT(GsMappedRegion);
    DEFAULT_CM_PU(GsMappedRegion);
public:
    ULONGLONG Start;
    ULONGLONG Length;
    void* VirtualPointer;
};

struct GsMappedBarMap final
{
    DEFAULT_CONSTRUCT_PU(GsMappedBarMap);
    DEFAULT_DESTRUCT(GsMappedBarMap);
    DEFAULT_CM_PU(GsMappedBarMap);
public:
    GsMappedRegion Region0;
    GsMappedRegion Region1;
    GsMappedRegion Region2;
    GsMappedRegion Region3;
    GsMappedRegion Region4;
    GsMappedRegion Region5;
};

union GsSegmentFlags final
{
    DEFAULT_CONSTRUCT_PU(GsSegmentFlags);
    DEFAULT_DESTRUCT(GsSegmentFlags);
    DEFAULT_CM_PU(GsSegmentFlags);
public:
    struct Bits
    {
        UINT Aperture : 1;                          // 0x00000001
        UINT Agp : 1;                               // 0x00000002
        UINT CpuVisible : 1;                        // 0x00000004
        UINT UseBanking : 1;                        // 0x00000008
        UINT CacheCoherent : 1;                     // 0x00000010
        UINT PitchAlignment : 1;                    // 0x00000020
        UINT PopulatedFromSystemMemory : 1;         // 0x00000040
        UINT PreservedDuringStandby : 1;            // 0x00000080
        UINT PreservedDuringHibernate : 1;          // 0x00000100
        UINT PartiallyPreservedDuringHibernate : 1; // 0x00000200
        UINT DirectFlip : 1;                        // 0x00000400
        UINT Use64KBPages : 1;                      // 0x00000800 // Defines if the segment is using 4GB or 64 KB pages
        UINT ReservedSysMem : 1;                    // 0x00001000 // Reserved for system use
        UINT SupportsCpuHostAperture : 1;           // 0x00002000 // True if segment supports a CpuHostAperture
        UINT SupportsCachedCpuHostAperture : 1;     // 0x00004000 // True if segment supports cache coherent CpuHostAperture
        UINT ApplicationTarget : 1;                 // 0x00008000 // Deprecated, replaced by LocalBudgetGroup and NonLocalBudgetGroup flags.
        UINT VprSupported : 1;                      // 0x00010000 // Video Protected Region supported
        UINT VprPreservedDuringStandby : 1;         // 0x00020000 // Content of Video Protected Regions preserved during standby
        UINT EncryptedPagingSupported : 1;          // 0x00040000 // Hardware protected allocation are encrypted during paging
        UINT LocalBudgetGroup : 1;                  // 0x00080000 // This segment counts against local memory segment budget group.
        UINT NonLocalBudgetGroup : 1;               // 0x00100000 // This segment counts against non-local memory segment budget group.
        UINT PopulatedByReservedDDRByFirmware : 1;  // 0x00200000 // This segment is populated from reserved system memory by the firmware.
        UINT Reserved : 10;                         // 0xFFC00000
    } Bits;
    UINT Value;
};

struct GsSegment final
{
    DEFAULT_CONSTRUCT_PU(GsSegment);
    DEFAULT_DESTRUCT(GsSegment);
    DEFAULT_CM_PU(GsSegment);
public:
    // GPU logical base address for the segment.
    PHYSICAL_ADDRESS BaseAddress;
    // Size of the segment.
    SIZE_T Size;
    // CPU translated base address for the segment if CPU visible.
    PHYSICAL_ADDRESS CpuTranslatedAddress;
    // Maximum number of bytes that can be commited to this segment, apply to aperture segment only.
    SIZE_T CommitLimit;
    // Segment bit field flags
    GsSegmentFlags Flags;
};

class GsMemoryManager final
{
    DEFAULT_CONSTRUCT_PU(GsMemoryManager);
    DEFAULT_DESTRUCT(GsMemoryManager);
    DELETE_CM(GsMemoryManager);
public:
    static inline constexpr bool EnableExtensiveLogging = true;
    static inline constexpr bool EnableApertureSegment = true;

    static inline constexpr UINT InvalidSegmentId = 0xFFFFFFFF;
public:
    NTSTATUS Init(const UINT16 deviceId, const DXGK_DEVICE_INFO& deviceInfo, const DXGKRNL_INTERFACE& dxgkInterface) noexcept;
    NTSTATUS InitSegments(const ULONGLONG vramSize) noexcept;
    NTSTATUS Close() noexcept;

    NTSTATUS FillSegments(DXGK_QUERYSEGMENTOUT& querySegment) const noexcept;

    [[nodiscard]] const GsBarMap& BarMap() const noexcept { return m_DeviceBarMap; }
    [[nodiscard]] const GsMappedBarMap& MappedBarMap() const noexcept { return m_MappedBarMap; }

    [[nodiscard]] UINT ActiveSegments() const noexcept { return m_ActiveSegments; }
    [[nodiscard]] const GsSegment* EmbeddedSegments() const noexcept { return m_EmbeddedSegments; }

    [[nodiscard]] UINT ApertureSegmentId() const noexcept { return m_ApertureSegmentId; }
    [[nodiscard]] UINT CpuInvisibleSegmentId() const noexcept { return m_CpuInvisibleSegmentId; }
    [[nodiscard]] UINT CpuVisibleSegmentId() const noexcept { return m_CpuVisibleSegmentId; }

    [[nodiscard]] UINT PagingBufferSegmentId() const noexcept { return m_PagingBufferSegmentId; }
    [[nodiscard]] UINT PagingBufferSize() const noexcept { return m_PagingBufferSize; }
private:
    NTSTATUS LoadRegionMap() noexcept;
    NTSTATUS LoadBar(CM_PARTIAL_RESOURCE_DESCRIPTOR& desc, const UINT regionIndex, const DXGKRNL_INTERFACE& dxgkInterface) noexcept;
private:
    UINT16 m_DeviceId;
    DXGKRNL_INTERFACE m_DxgkInterface;
    bool m_Initialized = false;
    GsBarMap m_DeviceBarMap;
    GsMappedBarMap m_MappedBarMap;

    UINT m_ActiveSegments;
    GsSegment m_EmbeddedSegments[8];

    UINT m_ApertureSegmentId;
    UINT m_CpuInvisibleSegmentId;
    UINT m_CpuVisibleSegmentId;

    UINT m_PagingBufferSegmentId;
    UINT m_PagingBufferSize;
};
