#pragma once

#include "Common.hpp"

void APIENTRY GsCreateDepthStencilState10(
    const D3D10DDI_HDEVICE hDevice, 
    const D3D10_DDI_DEPTH_STENCIL_DESC* const pDepthStencilDesc, 
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState, 
    const D3D10DDI_HRTDEPTHSTENCILSTATE hRtDepthStencilState
);
