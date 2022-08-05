#include <ntddk.h>
#include <ntstrsafe.h>

#include "Logging.h"

void HyLog(const char* level, const size_t levelLength, const void* const address, const char* const fmt, va_list args)
{
    char logBuffer[384];

    (void) memset(logBuffer, '\0', sizeof(logBuffer));
    (void) memcpy(logBuffer + 1, level, levelLength);

    size_t index = levelLength + 1;

    logBuffer[0] = '[';
    logBuffer[index++] = ']';
    logBuffer[index++] = '(';

    {
        // Insert the call site address.
        const NTSTATUS formatAddressStatus = RtlStringCbPrintfA(logBuffer + index, sizeof(logBuffer) - index, "0x%p", address);

        if(!NT_SUCCESS(formatAddressStatus))
        {
            return;
        }

        // Adjust index to the end of the address string.
        index = strlen(logBuffer + index) + index;
    }

    logBuffer[index++] = ')';
    logBuffer[index++] = ':';
    logBuffer[index++] = ' ';

    const NTSTATUS formatStatus = RtlStringCbVPrintfA(logBuffer + index, sizeof(logBuffer) - index, fmt, args);

    if(!NT_SUCCESS(formatStatus))
    {
        return;
    }

    // Ensure there is a null terminator just in case.
    logBuffer[sizeof(logBuffer) - 1] = '\0';

    DbgPrint("%s", logBuffer);
}
