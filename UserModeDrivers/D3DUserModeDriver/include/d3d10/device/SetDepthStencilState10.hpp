#pragma once

#include "Common.hpp"

void APIENTRY GsSetDepthStencilState10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    const UINT StencilRef
);
