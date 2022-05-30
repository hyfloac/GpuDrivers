// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nf-fltkernel-fltcreatecommunicationport
#include <ntddk.h>
#include <fltKernel.h>

#include "FilterDisconnectNotify.h"
#include "FilterDeviceManager.h"

void HyFilterDisconnectNotify(PVOID ConnectionPortCookie)
{
    HyUnregisterFilterClient(ConnectionPortCookie);
}
