#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <ntddk.h>

#pragma warning(push)
#pragma warning(disable: 4505)
static void __declspec(noinline) HyGetCallAddress(void** address)
{
    *address = _ReturnAddress();
}
#pragma warning(pop)

void HyLog(const char* level, size_t levelLength, ULONG filterLevel, const void* address, const char* fmt, va_list args);

const char* GetFileDeviceString(DEVICE_TYPE deviceType);

#define DECL_LOG(NAME, FILTER_LEVEL) \
    inline void HyLog##NAME(const void* address, const char* fmt, ...) \
    {                                                                  \
        va_list args;                                                  \
        va_start(args, fmt);                                           \
        HyLog(#NAME, sizeof(#NAME), FILTER_LEVEL, address, fmt, args); \
        va_end(args);                                                  \
    }

DECL_LOG(Debug, DPFLTR_INFO_LEVEL);
DECL_LOG(Info, DPFLTR_INFO_LEVEL);
DECL_LOG(Warn, DPFLTR_WARNING_LEVEL);
DECL_LOG(Error, DPFLTR_ERROR_LEVEL);

#define internal_LOG(LEVEL, FMT, ...) \
    do {                                                    \
        void* logFuncAddress;                               \
        HyGetCallAddress(&logFuncAddress);                  \
        HyLog##LEVEL(logFuncAddress, FMT , ## __VA_ARGS__); \
    }  while(0)

#if defined(DEBUG) && DEBUG
  #define LOG_DEBUG(FMT, ...) internal_LOG(Debug, FMT , ## __VA_ARGS__)
#else
  #define LOG_DEBUG(FMT, ...) do { } while(0)
#endif
#define LOG_INFO(FMT, ...) internal_LOG(Info, FMT , ## __VA_ARGS__)
#define LOG_WARN(FMT, ...) internal_LOG(Warn, FMT , ## __VA_ARGS__)
#define LOG_ERROR(FMT, ...) internal_LOG(Error, FMT , ## __VA_ARGS__)

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
