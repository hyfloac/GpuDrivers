// ReSharper disable CppClangTidyClangDiagnosticCastFunctionType
#include <Windows.h>
#include <d3dkmthk.h>

#include "GdiThunks.hpp"

#include "_Resharper.h"

PFND3DKMT_CREATEALLOCATION GDICreateAllocation = nullptr;
PFND3DKMT_DESTROYALLOCATION GDIDestroyAllocation = nullptr;
PFND3DKMT_SETALLOCATIONPRIORITY GDISetAllocationPriority = nullptr;
PFND3DKMT_QUERYALLOCATIONRESIDENCY GDIQueryAllocationResidency = nullptr;
PFND3DKMT_QUERYRESOURCEINFO GDIQueryResourceInfo = nullptr;
PFND3DKMT_OPENRESOURCE GDIOpenResource = nullptr;
PFND3DKMT_CREATEDEVICE GDICreateDevice = nullptr;
PFND3DKMT_DESTROYDEVICE GDIDestroyDevice = nullptr;
PFND3DKMT_QUERYADAPTERINFO GDIQueryAdapterInfo = nullptr;
PFND3DKMT_LOCK GDILock = nullptr;
PFND3DKMT_UNLOCK GDIUnlock = nullptr;
PFND3DKMT_GETDISPLAYMODELIST GDIGetDisplayModeList = nullptr;
PFND3DKMT_SETDISPLAYMODE GDISetDisplayMode = nullptr;
PFND3DKMT_GETMULTISAMPLEMETHODLIST GDIGetMultisampleMethodList = nullptr;
PFND3DKMT_PRESENT GDIPresent = nullptr;
PFND3DKMT_RENDER GDIRender = nullptr;
PFND3DKMT_OPENADAPTERFROMHDC GDIOpenAdapterFromHdc = nullptr;
PFND3DKMT_OPENADAPTERFROMDEVICENAME GDIOpenAdapterFromDeviceName = nullptr;
PFND3DKMT_CLOSEADAPTER GDICloseAdapter = nullptr;
PFND3DKMT_GETSHAREDPRIMARYHANDLE GDIGetSharedPrimaryHandle = nullptr;
PFND3DKMT_ESCAPE GDIEscape = nullptr;
PFND3DKMT_SETVIDPNSOURCEOWNER GDISetVidPnSourceOwner = nullptr;

PFND3DKMT_CREATEOVERLAY GDICreateOverlay = nullptr;
PFND3DKMT_UPDATEOVERLAY GDIUpdateOverlay = nullptr;
PFND3DKMT_FLIPOVERLAY GDIFlipOverlay = nullptr;
PFND3DKMT_DESTROYOVERLAY GDIDestroyOverlay = nullptr;
PFND3DKMT_WAITFORVERTICALBLANKEVENT GDIWaitForVerticalBlankEvent = nullptr;
PFND3DKMT_SETGAMMARAMP GDISetGammaRamp = nullptr;
PFND3DKMT_GETDEVICESTATE GDIGetDeviceState = nullptr;
PFND3DKMT_CREATEDCFROMMEMORY GDICreateDCFromMemory = nullptr;
PFND3DKMT_DESTROYDCFROMMEMORY GDIDestroyDCFromMemory = nullptr;
PFND3DKMT_SETCONTEXTSCHEDULINGPRIORITY GDISetContextSchedulingPriority = nullptr;
PFND3DKMT_GETCONTEXTSCHEDULINGPRIORITY GDIGetContextSchedulingPriority = nullptr;
PFND3DKMT_SETPROCESSSCHEDULINGPRIORITYCLASS GDISetProcessSchedulingPriorityClass = nullptr;
PFND3DKMT_GETPROCESSSCHEDULINGPRIORITYCLASS GDIGetProcessSchedulingPriorityClass = nullptr;
PFND3DKMT_RELEASEPROCESSVIDPNSOURCEOWNERS GDIReleaseProcessVidPnSourceOwners = nullptr;
PFND3DKMT_GETSCANLINE GDIGetScanLine = nullptr;
PFND3DKMT_POLLDISPLAYCHILDREN GDIPollDisplayChildren = nullptr;
PFND3DKMT_SETQUEUEDLIMIT GDISetQueuedLimit = nullptr;
PFND3DKMT_INVALIDATEACTIVEVIDPN GDIInvalidateActiveVidPn = nullptr;
PFND3DKMT_CHECKOCCLUSION GDICheckOcclusion = nullptr;
PFND3DKMT_GETPRESENTHISTORY GDIGetPresentHistory = nullptr;
PFND3DKMT_CREATECONTEXT GDICreateContext = nullptr;
PFND3DKMT_DESTROYCONTEXT GDIDestroyContext = nullptr;
PFND3DKMT_CREATESYNCHRONIZATIONOBJECT GDICreateSynchronizationObject = nullptr;
PFND3DKMT_DESTROYSYNCHRONIZATIONOBJECT GDIDestroySynchronizationObject = nullptr;
PFND3DKMT_WAITFORSYNCHRONIZATIONOBJECT GDIWaitForSynchronizationObject = nullptr;
PFND3DKMT_SIGNALSYNCHRONIZATIONOBJECT GDISignalSynchronizationObject = nullptr;
PFND3DKMT_CHECKMONITORPOWERSTATE GDICheckMonitorPowerState = nullptr;
PFND3DKMT_OPENADAPTERFROMGDIDISPLAYNAME GDIOpenAdapterFromGDIDisplayName = nullptr;
PFND3DKMT_CHECKEXCLUSIVEOWNERSHIP GDICheckExclusiveOwnership = nullptr;
PFND3DKMT_SETDISPLAYPRIVATEDRIVERFORMAT GDISetDisplayPrivateDriverFormat = nullptr;
PFND3DKMT_SHAREDPRIMARYLOCKNOTIFICATION GDISharedPrimaryLockNotification = nullptr;
PFND3DKMT_SHAREDPRIMARYUNLOCKNOTIFICATION GDISharedPrimaryUnLockNotification = nullptr;

PFND3DKMT_ENUMADAPTERS GDIEnumAdapters = nullptr;
PFND3DKMT_ENUMADAPTERS2 GDIEnumAdapters2 = nullptr;

bool InitKernelThunks() noexcept
{
    // Check if we're already initialized.
    if(GDICreateAllocation)
    {
        return true;
    }

    const HMODULE gdi32 = LoadLibraryA("gdi32.dll");
    
    if(!gdi32)
    {
        return false;
    }

    GDICreateAllocation = reinterpret_cast<PFND3DKMT_CREATEALLOCATION>(GetProcAddress(gdi32, "D3DKMTCreateAllocation"));
    GDIQueryResourceInfo = reinterpret_cast<PFND3DKMT_QUERYRESOURCEINFO>(GetProcAddress(gdi32, "D3DKMTQueryResourceInfo"));
    GDIOpenResource = reinterpret_cast<PFND3DKMT_OPENRESOURCE>(GetProcAddress(gdi32, "D3DKMTCreateAllocation"));
    GDIDestroyAllocation = reinterpret_cast<PFND3DKMT_DESTROYALLOCATION>(GetProcAddress(gdi32, "D3DKMTDestroyAllocation"));
    GDISetAllocationPriority = reinterpret_cast<PFND3DKMT_SETALLOCATIONPRIORITY>(GetProcAddress(gdi32, "D3DKMTSetAllocationPriority"));
    GDIQueryAllocationResidency = reinterpret_cast<PFND3DKMT_QUERYALLOCATIONRESIDENCY>(GetProcAddress(gdi32, "D3DKMTQueryAllocationResidency"));
    GDICreateDevice = reinterpret_cast<PFND3DKMT_CREATEDEVICE>(GetProcAddress(gdi32, "D3DKMTCreateDevice"));
    GDIDestroyDevice = reinterpret_cast<PFND3DKMT_DESTROYDEVICE>(GetProcAddress(gdi32, "D3DKMTDestroyDevice"));
    GDIQueryAdapterInfo = reinterpret_cast<PFND3DKMT_QUERYADAPTERINFO>(GetProcAddress(gdi32, "D3DKMTQueryAdapterInfo"));
    GDILock = reinterpret_cast<PFND3DKMT_LOCK>(GetProcAddress(gdi32, "D3DKMTLock"));
    GDIUnlock = reinterpret_cast<PFND3DKMT_UNLOCK>(GetProcAddress(gdi32, "D3DKMTUnlock"));
    GDIGetDisplayModeList = reinterpret_cast<PFND3DKMT_GETDISPLAYMODELIST>(GetProcAddress(gdi32, "D3DKMTGetDisplayModeList"));
    GDISetDisplayMode = reinterpret_cast<PFND3DKMT_SETDISPLAYMODE>(GetProcAddress(gdi32, "D3DKMTSetDisplayMode"));
    GDIGetMultisampleMethodList = reinterpret_cast<PFND3DKMT_GETMULTISAMPLEMETHODLIST>(GetProcAddress(gdi32, "D3DKMTGetMultisampleMethodList"));
    GDIPresent = reinterpret_cast<PFND3DKMT_PRESENT>(GetProcAddress(gdi32, "D3DKMTPresent"));
    GDIRender = reinterpret_cast<PFND3DKMT_RENDER>(GetProcAddress(gdi32, "D3DKMTRender"));
    GDIOpenAdapterFromHdc = reinterpret_cast<PFND3DKMT_OPENADAPTERFROMHDC>(GetProcAddress(gdi32, "D3DKMTOpenAdapterFromHdc"));
    GDIOpenAdapterFromDeviceName = reinterpret_cast<PFND3DKMT_OPENADAPTERFROMDEVICENAME>(GetProcAddress(gdi32, "D3DKMTOpenAdapterFromDeviceName"));
    GDICloseAdapter = reinterpret_cast<PFND3DKMT_CLOSEADAPTER>(GetProcAddress(gdi32, "D3DKMTCloseAdapter"));
    GDIGetSharedPrimaryHandle = reinterpret_cast<PFND3DKMT_GETSHAREDPRIMARYHANDLE>(GetProcAddress(gdi32, "D3DKMTGetSharedPrimaryHandle"));
    GDIEscape = reinterpret_cast<PFND3DKMT_ESCAPE>(GetProcAddress(gdi32, "D3DKMTEscape"));
    GDISetVidPnSourceOwner = reinterpret_cast<PFND3DKMT_SETVIDPNSOURCEOWNER>(GetProcAddress(gdi32, "D3DKMTSetVidPnSourceOwner"));
    GDIReleaseProcessVidPnSourceOwners = reinterpret_cast<PFND3DKMT_RELEASEPROCESSVIDPNSOURCEOWNERS>(GetProcAddress(gdi32, "D3DKMTReleaseProcessVidPnSourceOwners"));

    GDICreateOverlay = reinterpret_cast<PFND3DKMT_CREATEOVERLAY>(GetProcAddress(gdi32, "D3DKMTCreateOverlay"));
    GDIUpdateOverlay = reinterpret_cast<PFND3DKMT_UPDATEOVERLAY>(GetProcAddress(gdi32, "D3DKMTUpdateOverlay"));
    GDIFlipOverlay = reinterpret_cast<PFND3DKMT_FLIPOVERLAY>(GetProcAddress(gdi32, "D3DKMTFlipOverlay"));
    GDIDestroyOverlay = reinterpret_cast<PFND3DKMT_DESTROYOVERLAY>(GetProcAddress(gdi32, "D3DKMTDestroyOverlay"));
    GDIWaitForVerticalBlankEvent = reinterpret_cast<PFND3DKMT_WAITFORVERTICALBLANKEVENT>(GetProcAddress(gdi32, "D3DKMTWaitForVerticalBlankEvent"));
    GDISetGammaRamp = reinterpret_cast<PFND3DKMT_SETGAMMARAMP>(GetProcAddress(gdi32, "D3DKMTSetGammaRamp"));
    GDIGetDeviceState = reinterpret_cast<PFND3DKMT_GETDEVICESTATE>(GetProcAddress(gdi32, "D3DKMTGetDeviceState"));
    GDICreateDCFromMemory = reinterpret_cast<PFND3DKMT_CREATEDCFROMMEMORY>(GetProcAddress(gdi32, "D3DKMTCreateDCFromMemory"));
    GDIDestroyDCFromMemory = reinterpret_cast<PFND3DKMT_DESTROYDCFROMMEMORY>(GetProcAddress(gdi32, "D3DKMTDestroyDCFromMemory"));
    GDISetContextSchedulingPriority = reinterpret_cast<PFND3DKMT_SETCONTEXTSCHEDULINGPRIORITY>(GetProcAddress(gdi32, "D3DKMTSetContextSchedulingPriority"));
    GDIGetContextSchedulingPriority = reinterpret_cast<PFND3DKMT_GETCONTEXTSCHEDULINGPRIORITY>(GetProcAddress(gdi32, "D3DKMTGetContextSchedulingPriority"));
    GDISetProcessSchedulingPriorityClass = reinterpret_cast<PFND3DKMT_SETPROCESSSCHEDULINGPRIORITYCLASS>(GetProcAddress(gdi32, "D3DKMTSetProcessSchedulingPriorityClass"));
    GDIGetProcessSchedulingPriorityClass = reinterpret_cast<PFND3DKMT_GETPROCESSSCHEDULINGPRIORITYCLASS>(GetProcAddress(gdi32, "D3DKMTGetProcessSchedulingPriorityClass"));
    GDIGetScanLine = reinterpret_cast<PFND3DKMT_GETSCANLINE>(GetProcAddress(gdi32, "D3DKMTGetScanLine"));
    GDISetQueuedLimit = reinterpret_cast<PFND3DKMT_SETQUEUEDLIMIT>(GetProcAddress(gdi32, "D3DKMTSetQueuedLimit"));
    GDIPollDisplayChildren = reinterpret_cast<PFND3DKMT_POLLDISPLAYCHILDREN>(GetProcAddress(gdi32, "D3DKMTPollDisplayChildren"));
    GDIInvalidateActiveVidPn = reinterpret_cast<PFND3DKMT_INVALIDATEACTIVEVIDPN>(GetProcAddress(gdi32, "D3DKMTInvalidateActiveVidPn"));
    GDICheckOcclusion = reinterpret_cast<PFND3DKMT_CHECKOCCLUSION>(GetProcAddress(gdi32, "D3DKMTCheckOcclusion"));
    GDIGetPresentHistory = reinterpret_cast<PFND3DKMT_GETPRESENTHISTORY>(GetProcAddress(gdi32, "D3DKMTGetPresentHistory"));
    GDICreateContext = reinterpret_cast<PFND3DKMT_CREATECONTEXT>(GetProcAddress(gdi32, "D3DKMTCreateContext"));
    GDIDestroyContext = reinterpret_cast<PFND3DKMT_DESTROYCONTEXT>(GetProcAddress(gdi32, "D3DKMTDestroyContext"));
    GDICreateSynchronizationObject = reinterpret_cast<PFND3DKMT_CREATESYNCHRONIZATIONOBJECT>(GetProcAddress(gdi32, "D3DKMTCreateSynchronizationObject"));
    GDIDestroySynchronizationObject = reinterpret_cast<PFND3DKMT_DESTROYSYNCHRONIZATIONOBJECT>(GetProcAddress(gdi32, "D3DKMTDestroySynchronizationObject"));
    GDIWaitForSynchronizationObject = reinterpret_cast<PFND3DKMT_WAITFORSYNCHRONIZATIONOBJECT>(GetProcAddress(gdi32, "D3DKMTWaitForSynchronizationObject"));
    GDISignalSynchronizationObject = reinterpret_cast<PFND3DKMT_SIGNALSYNCHRONIZATIONOBJECT>(GetProcAddress(gdi32, "D3DKMTSignalSynchronizationObject"));
    GDICheckMonitorPowerState = reinterpret_cast<PFND3DKMT_CHECKMONITORPOWERSTATE>(GetProcAddress(gdi32, "D3DKMTCheckMonitorPowerState"));
    GDIOpenAdapterFromGDIDisplayName = reinterpret_cast<PFND3DKMT_OPENADAPTERFROMGDIDISPLAYNAME>(GetProcAddress(gdi32, "D3DKMTOpenAdapterFromGdiDisplayName"));
    GDICheckExclusiveOwnership = reinterpret_cast<PFND3DKMT_CHECKEXCLUSIVEOWNERSHIP>(GetProcAddress(gdi32, "D3DKMTCheckExclusiveOwnership"));
    GDISetDisplayPrivateDriverFormat = reinterpret_cast<PFND3DKMT_SETDISPLAYPRIVATEDRIVERFORMAT>(GetProcAddress(gdi32, "D3DKMTSetDisplayPrivateDriverFormat"));
    GDISharedPrimaryLockNotification = reinterpret_cast<PFND3DKMT_SHAREDPRIMARYLOCKNOTIFICATION>(GetProcAddress(gdi32, "D3DKMTSharedPrimaryLockNotification"));
    GDISharedPrimaryUnLockNotification = reinterpret_cast<PFND3DKMT_SHAREDPRIMARYUNLOCKNOTIFICATION>(GetProcAddress(gdi32, "D3DKMTSharedPrimaryUnLockNotification"));

    GDIEnumAdapters = reinterpret_cast<PFND3DKMT_ENUMADAPTERS>(GetProcAddress(gdi32, "D3DKMTEnumAdapters"));
    GDIEnumAdapters2 = reinterpret_cast<PFND3DKMT_ENUMADAPTERS2>(GetProcAddress(gdi32, "D3DKMTEnumAdapters2"));

    if(!GDICreateAllocation ||
       !GDIQueryResourceInfo ||
       !GDIOpenResource ||
       !GDIDestroyAllocation ||
       !GDISetAllocationPriority ||
       !GDIQueryAllocationResidency ||
       !GDICreateDevice ||
       !GDIDestroyDevice ||
       !GDIQueryAdapterInfo ||
       !GDILock ||
       !GDIUnlock ||
       !GDIGetDisplayModeList ||
       !GDISetDisplayMode ||
       !GDIGetMultisampleMethodList ||
       !GDIPresent ||
       !GDIRender ||
       !GDIOpenAdapterFromHdc ||
       !GDIOpenAdapterFromDeviceName ||
       !GDICloseAdapter ||
       !GDIGetSharedPrimaryHandle ||
       !GDIEscape ||
       !GDISetVidPnSourceOwner ||
       !GDICreateOverlay ||
       !GDIUpdateOverlay ||
       !GDIFlipOverlay ||
       !GDIDestroyOverlay ||
       !GDIWaitForVerticalBlankEvent ||
       !GDISetGammaRamp ||
       !GDIGetDeviceState ||
       !GDICreateDCFromMemory ||
       !GDIDestroyDCFromMemory ||
       !GDISetContextSchedulingPriority ||
       !GDIGetContextSchedulingPriority ||
       !GDISetProcessSchedulingPriorityClass ||
       !GDIGetProcessSchedulingPriorityClass ||
       !GDIReleaseProcessVidPnSourceOwners ||
       !GDIGetScanLine ||
       !GDISetQueuedLimit ||
       !GDIPollDisplayChildren ||
       !GDIInvalidateActiveVidPn ||
       !GDICheckOcclusion ||
       !GDICreateContext ||
       !GDIDestroyContext ||
       !GDICreateSynchronizationObject ||
       !GDIDestroySynchronizationObject ||
       !GDIWaitForSynchronizationObject ||
       !GDISignalSynchronizationObject ||
       !GDICheckMonitorPowerState ||
       !GDIOpenAdapterFromGDIDisplayName ||
       !GDICheckExclusiveOwnership ||
       !GDISetDisplayPrivateDriverFormat ||
       !GDISharedPrimaryLockNotification ||
       !GDISharedPrimaryUnLockNotification ||
       !GDIGetPresentHistory ||
       !GDIEnumAdapters)
    {
        return false;
    }

    return true;
}
