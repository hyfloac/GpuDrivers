// ReSharper disable CppCStyleCast

#ifdef __cplusplus
extern "C" {
#endif

#include "RegisterMemCopy.h"

BOOLEAN RegisterMemCopyVV32(volatile void* const Destination, const volatile void* const Source, const UINT64 ByteCount)
{
    const UINT_PTR destinationAddress = (UINT_PTR) Destination;
    const UINT_PTR sourceAddress = (UINT_PTR) Source;

    volatile UINT32* const Destination32 = Destination;
    const volatile UINT32* const Source32 = Source;

    if(destinationAddress % sizeof(UINT32) == 0 && sourceAddress % sizeof(UINT32) == 0)
    {
        const UINT64 dwordCount = ByteCount / sizeof(UINT32);
        const UINT64 slipCount = ByteCount % sizeof(UINT32);

        if(slipCount != 0)
        {
            return FALSE;
        }

        for(UINT64 i = 0; i < dwordCount; ++i)
        {
            Destination32[i] = Source32[i];
        }

        return TRUE;
    }

    return FALSE;
}

BOOLEAN RegisterMemCopyNV32(void* const Destination, const volatile void* const Source, const UINT64 ByteCount)
{
    const UINT_PTR destinationAddress = (UINT_PTR) Destination;
    const UINT_PTR sourceAddress = (UINT_PTR) Source;

    UINT8* const Destination8 = (UINT8*) Destination;
    const volatile UINT8* const Source8 = (const volatile UINT8*) Source;

    UINT32* const Destination32 = (UINT32*) Destination;
    const volatile UINT32* Source32 = (const volatile UINT32*) Source;

    if(destinationAddress % sizeof(UINT32) == 0 && sourceAddress % sizeof(UINT32) == 0)
    {
        const UINT64 dwordCount = ByteCount / sizeof(UINT32);
        const UINT64 slipCount = ByteCount % sizeof(UINT32);

        for(UINT64 i = 0; i < dwordCount; ++i)
        {
            Destination32[i] = Source32[i];
        }

        if(slipCount != 0)
        {
            const UINT32 finalDword = Source32[dwordCount];
            const UINT8* finalBytes = (const UINT8*) &finalDword;

            for(UINT64 i = 0; i < slipCount; ++i)
            {
                Destination8[dwordCount * sizeof(UINT32) + i] = finalBytes[i];
            }
        }

        return TRUE;
    }
    else
    {
        const UINT64 dwordCount = ByteCount / sizeof(UINT32);
        const UINT64 slipCount = ByteCount % sizeof(UINT32);
        UINT8 slipBuffer[8];
        UINT64 slipOffset = 0;

        const UINT64 sourceAddressPreSlip = sourceAddress % sizeof(UINT32);

        if(sourceAddressPreSlip != 0)
        {
            const UINT32 preDword = *((const volatile UINT32*) (Source8 - sourceAddressPreSlip));
            RtlCopyMemory(slipBuffer, ((const UINT8*) &preDword) + sourceAddressPreSlip, sizeof(UINT32) - sourceAddressPreSlip);
            slipOffset = sizeof(UINT32) - sourceAddressPreSlip;
            Source32 = (const volatile UINT32*) (Source8 + slipOffset);
        }

        for(UINT64 i = 0; i < dwordCount; ++i)
        {
            if(slipOffset < 4)
            {
                const UINT32 dword = Source32[i];
                RtlCopyMemory(&slipBuffer[slipOffset], &dword, sizeof(UINT32));
                slipOffset += 4;
            }

            RtlCopyMemory(&Destination32[i], slipBuffer, sizeof(UINT32));
            slipOffset -= 4;
            RtlMoveMemory(slipBuffer, &slipBuffer[slipOffset], 8 - slipOffset);
        }

        if(slipCount != 0)
        {
            const UINT32 finalDword = Source32[dwordCount];
            RtlCopyMemory(&slipBuffer[slipOffset], &finalDword, sizeof(UINT32));
            slipOffset += sizeof(UINT32);

            for(UINT64 i = 0; i < slipCount; ++i)
            {
                Destination8[dwordCount * sizeof(UINT32) + i] = slipBuffer[i];
            }
        }

        return TRUE;
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif
