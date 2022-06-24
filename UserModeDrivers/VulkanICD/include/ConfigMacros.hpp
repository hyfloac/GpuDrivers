#pragma once

#include <guiddef.h>

#ifndef DRIVER_DEBUG_LOG
  #ifdef _DEBUG
    #define DRIVER_DEBUG_LOG (1)
  #else
    #define DRIVER_DEBUG_LOG (0)
  #endif
#endif

#ifndef DRIVER_DUMMY_PUCK_FUNCTION
  #define DRIVER_DUMMY_PUCK_FUNCTION (01)
#endif

#ifndef DRIVER_DUMMY_DEVICE
  #define DRIVER_DUMMY_DEVICE (1)
#endif

// 3855de62-1330-4d17-a74a-e1e62d783448
// This has no inherent meaning, it's just the first value I got from https://www.uuidgenerator.net/ today.
static inline constexpr GUID DUMMY_ADAPTER_GUID {
    0x3855de62,
    0x1330,
    0x4d17,
    { 0xa7, 0x4a, 0xe1, 0xe6, 0x2d, 0x78, 0x34, 0x48 }
};

static inline constexpr LUID DUMMY_ADAPTER_LUID {
    0x3855de62,
    0x13304d17
};
