// ReSharper disable CppClangTidyModernizeMacroToEnum
#pragma once

// #include "Common.h"
#include <minwindef.h>
#include <d3dukmdt.h>
#include "DriverDataIdentifiers.hpp"

struct CreateAllocationDriverData_V1 final
{
    static constexpr UINT Version = 1;

    DriverDataBase Base;
    UINT64 PhysicalSize;
};

#define CREATE_ALLOCATION_MAGIC ('ACSG') // Grafika Strahlen Create Allocation

union CreateAllocationDriverData final
{
    DriverDataBase Base;
    CreateAllocationDriverData_V1 V1;
};

struct AllocationInfoDriverData_V1 final
{
    static constexpr UINT Version = 1;

    DriverDataBase Base;
    UINT64 PhysicalSize;
    struct
    {
        UINT PrivateFormat : 1;
        UINT Swizzled : 1;
        UINT MipMapped : 1;
        UINT CubeTexture : 1;
        UINT VolumeTexture : 1;
        UINT VertexBuffer : 1;
        UINT IndexBuffer : 1;
        UINT CpuRead : 1;
        UINT CpuWrite : 1;
        UINT PermanentSysMem : 1;
        UINT Reserved : 23;
    } Flags;
    union
    {
        D3DDDIFORMAT Format;
        UINT PrivateFormat;
    };
    UINT Width;
    UINT Height;
    UINT Pitch;
    UINT Depth;
    UINT SlicePitch;
};

#define ALLOCATION_INFO_MAGIC ('IASG') // Grafika Strahlen Allocation Information

union AllocationInfoDriverData final
{
    DriverDataBase Base;
    AllocationInfoDriverData_V1 V1;
};