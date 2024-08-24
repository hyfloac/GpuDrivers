#pragma once

#include "Common.hpp"

HRESULT GsCreateDeviceD3D10(
    const D3D10DDI_HADAPTER hAdapter, 
    D3D10DDIARG_CREATEDEVICE* const pCreateDevice
);
