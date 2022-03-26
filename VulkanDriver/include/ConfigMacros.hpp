#pragma once

#ifndef DRIVER_DEBUG_LOG
  #ifdef _DEBUG
    #define DRIVER_DEBUG_LOG (1)
  #else
    #define DRIVER_DEBUG_LOG (0)
  #endif
#endif
