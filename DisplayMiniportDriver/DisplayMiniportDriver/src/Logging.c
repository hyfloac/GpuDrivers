#include <ntddk.h>
#include <ntstrsafe.h>

#include "MemoryAllocator.h"
#include "Logging.h"

static NTSTATUS HyPrintBuffer(char* const logBuffer, const size_t bufferSize, const char* const level, const size_t levelLength, const void* const address, const char* const fmt, va_list args)
{
    (void) memcpy(logBuffer + 1, level, levelLength);

    size_t index = levelLength;

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
        index = strlen(logBuffer);
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
    
    DbgPrintEx(DPFLTR_DEFAULT_ID, filterLevel, "%s", logBuffer);
}
