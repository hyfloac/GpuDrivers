#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Config.h"
#include <stdarg.h>
#include <ntddk.h>

#pragma warning(push)
#pragma warning(disable: 4505)
#pragma code_seg(push)
#pragma code_seg("_KTEXT")
static void __declspec(noinline) HyGetCallAddress(void** address)
{
    *address = _ReturnAddress();
}
#pragma code_seg(pop)
#pragma warning(pop)

void HyLog(
    const char* const level, 
    const size_t levelLength, 
    const ULONG filterLevel, 
    const void* const address, 
    const char* const functionName,
    const size_t functionNameLength,
    const char* const fileName,
    const size_t fileNameLength,
    const size_t line, 
    const char* fmt, 
    va_list args
);

const char* GetFileDeviceString(DEVICE_TYPE deviceType);

#define DECL_LOG(NAME, FILTER_LEVEL) \
    inline void HyLog##NAME(const void* address, const char* const functionName, const size_t functionNameLength, const char* const fileName, const size_t fileNameLength, const size_t line, const char* fmt, ...) \
    {                                                                  \
        va_list args;                                                  \
        va_start(args, fmt);                                           \
        HyLog(#NAME, sizeof(#NAME), FILTER_LEVEL, address, functionName, functionNameLength, fileName, fileNameLength, line, fmt, args); \
        va_end(args);                                                  \
    }

#pragma code_seg(push)
#pragma code_seg("_KTEXT")
DECL_LOG(Debug, DPFLTR_INFO_LEVEL);
DECL_LOG(Info, DPFLTR_INFO_LEVEL);
DECL_LOG(Warn, DPFLTR_WARNING_LEVEL);
DECL_LOG(Error, DPFLTR_ERROR_LEVEL);
#pragma code_seg(pop)

#define internal_LOG(LEVEL, FUNCTION, FILE, LINE, FMT, ...) \
    do {                                                    \
        void* logFuncAddress;                               \
        HyGetCallAddress(&logFuncAddress);                  \
        HyLog##LEVEL(logFuncAddress, FUNCTION, sizeof(FUNCTION), FILE, sizeof(FILE), LINE, FMT , ## __VA_ARGS__); \
    }  while(0)

#if defined(DEBUG) && DEBUG
  #define LOG_DEBUG(FMT, ...) internal_LOG(Debug, __FUNCTION__, __FILE__, __LINE__, FMT , ## __VA_ARGS__)
#else
  #define LOG_DEBUG(FMT, ...) do { } while(0)
#endif

#if GS_ENABLE_ENTRYPOINT_TRACING
  #define TRACE_ENTRYPOINT() internal_LOG(Debug, __FUNCTION__, __FILE__, __LINE__, "\n")
  #define TRACE_ENTRYPOINT_ARG(FMT, ...) internal_LOG(Debug, __FUNCTION__, __FILE__, __LINE__, FMT , ## __VA_ARGS__)
#else 
  #define TRACE_ENTRYPOINT()
  #define TRACE_ENTRYPOINT_ARG(FMT, ...)
#endif

#define LOG_INFO(FMT, ...) internal_LOG(Info, __FUNCTION__, __FILE__, __LINE__, FMT , ## __VA_ARGS__)
#define LOG_WARN(FMT, ...) internal_LOG(Warn, __FUNCTION__, __FILE__, __LINE__, FMT , ## __VA_ARGS__)
#define LOG_ERROR(FMT, ...) internal_LOG(Error, __FUNCTION__, __FILE__, __LINE__, FMT , ## __VA_ARGS__)

#if DEBUG
  #define CHECK_IRQL(IRQL)                                                                                                \
      if(KeGetCurrentIrql() > (IRQL))                                                                                     \
      {                                                                                                                   \
          LOG_DEBUG("IRQL %d did not match the required IRQL of %d for %s.\n", KeGetCurrentIrql(), (IRQL), __FUNCTION__); \
          __debugbreak();                                                                                                 \
      }
#else
  #define CHECK_IRQL(IRQL) do { } while(0)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
