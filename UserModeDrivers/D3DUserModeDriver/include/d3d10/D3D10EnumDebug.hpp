#pragma once

#include <d3d10umddi.h>

static inline const char* D3D10ResourceTypeToString(const D3D10DDIRESOURCE_TYPE resourceType) noexcept
{
    switch(resourceType)
    {
        case D3D10DDIRESOURCE_BUFFER:      return "D3D10DDIRESOURCE_BUFFER";
        case D3D10DDIRESOURCE_TEXTURE1D:   return "D3D10DDIRESOURCE_TEXTURE1D";
        case D3D10DDIRESOURCE_TEXTURE2D:   return "D3D10DDIRESOURCE_TEXTURE2D";
        case D3D10DDIRESOURCE_TEXTURE3D:   return "D3D10DDIRESOURCE_TEXTURE3D";
        case D3D10DDIRESOURCE_TEXTURECUBE: return "D3D10DDIRESOURCE_TEXTURECUBE";
#if D3D11DDI_MINOR_HEADER_VERSION >= 1
        case D3D11DDIRESOURCE_BUFFEREX:    return "D3D11DDIRESOURCE_BUFFEREX";
#endif
        default:                           return "Unknown";
    }
}
