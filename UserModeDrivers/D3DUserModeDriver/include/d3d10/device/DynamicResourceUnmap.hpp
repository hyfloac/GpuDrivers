#pragma once

#include "Common.hpp"

void APIENTRY GsDynamicResourceUnmap(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HRESOURCE hResource,
    const UINT Subresource
);
