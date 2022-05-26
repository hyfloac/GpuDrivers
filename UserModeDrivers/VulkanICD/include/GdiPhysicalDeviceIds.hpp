#pragma once

#include <cstddef>
#include <NumTypes.hpp>

/**
 *   The proprietary structure returned from D3DKMTQueryAdapterInfo
 * with the query type set to KMTQAITYPE_PHYSICALADAPTERDEVICEIDS.
 */
struct PhysicalDeviceIds final
{
    /**
     *   An unknown value that is 0 for both my 3070 Ti, and the RDP vGPU
     * on my system.
     */
    u32 field0;
    /**
     *   The 4 hex digit value that represents the PCI-SIG supplied Vendor
     * ID.
     */
    u32 VendorID;
    /**
     * The 4 hex digit value that represents the vendor chosen Device ID.
     */
    u32 DeviceID;
    u32 SubsystemVendorID;
    u32 SubsystemID;
    u32 RevisionNumber;
    /**
     *   An unknown value that is 1 for my 3070 Ti, and 3 for the RDP vGPU
     * on my system.
     */
    u32 field6;
};

static_assert(sizeof(PhysicalDeviceIds) == 0x1C, "PhysicalDeviceIds does not meet the proprietary size requirements of dxgkrnl.sys.");
static_assert(offsetof(PhysicalDeviceIds, VendorID) == 4, "PhysicalDeviceIds::VendorID does not meet the proprietary offset requirements of dxgkrnl.sys.");
