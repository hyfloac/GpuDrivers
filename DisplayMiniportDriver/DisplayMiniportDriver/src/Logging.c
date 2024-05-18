#include "Common.h"
#include <ntstrsafe.h>

#include "MemoryAllocator.h"
#include "Logging.h"

static NTSTATUS HyPrintBuffer(char* const logBuffer, const size_t bufferSize, const char* const level, const size_t levelLength, const void* const address, const char* const fmt, va_list args)
{
    (void) memcpy(logBuffer + 1, level, levelLength);

    size_t index = levelLength + 1;

    logBuffer[0] = '[';
    logBuffer[index++] = ']';
    logBuffer[index++] = '(';

    {
        // Insert the call site address.
        const NTSTATUS formatAddressStatus = RtlStringCbPrintfA(logBuffer + index, bufferSize - index, "0x%p", address);

        if(!NT_SUCCESS(formatAddressStatus))
        {
            return formatAddressStatus;
        }

        // Adjust index to the end of the address string.
        const NTSTATUS lengthStatus = RtlStringCbLengthA(logBuffer, bufferSize, &index);

        if(!NT_SUCCESS(lengthStatus))
        {
            // Ensure there is a null terminator just in case.
            logBuffer[bufferSize - 1] = '\0';

            return STATUS_SUCCESS;
        }
    }

    logBuffer[index++] = ')';
    logBuffer[index++] = ':';
    logBuffer[index++] = ' ';

    const NTSTATUS formatStatus = RtlStringCbVPrintfA(logBuffer + index, bufferSize - index, fmt, args);

    if(!NT_SUCCESS(formatStatus))
    {
        return formatStatus;
    }

    // Ensure there is a null terminator just in case.
    logBuffer[bufferSize - 1] = '\0';
    if(logBuffer[bufferSize - 3] != '\n')
    {
        logBuffer[bufferSize - 2] = '\n';
    }
    else
    {
        logBuffer[bufferSize - 2] = '\0';
    }

    return STATUS_SUCCESS;
}

static void HyLogAlloc(const char* const level, const size_t levelLength, const ULONG filterLevel, const void* const address, const char* const fmt, va_list args)
{
    const size_t bufferLength = 8192 * sizeof(char);

    char* logBuffer = HyAllocateZeroed(PagedPoolCacheAligned, bufferLength, POOL_TAG_LOGGING);

    (void) memset(logBuffer, '\0', bufferLength);

    {
        const NTSTATUS printResult = HyPrintBuffer(logBuffer, bufferLength, level, levelLength, address, fmt, args);
        if(!NT_SUCCESS(printResult)) // Just print whatever was formatted.
        {
            // return;
        }
    }

    DbgPrintEx(DPFLTR_DEFAULT_ID, filterLevel, "%s", logBuffer);

    HyDeallocate(logBuffer, POOL_TAG_LOGGING);
}

void HyLog(const char* const level, const size_t levelLength, const ULONG filterLevel, const void* const address, const char* const fmt, va_list args)
{
    char logBuffer[384];

    (void) memset(logBuffer, '\0', sizeof(logBuffer));

    {
        const NTSTATUS printResult = HyPrintBuffer(logBuffer, sizeof(logBuffer), level, levelLength, address, fmt, args);
        if(printResult == STATUS_BUFFER_OVERFLOW)
        {
            HyLogAlloc(level, levelLength, filterLevel, address, fmt, args);
            return;
        }

        if(!NT_SUCCESS(printResult)) // Just print whatever was formatted.
        {
            // return;
        }
    }
    
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, filterLevel, "%s", logBuffer);
}

const char* GetFileDeviceString(const DEVICE_TYPE deviceType)
{
#define CASE_FILE_DEVICE_STRING(A) #A
#define CASE_FILE_DEVICE_XSTRING(A) CASE_FILE_DEVICE_STRING(A)

#define CASE_FILE_DEVICE(DEV_TYPE) case DEV_TYPE: return CASE_FILE_DEVICE_XSTRING(DEV_TYPE)

    switch(deviceType)
    {
        case FILE_DEVICE_BEEP: return "FILE_DEVICE_BEEP";
        case FILE_DEVICE_CD_ROM: return "FILE_DEVICE_CD_ROM";
        case FILE_DEVICE_CD_ROM_FILE_SYSTEM: return "FILE_DEVICE_CD_ROM_FILE_SYSTEM";
        case FILE_DEVICE_CONTROLLER: return "FILE_DEVICE_CONTROLLER";
        case FILE_DEVICE_DATALINK: return "FILE_DEVICE_DATALINK";
        case FILE_DEVICE_DFS: return "FILE_DEVICE_DFS";
        case FILE_DEVICE_DISK: return "FILE_DEVICE_DISK";
        case FILE_DEVICE_DISK_FILE_SYSTEM: return "FILE_DEVICE_DISK_FILE_SYSTEM";
        case FILE_DEVICE_FILE_SYSTEM: return "FILE_DEVICE_FILE_SYSTEM";
        case FILE_DEVICE_INPORT_PORT: return "FILE_DEVICE_INPORT_PORT";
        case FILE_DEVICE_KEYBOARD: return "FILE_DEVICE_KEYBOARD";
        case FILE_DEVICE_MAILSLOT: return "FILE_DEVICE_MAILSLOT";
        case FILE_DEVICE_MIDI_IN: return "FILE_DEVICE_MIDI_IN";
        case FILE_DEVICE_MIDI_OUT: return "FILE_DEVICE_MIDI_OUT";
        case FILE_DEVICE_MOUSE: return "FILE_DEVICE_MOUSE";
        case FILE_DEVICE_MULTI_UNC_PROVIDER: return "FILE_DEVICE_MULTI_UNC_PROVIDER";
        case FILE_DEVICE_NAMED_PIPE: return "FILE_DEVICE_NAMED_PIPE";
        case FILE_DEVICE_NETWORK: return "FILE_DEVICE_NETWORK";
        case FILE_DEVICE_NETWORK_BROWSER: return "FILE_DEVICE_NETWORK_BROWSER";
        case FILE_DEVICE_NETWORK_FILE_SYSTEM: return "FILE_DEVICE_NETWORK_FILE_SYSTEM";
        case FILE_DEVICE_NULL: return "FILE_DEVICE_NULL";
        case FILE_DEVICE_PARALLEL_PORT: return "FILE_DEVICE_PARALLEL_PORT";
        case FILE_DEVICE_PHYSICAL_NETCARD: return "FILE_DEVICE_PHYSICAL_NETCARD";
        case FILE_DEVICE_PRINTER: return "FILE_DEVICE_PRINTER";
        case FILE_DEVICE_SCANNER: return "FILE_DEVICE_SCANNER";
        case FILE_DEVICE_SERIAL_MOUSE_PORT: return "FILE_DEVICE_SERIAL_MOUSE_PORT";
        case FILE_DEVICE_SERIAL_PORT: return "FILE_DEVICE_SERIAL_PORT";
        case FILE_DEVICE_SCREEN: return "FILE_DEVICE_SCREEN";
        case FILE_DEVICE_SOUND: return "FILE_DEVICE_SOUND";
        case FILE_DEVICE_STREAMS: return "FILE_DEVICE_STREAMS";
        case FILE_DEVICE_TAPE: return "FILE_DEVICE_TAPE";
        case FILE_DEVICE_TAPE_FILE_SYSTEM: return "FILE_DEVICE_TAPE_FILE_SYSTEM";
        case FILE_DEVICE_TRANSPORT: return "FILE_DEVICE_TRANSPORT";
        case FILE_DEVICE_UNKNOWN: return "FILE_DEVICE_UNKNOWN";
        case FILE_DEVICE_VIDEO: return "FILE_DEVICE_VIDEO";
        case FILE_DEVICE_VIRTUAL_DISK: return "FILE_DEVICE_VIRTUAL_DISK";
        case FILE_DEVICE_WAVE_IN: return "FILE_DEVICE_WAVE_IN";
        case FILE_DEVICE_WAVE_OUT: return "FILE_DEVICE_WAVE_OUT";
        case FILE_DEVICE_8042_PORT: return "FILE_DEVICE_8042_PORT";
        case FILE_DEVICE_NETWORK_REDIRECTOR: return "FILE_DEVICE_NETWORK_REDIRECTOR";
        case FILE_DEVICE_BATTERY: return "FILE_DEVICE_BATTERY";
        case FILE_DEVICE_BUS_EXTENDER: return "FILE_DEVICE_BUS_EXTENDER";
        case FILE_DEVICE_MODEM: return "FILE_DEVICE_MODEM";
        case FILE_DEVICE_VDM: return "FILE_DEVICE_VDM";
        case FILE_DEVICE_MASS_STORAGE: return "FILE_DEVICE_MASS_STORAGE";
        case FILE_DEVICE_SMB: return "FILE_DEVICE_SMB";
        case FILE_DEVICE_KS: return "FILE_DEVICE_KS";
        case FILE_DEVICE_CHANGER: return "FILE_DEVICE_CHANGER";
        case FILE_DEVICE_SMARTCARD: return "FILE_DEVICE_SMARTCARD";
        case FILE_DEVICE_ACPI: return "FILE_DEVICE_ACPI";
        case FILE_DEVICE_DVD: return "FILE_DEVICE_DVD";
        case FILE_DEVICE_FULLSCREEN_VIDEO: return "FILE_DEVICE_FULLSCREEN_VIDEO";
        case FILE_DEVICE_DFS_FILE_SYSTEM: return "FILE_DEVICE_DFS_FILE_SYSTEM";
        case FILE_DEVICE_DFS_VOLUME: return "FILE_DEVICE_DFS_VOLUME";
        case FILE_DEVICE_SERENUM: return "FILE_DEVICE_SERENUM";
        case FILE_DEVICE_TERMSRV: return "FILE_DEVICE_TERMSRV";
        case FILE_DEVICE_KSEC: return "FILE_DEVICE_KSEC";
        case FILE_DEVICE_FIPS: return "FILE_DEVICE_FIPS";
        case FILE_DEVICE_INFINIBAND: return "FILE_DEVICE_INFINIBAND";
        case FILE_DEVICE_VMBUS: return "FILE_DEVICE_VMBUS";
        case FILE_DEVICE_CRYPT_PROVIDER: return "FILE_DEVICE_CRYPT_PROVIDER";
        case FILE_DEVICE_WPD: return "FILE_DEVICE_WPD";
        case FILE_DEVICE_BLUETOOTH: return "FILE_DEVICE_BLUETOOTH";
        case FILE_DEVICE_MT_COMPOSITE: return "FILE_DEVICE_MT_COMPOSITE";
        case FILE_DEVICE_MT_TRANSPORT: return "FILE_DEVICE_MT_TRANSPORT";
        case FILE_DEVICE_BIOMETRIC: return "FILE_DEVICE_BIOMETRIC";
        case FILE_DEVICE_PMI: return "FILE_DEVICE_PMI";
        case FILE_DEVICE_EHSTOR: return "FILE_DEVICE_EHSTOR";
        case FILE_DEVICE_DEVAPI: return "FILE_DEVICE_DEVAPI";
        case FILE_DEVICE_GPIO: return "FILE_DEVICE_GPIO";
        case FILE_DEVICE_USBEX: return "FILE_DEVICE_USBEX";
        case FILE_DEVICE_CONSOLE: return "FILE_DEVICE_CONSOLE";
        case FILE_DEVICE_NFP: return "FILE_DEVICE_NFP";
        case FILE_DEVICE_SYSENV: return "FILE_DEVICE_SYSENV";
        case FILE_DEVICE_VIRTUAL_BLOCK: return "FILE_DEVICE_VIRTUAL_BLOCK";
        case FILE_DEVICE_POINT_OF_SERVICE: return "FILE_DEVICE_POINT_OF_SERVICE";
        case FILE_DEVICE_STORAGE_REPLICATION: return "FILE_DEVICE_STORAGE_REPLICATION";
        case FILE_DEVICE_TRUST_ENV: return "FILE_DEVICE_TRUST_ENV";
        case FILE_DEVICE_UCM: return "FILE_DEVICE_UCM";
        case FILE_DEVICE_UCMTCPCI: return "FILE_DEVICE_UCMTCPCI";
        case FILE_DEVICE_PERSISTENT_MEMORY: return "FILE_DEVICE_PERSISTENT_MEMORY";
        case FILE_DEVICE_NVDIMM: return "FILE_DEVICE_NVDIMM";
        case FILE_DEVICE_HOLOGRAPHIC: return "FILE_DEVICE_HOLOGRAPHIC";
        case FILE_DEVICE_SDFXHCI: return "FILE_DEVICE_SDFXHCI";
        case FILE_DEVICE_UCMUCSI: return "FILE_DEVICE_UCMUCSI";
        case FILE_DEVICE_PRM: return "FILE_DEVICE_PRM";
        case FILE_DEVICE_EVENT_COLLECTOR: return "FILE_DEVICE_EVENT_COLLECTOR";
        case FILE_DEVICE_USB4: return "FILE_DEVICE_USB4";
        case FILE_DEVICE_SOUNDWIRE: return "FILE_DEVICE_SOUNDWIRE";
        default: return "Unknown File Device";
    }

#undef CASE_FILE_DEVICE
#undef CASE_FILE_DEVICE_XSTRING
#undef CASE_FILE_DEVICE_STRING
}
