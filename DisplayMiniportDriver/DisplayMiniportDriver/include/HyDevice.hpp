#pragma once

#include "Common.h"
#include "Objects.hpp"
#include "GsPresentManager.hpp"
#include "GsMemoryManager.hpp"

#pragma warning(push)
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4201) // anonymous unions warning

// Represents the current mode, may not always be set (i.e. frame buffer mapped) if representing the mode passed in on single mode setups.
struct HyDisplayMode final
{
    DEFAULT_CONSTRUCT_PU(HyDisplayMode);
    DEFAULT_DESTRUCT(HyDisplayMode);
    DELETE_CM(HyDisplayMode);
public:
    // The source mode currently set for HW Framebuffer.
    // For sample driver this info filled in StartDevice by the OS and never changed.
    DXGK_DISPLAY_INFORMATION DisplayInfo;

    // The rotation of the current mode. Rotation is performed in software during Present call.
    D3DKMDT_VIDPN_PRESENT_PATH_ROTATION Rotation;

    D3DKMDT_VIDPN_PRESENT_PATH_SCALING Scaling;

    UINT SrcModeWidth;
    UINT SrcModeHeight;
    
    // Various boolean flags the struct uses
    struct
    {
        UINT SourceNotVisible     : 1; // 0 if source is visible
        UINT FullscreenPresent    : 1; // 0 if should use dirty rects for present
        UINT FrameBufferIsActive  : 1; // 0 if not currently active (i.e. target not connected to source)
        UINT DoNotMapOrUnmap      : 1; // 1 if the FrameBuffer should not be (un)mapped during normal execution
        UINT VSyncEnabled         : 2; // 0 if not enabled. 1 if CRTC enabled. 2 if Display Only Enabled. 3 is invalid.
        UINT Unused               : 26;
    } Flags;

    // The start and end of physical memory known to be all zeroes. Used to optimize the BlackOutScreen function to not write
    // zeroes to memory already known to be zero. (Physical address is located in DispInfo)
    PHYSICAL_ADDRESS ZeroedOutStart;
    PHYSICAL_ADDRESS ZeroedOutEnd;

    void* FrameBufferPointer;

    // Current monitor power state
    DEVICE_POWER_STATE PowerState;

    // The PN Target ID. This is used for VSync interrupts.
    D3DDDI_VIDEO_PRESENT_TARGET_ID VidPnTargetId;
};

class HyMiniportDevice;

struct GsSynchronizeParams final
{
    HyMiniportDevice* Device;
    DXGKARGCB_NOTIFY_INTERRUPT_DATA InterruptData;
};

typedef struct HyPrivateDriverData
{
    static inline constexpr UINT HY_PRIVATE_DRIVER_DATA_MAGIC = 0x48794444;
    static inline constexpr UINT HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION = 1;

    // A magic value to verify that this is our data.
    UINT Magic; // HY_PRIVATE_DRIVER_DATA_MAGIC 0x48794444 HyDD
    // The size of this structure, this is an additional validation measure for checking that it is our data.
    UINT Size; // sizeof(HyPrivateDriverData)
    // An incremental version identifier, this is an additional validation measure for checking that we're communicating with our driver.
    UINT Version; // HY_PRIVATE_DRIVER_DATA_CURRENT_VERSION

    // The size of our GPU's pages.
    ULONGLONG PageSize;
} HyPrivateDriverData;

class HyMiniportDevice final
{
    DEFAULT_DESTRUCT(HyMiniportDevice);
    DELETE_CM(HyMiniportDevice);
public:
    static inline constexpr bool EnableApertureSegment = true;

    static inline constexpr UINT32 VALUE_REGISTER_MAGIC            = 0x4879666C;
    static inline constexpr UINT32 VALUE_REGISTER_REVISION         = 0x00000001;
    static inline constexpr UINT32 MASK_REGISTER_CONTROL           = 0x00000001;
    static inline constexpr UINT32 VALUE_REGISTER_EMULATION_MICROPROCESSOR = 0;
    static inline constexpr UINT32 VALUE_REGISTER_EMULATION_FPGA           = 1;
    static inline constexpr UINT32 VALUE_REGISTER_EMULATION_SIMULATION     = 2;
                                                                   
    static inline constexpr UINT32 REGISTER_MAGIC                  = 0x0000;
    static inline constexpr UINT32 REGISTER_REVISION               = 0x0004;
    static inline constexpr UINT32 REGISTER_EMULATION              = 0x0008;
    static inline constexpr UINT32 REGISTER_RESET                  = 0x000C;
    static inline constexpr UINT32 REGISTER_CONTROL                = 0x0010;
    static inline constexpr UINT32 REGISTER_VRAM_SIZE_LOW          = 0x0014;
    static inline constexpr UINT32 REGISTER_VRAM_SIZE_HIGH         = 0x0018;
    static inline constexpr UINT32 REGISTER_INTERRUPT_TYPE         = 0x001C;

    static inline constexpr UINT32 MSG_INTERRUPT_NONE              = 0x00000000;
    static inline constexpr UINT32 MSG_INTERRUPT_VSYNC_DISPLAY_0   = 0x00000010; // 0x10 - 0x18

    static inline constexpr UINT32 BASE_REGISTER_DI                = 0x2000;
    static inline constexpr UINT32 SIZE_REGISTER_DI                = 6 * 0x4;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_WIDTH        = 0x00;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_HEIGHT       = 0x04;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_BPP          = 0x08;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_ENABLE       = 0x0C;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_REFRESH_RATE_NUMERATOR   = 0x10;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_REFRESH_RATE_DENOMINATOR = 0x14;
    static inline constexpr UINT32 OFFSET_REGISTER_DI_VSYNC_ENABLE = 0x18;

    static inline constexpr UINT32 BASE_REGISTER_EDID              = 0x3000;
    static inline constexpr UINT32 SIZE_REGISTER_EDID              = 128;

    static inline constexpr UINT32 REGISTER_DEBUG_PRINT            = 0x8000;
    static inline constexpr UINT32 REGISTER_DEBUG_LOG_LOCK         = 0x8004;
    static inline constexpr UINT32 VALUE_DEBUG_LOG_LOCK_UNLOCKED   = 0x00000000;
    static inline constexpr UINT32 REGISTER_DEBUG_LOG_MULTI        = 0x8008;

    static inline constexpr UINT MaxViews    = 1;
    static inline constexpr UINT MaxChildren = 1;

    static inline constexpr UINT GPU_TYPE_SOFTWARE = 0;
    static inline constexpr UINT GPU_TYPE_SIMULATED = 1;
    static inline constexpr UINT GPU_TYPE_FPGA = 2;
    static inline constexpr UINT GPU_TYPE_MICROPROCESSOR = 3;
public:
    void* operator new(SIZE_T count);
    void operator delete(void* ptr);
public:
    static HyMiniportDevice* FromHandle(HANDLE hDevice) noexcept
    {
        return static_cast<HyMiniportDevice*>(hDevice);
    }
public:
    HyMiniportDevice(PDEVICE_OBJECT PhysicalDeviceObject) noexcept;

    NTSTATUS GetAdapterInfo() noexcept;

    NTSTATUS StartDevice(IN_PDXGK_START_INFO DxgkStartInfo, IN_PDXGKRNL_INTERFACE DxgkInterface, OUT_PULONG NumberOfVideoPresentSurfaces, OUT_PULONG NumberOfChildren) noexcept;
    NTSTATUS StopDevice() noexcept;

    BOOLEAN InterruptRoutine(IN_ULONG MessageNumber) noexcept;
    void DpcRoutine() noexcept;
    NTSTATUS QueryChildRelations(PDXGK_CHILD_DESCRIPTOR ChildRelations, ULONG ChildRelationsSize) noexcept;
    NTSTATUS QueryChildStatus(INOUT_PDXGK_CHILD_STATUS ChildStatus, IN_BOOLEAN NonDestructiveOnly) noexcept;
    NTSTATUS QueryDeviceDescriptor(IN_ULONG ChildUid, INOUT_PDXGK_DEVICE_DESCRIPTOR DeviceDescriptor) noexcept;
    NTSTATUS SetPowerState(IN_ULONG DeviceUid, IN_DEVICE_POWER_STATE DevicePowerState, IN_POWER_ACTION ActionType) noexcept;

    void ResetDevice() noexcept;

    NTSTATUS QueryAdapterInfo(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) noexcept;
    NTSTATUS CreateDevice(INOUT_PDXGKARG_CREATEDEVICE pCreateDevice) noexcept;

    NTSTATUS CollectDbgInfo(IN_CONST_PDXGKARG_COLLECTDBGINFO pCollectDbgInfo) noexcept;
    NTSTATUS IsSupportedVidPn(INOUT_PDXGKARG_ISSUPPORTEDVIDPN pIsSupportedVidPn) noexcept;
    NTSTATUS RecommendFunctionalVidPn(IN_CONST_PDXGKARG_RECOMMENDFUNCTIONALVIDPN_CONST pRecommendFunctionalVidPn) noexcept;
    NTSTATUS EnumVidPnCofuncModality(IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality) noexcept;
    NTSTATUS SetVidPnSourceAddress(IN_CONST_PDXGKARG_SETVIDPNSOURCEADDRESS pSetVidPnSourceAddress) noexcept;
    NTSTATUS SetVidPnSourceVisibility(IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility) noexcept;
    
    NTSTATUS CommitVidPn(IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn) noexcept;
    NTSTATUS UpdateActiveVidPnPresentPath(IN_CONST_PDXGKARG_UPDATEACTIVEVIDPNPRESENTPATH_CONST pUpdateActiveVidPnPresentPath) noexcept;
    NTSTATUS RecommendMonitorModes(IN_CONST_PDXGKARG_RECOMMENDMONITORMODES_CONST pRecommendMonitorModes) noexcept;

    NTSTATUS GetScanLine(INOUT_PDXGKARG_GETSCANLINE pGetScanLine) noexcept;

#if DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8
    NTSTATUS PresentDisplayOnly(IN_CONST_PDXGKARG_PRESENT_DISPLAYONLY pPresentDisplayOnly) noexcept;
#endif

    NTSTATUS StopDeviceAndReleasePostDisplayOwnership(IN_CONST_D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId, PDXGK_DISPLAY_INFORMATION DisplayInfo) noexcept;

    NTSTATUS ControlInterrupt(IN_CONST_DXGK_INTERRUPT_TYPE InterruptType, IN_BOOLEAN EnableInterrupt) noexcept;

    void ReportPresentProgress(D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId, BOOLEAN CompletedOrFailed) noexcept;

    volatile UINT* GetDeviceConfigRegister(const UINT registerAddress) const noexcept
    {
        return reinterpret_cast<UINT*>(&static_cast<BYTE*>(m_ConfigRegistersPointer)[registerAddress]);
    }

    UINT64 GetDeviceVramSize() const noexcept
    {
        const UINT lowSize = *GetDeviceConfigRegister(REGISTER_VRAM_SIZE_LOW);
        const UINT highSize = *GetDeviceConfigRegister(REGISTER_VRAM_SIZE_HIGH);

        return (static_cast<UINT64>(highSize) << 32ull) | static_cast<UINT64>(lowSize);
    }

    void SetDisplayState(UINT displayIndex, bool enable) const noexcept
    {
        if(displayIndex > 8)
        {
            return;
        }

        volatile UINT* const displayEnable = GetDeviceConfigRegister(BASE_REGISTER_DI + SIZE_REGISTER_DI * displayIndex + OFFSET_REGISTER_DI_ENABLE);

        // Disable Display
        *displayEnable = enable ? 1 : 0;
    }

    bool ObtainLogLock(const bool ReturnOnFailure) noexcept;
    void ReleaseLogLock() noexcept;

    void DebugLog(const char* String, const SIZE_T Length, const bool Lock = true) noexcept;
private:
    NTSTATUS CheckDevice() noexcept;
    NTSTATUS LoadPostDisplayInfo() noexcept;
    NTSTATUS CheckDevicePrefix(bool* const gpuTypeFound) noexcept;

    NTSTATUS FillUmDriverPrivate(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) const noexcept;
    NTSTATUS FillDriverCaps(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) const noexcept;
    NTSTATUS FillQuerySegment(IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo) const noexcept;

    NTSTATUS AddSingleSourceMode(const DXGK_VIDPNSOURCEMODESET_INTERFACE* const pVidPnSourceModeSetInterface, D3DKMDT_HVIDPNSOURCEMODESET hVidPnSourceModeSet, D3DDDI_VIDEO_PRESENT_SOURCE_ID SourceId) noexcept;
    NTSTATUS AddSingleTargetMode(const DXGK_VIDPNTARGETMODESET_INTERFACE* const pVidPnTargetModeSetInterface, D3DKMDT_HVIDPNTARGETMODESET hVidPnTargetModeSet, const D3DKMDT_VIDPN_SOURCE_MODE* pVidPnPinnedSourceModeInfo, D3DDDI_VIDEO_PRESENT_SOURCE_ID SourceId) noexcept;

    NTSTATUS SetSourceModeAndPath(const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode, const D3DKMDT_VIDPN_PRESENT_PATH* pPath) noexcept;

    NTSTATUS UnmapFrameBuffer(void* VirtualAddress, ULONG Length) noexcept;

    NTSTATUS AreVidPnPathFieldsValid(const D3DKMDT_VIDPN_PRESENT_PATH* pPath) const noexcept;
    NTSTATUS AreVidPnSourceModeFieldsValid(const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode) const noexcept;
private:
    static BOOLEAN SynchronizeVidSchNotifyInterrupt(PVOID Params) noexcept;
private:
    PDEVICE_OBJECT m_PhysicalDeviceObject;
    DXGK_START_INFO m_DxgkStartInfo;
    DXGKRNL_INTERFACE m_DxgkInterface;
    DXGK_DEVICE_INFO m_DeviceInfo;
    USHORT m_DeviceId;

    union
    {
        struct
        {
            UINT IsStarted : 1;
            UINT GpuType : 2; // 0 - Software Device (doesn't exist), 1 - simulated in VBox, 2 - FPGA, 2 - Full Hardware
            UINT Reserved : 30;
        };
        UINT Value;
    } m_Flags;

    ULONG m_PCIBusNumber;
    PCI_SLOT_NUMBER m_PCISlotNumber;
    PCI_COMMON_CONFIG m_PCIConfig;

    //DXGK_DISPLAY_INFORMATION PostDisplayInfo;

    void* m_ConfigRegistersPointer;

    HyDisplayMode m_CurrentDisplayMode[8];

    // Current adapter power state
    DEVICE_POWER_STATE m_AdapterPowerState;

    volatile LONG m_CurrentLogLockValue;

    GsPresentManager m_PresentManager;
    GsMemoryManager m_MemoryManager;
};

// #define HY_MINIPORT_DEVICE_FROM_HANDLE(HANDLE) static_cast<HyMiniportDevice*>(HANDLE)
#define HY_MINIPORT_DEVICE_FROM_HANDLE(HANDLE) HyMiniportDevice::FromHandle(HANDLE)

#pragma warning(pop)
