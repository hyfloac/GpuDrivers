#pragma once

#include "Common.hpp"

void APIENTRY GsCreateBlendState10(
    const D3D10DDI_HDEVICE hDevice, 
    const D3D10_DDI_BLEND_DESC* pBlendDesc, 
    const D3D10DDI_HBLENDSTATE hBlendState,
    const D3D10DDI_HRTBLENDSTATE hRtBlendState
);
