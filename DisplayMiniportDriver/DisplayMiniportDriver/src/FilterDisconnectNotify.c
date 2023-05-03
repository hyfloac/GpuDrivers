// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nf-fltkernel-fltcreatecommunicationport
// #include <ntddk.h>
#include <fltKernel.h>

#include "FilterDisconnectNotify.h"
#include "FilterDeviceManager.h"
#include "Logging.h"

#pragma code_seg("PAGE")

void HyFilterDisconnectNotify(PVOID ConnectionPortCookie)
{
    PAGED_CODE();

    LOG_DEBUG("HyFilterDisconnectNotify\n");

    HyUnregisterFilterClient(ConnectionPortCookie);
}
