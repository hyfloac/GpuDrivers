// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nf-fltkernel-fltcreatecommunicationport
#include <ntddk.h>
#include <fltKernel.h>

#include "FilterConnectNotify.h"
#include "FilterDeviceManager.h"

NTSTATUS HyFilterConnectNotify(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID* ConnectionPortCookie)
{
    PAGED_CODE();

    // Store the ClientPort and return the cookie.
    const NTSTATUS registerClientStatus = HyRegisterFilterClient(ClientPort, ConnectionPortCookie);

    // Propagate errors.
    if(!NT_SUCCESS(registerClientStatus))
    {
        // Adjust the invalid parameter index if necessary.
        if(registerClientStatus == STATUS_INVALID_PARAMETER_2)
        {
            return STATUS_INVALID_PARAMETER_5;
        }

        return registerClientStatus;
    }

    return STATUS_SUCCESS;
}
