#pragma once

#include "Common.hpp"

void APIENTRY GsDynamicResourceMapDiscard10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HRESOURCE hResource,
    const UINT Subresource,
    const D3D10_DDI_MAP MapType,
    const UINT MapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* const pMappedSubresource
);
