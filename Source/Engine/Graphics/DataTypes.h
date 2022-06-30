#pragma once

#include "pch.h"

namespace pr
{
    struct VertexP
    {
        XMFLOAT3 Position;
    };

    struct VertexPT
    {
        XMFLOAT3 Position;
    };

    struct VertexPN
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
    };

    struct VertexPNT
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT2 TexCoord;
    };
}
