// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nc-fltkernel-pflt_instance_setup_callback
#include <ntddk.h>
#include <fltKernel.h>

#include "FilterInstanceSetup.h"

#pragma code_seg("PAGE")

NTSTATUS HyFilterInstanceSetup(const PCFLT_RELATED_OBJECTS FltObjects, const FLT_INSTANCE_SETUP_FLAGS Flags, const DEVICE_TYPE VolumeDeviceType, const FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
    PAGED_CODE();

    // We're only going to accept manual connections.
    if((Flags & FLTFL_INSTANCE_SETUP_MANUAL_ATTACHMENT) == 0)
    {
        return STATUS_FLT_DO_NOT_ATTACH;
    }

    return STATUS_SUCCESS;
}

