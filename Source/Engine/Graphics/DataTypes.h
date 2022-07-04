#pragma once

#include "pch.h"

namespace pr
{
    enum class eVertexType : BYTE
    {
        POS,
        POS_COLOR,
        POS_TEXCOORD,
        POS_NORM,
        POS_NORM_TEXCOORD,
        COUNT,
    };

    struct VertexP
    {
        XMFLOAT3 Position;
    };
    static_assert(sizeof(VertexP) == 12);

    struct VertexPC
    {
        XMFLOAT3 Position;
        XMFLOAT3 Color;
    };
    static_assert(sizeof(VertexPC) == 24);

    struct VertexPT
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
    };
    static_assert(sizeof(VertexPT) == 20);

    struct VertexPN
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
    };
    static_assert(sizeof(VertexPN) == 24);

    struct VertexPNT
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT2 TexCoord;
    };
    static_assert(sizeof(VertexPNT) == 32);

    constexpr size_t VERTEX_SIZE[] =
    {
        sizeof(VertexP),
        sizeof(VertexPC),
        sizeof(VertexPT),
        sizeof(VertexPN),
        sizeof(VertexPNT),
    };
}
