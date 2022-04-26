#pragma once

#include <winnt.h>

#ifndef STATUS_BUFFER_TOO_SMALL
  #define STATUS_BUFFER_TOO_SMALL ((DWORD) 0xC0000023L)
#endif

#ifndef STATUS_DEVICE_REMOVED
  #define STATUS_DEVICE_REMOVED   ((DWORD) 0xC00002B6L)
#endif

#ifndef STATUS_SUCCESS
  #define STATUS_SUCCESS          ((DWORD) 0x00000000L)
#endif
