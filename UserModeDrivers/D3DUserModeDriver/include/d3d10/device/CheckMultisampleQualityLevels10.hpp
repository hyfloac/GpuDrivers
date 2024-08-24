#pragma once

#include "Common.hpp"

void APIENTRY GsCheckMultisampleQualityLevels10(
    const D3D10DDI_HDEVICE hDevice, 
    const DXGI_FORMAT Format,
    const UINT SampleCount, 
    UINT* const pNumQualityLevels
);
