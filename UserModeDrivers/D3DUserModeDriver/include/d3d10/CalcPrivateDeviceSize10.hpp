#pragma once

#include "Common.hpp"

SIZE_T GsCalcPrivateDeviceSizeD3D10(
    const D3D10DDI_HADAPTER hAdapter,
    const D3D10DDIARG_CALCPRIVATEDEVICESIZE* const pCalcPrivateDeviceSize
);
