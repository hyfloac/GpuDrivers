#ifndef DRIVER_VERSION_H
#define DRIVER_VERSION_H

#include "Config.h"

#ifdef VER_PRODUCTVERSION
  #undef VER_PRODUCTVERSION
#endif

#ifdef VER_PRODUCTVERSION_STR
  #undef VER_PRODUCTVERSION_STR
#endif


#define GS_DXGKDDI_IV_VISTA      0x1052
#define GS_DXGKDDI_IV_VISTA_SP1  0x1053
#define GS_DXGKDDI_IV_WIN7       0x2005
#define GS_DXGKDDI_IV_WIN8       0x300E
#define GS_DXGKDDI_IV_WDDM1_3    0x4002
#define GS_DXGKDDI_IV_WDDM2_0    0x5023
#define GS_DXGKDDI_IV_WDDM2_1    0x6003
#define GS_DXGKDDI_IV_WDDM2_1_5  0x6010     // Used in RS1.7 for GPU-P
#define GS_DXGKDDI_IV_WDDM2_1_6  0x6011     // Used in RS1.8 for GPU-P
#define GS_DXGKDDI_IV_WDDM2_2    0x700A
#define GS_DXGKDDI_IV_WDDM2_3    0x8001
#define GS_DXGKDDI_IV_WDDM2_4    0x9006
#define GS_DXGKDDI_IV_WDDM2_5    0xA00B
#define GS_DXGKDDI_IV_WDDM2_6    0xB004
#define GS_DXGKDDI_IV_WDDM2_7    0xC004
#define GS_DXGKDDI_IV_WDDM2_8    0xD001
#define GS_DXGKDDI_IV_WDDM2_9    0xE003
#define GS_DXGKDDI_IV_WDDM3_0    0xF003


#if (DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM3_0)
  #define WDDM_MAJOR_VERSION 30
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_9
  #define WDDM_MAJOR_VERSION 29
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_8
  #define WDDM_MAJOR_VERSION 28
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_7
  #define WDDM_MAJOR_VERSION 27
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_6
  #define WDDM_MAJOR_VERSION 26
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_5
  #define WDDM_MAJOR_VERSION 25
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_4
  #define WDDM_MAJOR_VERSION 24
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_3
  #define WDDM_MAJOR_VERSION 23
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_2
  #define WDDM_MAJOR_VERSION 22
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_1
  #define WDDM_MAJOR_VERSION 21
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM2_0
  #define WDDM_MAJOR_VERSION 20
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WDDM1_3
  #define WDDM_MAJOR_VERSION 10
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WIN8
  #define WDDM_MAJOR_VERSION 9
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WIN7
  #define WDDM_MAJOR_VERSION 8
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_VISTA
  #define WDDM_MAJOR_VERSION 7
#endif

#if DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WIN8
  #if D3D_FEATURE_LEVEL_MAJOR >= 12 && D3D_FEATURE_LEVEL_MINOR >= 2
    #define WDDM_MINOR_VERSION 21
  #elif D3D_FEATURE_LEVEL_MAJOR >= 12 && D3D_FEATURE_LEVEL_MINOR >= 1
    #define WDDM_MINOR_VERSION 20
  #elif D3D_FEATURE_LEVEL_MAJOR >= 12
    #define WDDM_MINOR_VERSION 19
  #elif D3D_FEATURE_LEVEL_MAJOR >= 11 && D3D_FEATURE_LEVEL_MINOR >= 1
    #define WDDM_MINOR_VERSION 18
  #elif D3D_FEATURE_LEVEL_MAJOR >= 11
    #define WDDM_MINOR_VERSION 17
  #elif D3D_FEATURE_LEVEL_MAJOR >= 10 && D3D_FEATURE_LEVEL_MINOR >= 1
    #define WDDM_MINOR_VERSION 16
  #elif D3D_FEATURE_LEVEL_MAJOR >= 10
    #define WDDM_MINOR_VERSION 15
  #else
    #define WDDM_MINOR_VERSION 14
  #endif
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_WIN7
  #if D3D_FEATURE_LEVEL_MAJOR >= 11
    #define WDDM_MINOR_VERSION 17
  #elif D3D_11_ON_10
    #define WDDM_MINOR_VERSION 16
  #elif D3D_FEATURE_LEVEL_MAJOR >= 10
    #define WDDM_MINOR_VERSION 15
  #else
    #define WDDM_MINOR_VERSION 14
  #endif
#elif DXGKDDI_INTERFACE_VERSION >= GS_DXGKDDI_IV_VISTA
  #if D3D_FEATURE_LEVEL_MAJOR >= 10
    #define WDDM_MINOR_VERSION 15
  #else
    #define WDDM_MINOR_VERSION 14
  #endif
#endif

#define VER_PRODUCTVERSION WDDM_MAJOR_VERSION,WDDM_MINOR_VERSION,GS_PATCH_VERSION,GS_BUILD_VERSION

#define VER_PRODUCTVERSION_STR_TMP0(MAJOR, MINOR, BUILD, PATCH) #MAJOR "." #MINOR "." #BUILD "." #PATCH
#define VER_PRODUCTVERSION_STR_TMP(MAJOR, MINOR, BUILD, PATCH) VER_PRODUCTVERSION_STR_TMP0(MAJOR, MINOR, BUILD, PATCH)

#define VER_PRODUCTVERSION_STR VER_PRODUCTVERSION_STR_TMP(WDDM_MAJOR_VERSION, WDDM_MINOR_VERSION, GS_PATCH_VERSION, GS_BUILD_VERSION)

#endif
