#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <dispmprt.h>

#ifdef _MSC_VER
# pragma warning(push)
# if _MSC_VER >= 1900 /*RT_MSC_VER_VC140*/
#  pragma warning(disable:4255) /* d3dkmthk.h(2061): warning C4255: 'PFND3DKMT_CHECKEXCLUSIVEOWNERSHIP': no function prototype given: converting '()' to '(void)' */
# endif
#endif

/* 10.0.22000.0 SDK: */
#define DXGKDDI_INTERFACE_VERSION_WDDM_1_3 DXGKDDI_INTERFACE_VERSION_WDDM1_3
#define DXGKDDI_INTERFACE_VERSION_WDDM_2_0 DXGKDDI_INTERFACE_VERSION_WDDM2_0
#define DXGKDDI_INTERFACE_VERSION_WDDM1_3_M1 DXGKDDI_INTERFACE_VERSION_WDDM1_3

//#include <d3dkmthk.h>

#ifdef __cplusplus
} /* extern "C" */
#endif
