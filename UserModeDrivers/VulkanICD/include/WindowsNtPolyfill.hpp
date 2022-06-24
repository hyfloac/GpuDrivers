#pragma once

#include <utility>
#include <cstring>

#include <Windows.h>
#include <winnt.h>
#include <winternl.h>
#include <ntstatus.h>

#include <NumTypes.hpp>

#ifndef NT_SUCCESS
  #define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
  #define STATUS_BUFFER_TOO_SMALL  ((DWORD) 0xC0000023L)
#endif

#ifndef STATUS_DEVICE_REMOVED
  #define STATUS_DEVICE_REMOVED    ((DWORD) 0xC00002B6L)
#endif

#ifndef STATUS_SUCCESS
  #define STATUS_SUCCESS           ((DWORD) 0x00000000L)
#endif

#ifndef STATUS_INVALID_PARAMETER
  #define STATUS_INVALID_PARAMETER ((DWORD) 0xC000000DL)
#endif


namespace std {

template<>
struct hash<LUID>
{
    [[nodiscard]] ::std::size_t operator()(const LUID& luid) const noexcept
    {
        if constexpr(sizeof(::std::size_t) == sizeof(u64))
        {
            ::std::size_t ret;
            ::std::memcpy(&ret, &luid, sizeof(ret));
            return  ret;
        }
        else if constexpr(sizeof(::std::size_t) == sizeof(u32))
        {
            ::std::size_t buf[2];
            ::std::memcpy(buf, &luid, sizeof(buf));
            return (buf[0] * 37) + buf[1];
        }
        else if constexpr(sizeof(::std::size_t) == sizeof(u16))
        {
            ::std::size_t buf[4];
            ::std::memcpy(buf, &luid, sizeof(buf));
            ::std::size_t ret = buf[0];
            ret = ret * 37 + buf[1];
            ret = ret * 37 + buf[2];
            ret = ret * 37 + buf[3];
            return ret;
        }
        else
        {
            u64 ret;
            ::std::memcpy(&ret, &luid, sizeof(ret));
            return static_cast<::std::size_t>(ret);
        }
    }
};

template<>
struct hash<GUID>
{
    [[nodiscard]] ::std::size_t operator()(const GUID& guid) const noexcept
    {
        if constexpr(sizeof(::std::size_t) == sizeof(u64))
        {
            ::std::size_t buf[2];
            ::std::memcpy(&buf, &guid, sizeof(buf));
            return (buf[0] * 37) + buf[1];
        }
        else if constexpr(sizeof(::std::size_t) == sizeof(u32))
        {
            ::std::size_t buf[4];
            ::std::memcpy(buf, &guid, sizeof(buf));
            ::std::size_t ret = buf[0];
            ret = ret * 37 + buf[1];
            ret = ret * 37 + buf[2];
            ret = ret * 37 + buf[3];
        }
        else if constexpr(sizeof(::std::size_t) == sizeof(u16))
        {
            ::std::size_t buf[8];
            ::std::memcpy(buf, &guid, sizeof(buf));
            ::std::size_t ret = buf[0];
            ret = ret * 37 + buf[1];
            ret = ret * 37 + buf[2];
            ret = ret * 37 + buf[3];
            ret = ret * 37 + buf[4];
            ret = ret * 37 + buf[5];
            ret = ret * 37 + buf[6];
            ret = ret * 37 + buf[7];
            return ret;
        }
        else
        {
            u64 buf[2];
            ::std::memcpy(&buf, &guid, sizeof(buf));
            return static_cast<::std::size_t>((buf[0] * 37) + buf[1]);
        }
    }
};

}

[[nodiscard]] inline bool operator==(const LUID& left, const LUID& right) noexcept
{
    return left.LowPart == right.LowPart && left.HighPart == right.HighPart;
}

