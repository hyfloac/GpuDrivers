#pragma once

#include "Common.hpp"

void APIENTRY GsSetBlendState10(
    D3D10DDI_HDEVICE hDevice, 
    D3D10DDI_HBLENDSTATE hBlendState, 
    const FLOAT BlendFactor[4],
    UINT SampleMask
);
