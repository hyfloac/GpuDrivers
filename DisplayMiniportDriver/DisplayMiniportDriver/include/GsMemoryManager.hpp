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

class GsMemoryManager final
{
    DEFAULT_CONSTRUCT_PU(GsMemoryManager);
    DEFAULT_DESTRUCT(GsMemoryManager);
    DELETE_CM(GsMemoryManager);
public:
    static constexpr inline bool EnableExtensiveLogging = true;
public:
    NTSTATUS Init(const UINT16 deviceId, const DXGK_DEVICE_INFO& deviceInfo, const DXGKRNL_INTERFACE& dxgkInterface) noexcept;
    NTSTATUS Close() noexcept;

    [[nodiscard]] const GsBarMap& BarMap() const noexcept { return m_DeviceBarMap; }
    [[nodiscard]] const GsMappedBarMap& MappedBarMap() const noexcept { return m_MappedBarMap; }
private:
    NTSTATUS LoadRegionMap() noexcept;
    NTSTATUS LoadBar(CM_PARTIAL_RESOURCE_DESCRIPTOR& desc, const UINT regionIndex, const DXGKRNL_INTERFACE& dxgkInterface) noexcept;
private:
    UINT16 m_DeviceId;
    DXGKRNL_INTERFACE m_DxgkInterface;
    bool m_Initialized = false;
    GsBarMap m_DeviceBarMap;
    GsMappedBarMap m_MappedBarMap;
};
