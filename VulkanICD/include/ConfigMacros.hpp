#pragma once

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
