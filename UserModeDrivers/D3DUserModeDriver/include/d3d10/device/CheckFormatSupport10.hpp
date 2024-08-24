#pragma once

#include "Common.hpp"

void APIENTRY GsCheckFormatSupport10(
    const D3D10DDI_HDEVICE hDevice,
    const DXGI_FORMAT Format, 
    UINT* const pFormatSupport
);
