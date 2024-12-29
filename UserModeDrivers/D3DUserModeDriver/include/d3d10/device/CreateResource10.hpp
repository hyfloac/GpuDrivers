#pragma once

#include "Common.hpp"

void APIENTRY GsCreateResource10(
    const D3D10DDI_HDEVICE hDevice, 
    const D3D10DDIARG_CREATERESOURCE* const pCreateResource, 
    const D3D10DDI_HRESOURCE hResource, 
    const D3D10DDI_HRTRESOURCE hRtResource
);
