#pragma once

#include "Common.h"
#include "Objects.hpp"

struct GsRegion final
{
    DEFAULT_CONSTRUCT_PUC(GsRegion);
    DEFAULT_DESTRUCT(GsRegion);
    DEFAULT_CM_PUC(GsRegion);
public:
    BYTE Bar : 3;
    BYTE Pad0 : 2;
    BYTE IsCached : 1;
    BYTE IsReadWrite : 1;
    BYTE Is64Bit : 1;

    constexpr GsRegion(const BYTE bar, const bool isCached, const bool isReadWrite, const bool is64Bit) noexcept
        : Bar(bar)
        , Pad0(0)
        , IsCached(isCached)
        , IsReadWrite(isReadWrite)
        , Is64Bit(is64Bit)
    { }
};

struct GsBarMap final
{
    DEFAULT_CONSTRUCT_PU(GsBarMap);
    DEFAULT_DESTRUCT(GsBarMap);
    DEFAULT_CM_PU(GsBarMap);
public:
    GsRegion Region0;
    GsRegion Region1;
    GsRegion Region2;
    GsRegion Region3;
    GsRegion Region4;
    GsRegion Region5;

    constexpr GsBarMap(
        const GsRegion region0
    )
        : Region0(region0)
        , Region1()
        , Region2()
        , Region3()
        , Region4()
        , Region5()
    { }

    constexpr GsBarMap(
        const GsRegion region0,
        const GsRegion region1
    )
        : Region0(region0)
        , Region1(region1)
        , Region2()
        , Region3()
        , Region4()
        , Region5()
    { }

    constexpr GsBarMap(
        const GsRegion region0,
        const GsRegion region1,
        const GsRegion region2
    )
        : Region0(region0)
        , Region1(region1)
        , Region2(region2)
        , Region3()
        , Region4()
        , Region5()
    { }

    constexpr GsBarMap(
        const GsRegion region0,
        const GsRegion region1,
        const GsRegion region2,
        const GsRegion region3
    )
        : Region0(region0)
        , Region1(region1)
        , Region2(region2)
        , Region3(region3)
        , Region4()
        , Region5()
    { }

    constexpr GsBarMap(
        const GsRegion region0,
        const GsRegion region1,
        const GsRegion region2,
        const GsRegion region3,
        const GsRegion region4
    )
        : Region0(region0)
        , Region1(region1)
        , Region2(region2)
        , Region3(region3)
        , Region4(region4)
        , Region5()
    { }

    constexpr GsBarMap(
        const GsRegion region0,
        const GsRegion region1,
        const GsRegion region2,
        const GsRegion region3,
        const GsRegion region4,
        const GsRegion region5
    )
        : Region0(region0)
        , Region1(region1)
        , Region2(region2)
        , Region3(region3)
        , Region4(region4)
        , Region5(region5)
    { }
};

extern GsBarMap Device0001ManufacturerFFFD;
