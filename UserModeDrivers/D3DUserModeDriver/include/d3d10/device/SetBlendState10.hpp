#pragma once

#include "Common.hpp"

void APIENTRY GsSetBlendState10(
    const D3D10DDI_HDEVICE hDevice, 
    const D3D10DDI_HBLENDSTATE hBlendState, 
    const FLOAT BlendFactor[4],
    const UINT SampleMask
);
