#ifndef GS_D3D_CONFIG_H
#define GS_D3D_CONFIG_H

#ifndef DEBUG
  #if defined(_DEBUG) && _DEBUG
    #define DEBUG 1
  #else
    #define DEBUG 0
  #endif
#endif

#ifndef GS_ENABLE_ENTRYPOINT_TRACING
  #define GS_ENABLE_ENTRYPOINT_TRACING (1 && DEBUG)
#endif

#ifndef GS_ENABLE_DEBUG_LOGGING
  #define GS_ENABLE_DEBUG_LOGGING (1 && DEBUG)
#endif

#endif
