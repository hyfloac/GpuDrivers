#pragma once

#include "Common.h"

typedef struct _BLT_INFO
{
    PVOID pBits;
    UINT Pitch;
    UINT BitsPerPel;
    POINT Offset; // To unrotated top-left of dirty rects
    D3DKMDT_VIDPN_PRESENT_PATH_ROTATION Rotation;
    UINT Width; // For the unrotated image
    UINT Height; // For the unrotated image
} BLT_INFO;

void BltBits(BLT_INFO* pDst, const BLT_INFO* pSrc, UINT  NumRects, const RECT* pRects);


void BltBits_Dummy(BLT_INFO* pDst, const BLT_INFO* pSrc, const UINT Color, UINT  NumRects, const RECT* pRects);

