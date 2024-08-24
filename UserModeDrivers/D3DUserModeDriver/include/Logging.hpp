#pragma once

#include "Config.h"
#include "DriverDebug.hpp"
#include <intrin.h>

#pragma warning(push)
#pragma warning(disable: 4505)
static void __declspec(noinline) GsGetCallAddress(void** address)
{
    *address = _ReturnAddress();
}
#pragma warning(pop)

#if GS_ENABLE_DEBUG_LOGGING
#include <ConPrinter.hpp>
#endif

#if GS_ENABLE_DEBUG_LOGGING
template<typename... Args>
static void GsLog(
    const c8* const level,
    const void* const address,
    const char* const functionName,
    const size_t line,
    const Args&... args
) noexcept
{
    if(g_DebugEnable)
    {
        ConPrinter::Print(u8"[{}](0x{XP0}) {}:{}: ", level, address, functionName, line);
        ConPrinter::PrintLn(args...);
    }
}

#define DECL_LOG(NAME) \
    template<typename... Args> \
    inline void GsLog##NAME(const void* address, const char* const functionName, const size_t line, const Args&... args) \
    {                                                         \
        GsLog(u8###NAME, address, functionName, line, args...); \
    }

DECL_LOG(Debug);
DECL_LOG(Info);
DECL_LOG(Warn);
DECL_LOG(Error);

#define internal_LOG(LEVEL, FUNCTION, FILE, LINE, ...) \
    do {                                                    \
        void* logFuncAddress;                               \
        GsGetCallAddress(&logFuncAddress);                  \
        GsLog##LEVEL(logFuncAddress, FUNCTION, LINE , ## __VA_ARGS__); \
    } while(false)
#else
#define internal_LOG(LEVEL, FUNCTION, FILE, LINE, ...)
#endif

#if GS_ENABLE_ENTRYPOINT_TRACING
  #define TRACE_ENTRYPOINT() internal_LOG(Debug, __FUNCTION__, __FILE__, __LINE__, "")
  #define TRACE_ENTRYPOINT_ARG(...) internal_LOG(Debug, __FUNCTION__, __FILE__, __LINE__ , ## __VA_ARGS__)
#else 
  #define TRACE_ENTRYPOINT()
  #define TRACE_ENTRYPOINT_ARG(...)
#endif


#if GS_ENABLE_DEBUG_LOGGING
  #define LOG_DEBUG(...) internal_LOG(Debug, __FUNCTION__, __FILE__, __LINE__ , ## __VA_ARGS__)
#else
  #define LOG_DEBUG(...) do { } while(false)
#endif

#define LOG_INFO(...) internal_LOG(Info, __FUNCTION__, __FILE__, __LINE__ , ## __VA_ARGS__)
#define LOG_WARN(...) internal_LOG(Warn, __FUNCTION__, __FILE__, __LINE__ , ## __VA_ARGS__)
#define LOG_ERROR(...) internal_LOG(Error, __FUNCTION__, __FILE__, __LINE__ , ## __VA_ARGS__)
