/*+===================================================================
  File:      BASECUBE.H

  Summary:   Base cube header file contains declarations of BaseCube 
             class used for the lab samples of Game Graphics 
             Programming course.

  Classes: Cube

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include "Graphics/DataTypes.h"
#include "Graphics/Renderable.h"

namespace pr
{
    class BaseCube : public Renderable
    {
    public:
        explicit BaseCube() noexcept;
        BaseCube(const BaseCube& other) = delete;
        BaseCube(BaseCube&& other) = delete;
        BaseCube& operator=(const BaseCube& other) = delete;
        BaseCube& operator=(BaseCube&& other) = delete;
        ~BaseCube() = default;

        virtual HRESULT Initialize(_In_ ID3D12Device2* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList) override;
        virtual void Update(_In_ FLOAT deltaTime);

        UINT GetNumVertices() const override;
        UINT GetNumIndices() const override;
    protected:
        const void* getVertices() const override;
        const WORD* getIndices() const override;

        static constexpr const VertexPNT VERTICES[] =
        {
            {.Position = XMFLOAT3(-1.0f, 1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, 1.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 0.0f) },
            {.Position = XMFLOAT3(1.0f, 1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, 1.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 0.0f) },
            {.Position = XMFLOAT3(1.0f, 1.0f,  1.0f), .Normal = XMFLOAT3(0.0f, 1.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 1.0f) },
            {.Position = XMFLOAT3(-1.0f, 1.0f,  1.0f), .Normal = XMFLOAT3(0.0f, 1.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 1.0f) },

            {.Position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, -1.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 0.0f) },
            {.Position = XMFLOAT3(1.0f, -1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, -1.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 0.0f) },
            {.Position = XMFLOAT3(1.0f, -1.0f,  1.0f), .Normal = XMFLOAT3(0.0f, -1.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 1.0f) },
            {.Position = XMFLOAT3(-1.0f, -1.0f,  1.0f), .Normal = XMFLOAT3(0.0f, -1.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 1.0f) },

            {.Position = XMFLOAT3(-1.0f, -1.0f,  1.0f), .Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 1.0f) },
            {.Position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 1.0f) },
            {.Position = XMFLOAT3(-1.0f,  1.0f, -1.0f), .Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 0.0f) },
            {.Position = XMFLOAT3(-1.0f,  1.0f,  1.0f), .Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 0.0f) },

            {.Position = XMFLOAT3(1.0f, -1.0f,  1.0f), .Normal = XMFLOAT3(1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 1.0f) },
            {.Position = XMFLOAT3(1.0f, -1.0f, -1.0f), .Normal = XMFLOAT3(1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 1.0f) },
            {.Position = XMFLOAT3(1.0f,  1.0f, -1.0f), .Normal = XMFLOAT3(1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(0.0f, 0.0f) },
            {.Position = XMFLOAT3(1.0f,  1.0f,  1.0f), .Normal = XMFLOAT3(1.0f, 0.0f, 0.0f), .TexCoord = XMFLOAT2(1.0f, 0.0f) },

            {.Position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, -1.0f), .TexCoord = XMFLOAT2(0.0f, 1.0f) },
            {.Position = XMFLOAT3(1.0f, -1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, -1.0f), .TexCoord = XMFLOAT2(1.0f, 1.0f) },
            {.Position = XMFLOAT3(1.0f,  1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, -1.0f), .TexCoord = XMFLOAT2(1.0f, 0.0f) },
            {.Position = XMFLOAT3(-1.0f,  1.0f, -1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, -1.0f), .TexCoord = XMFLOAT2(0.0f, 0.0f) },

            {.Position = XMFLOAT3(-1.0f, -1.0f, 1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, 1.0f), .TexCoord = XMFLOAT2(1.0f, 1.0f) },
            {.Position = XMFLOAT3(1.0f, -1.0f, 1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, 1.0f), .TexCoord = XMFLOAT2(0.0f, 1.0f) },
            {.Position = XMFLOAT3(1.0f,  1.0f, 1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, 1.0f), .TexCoord = XMFLOAT2(0.0f, 0.0f) },
            {.Position = XMFLOAT3(-1.0f,  1.0f, 1.0f), .Normal = XMFLOAT3(0.0f, 0.0f, 1.0f), .TexCoord = XMFLOAT2(1.0f, 0.0f) },
        };

        static constexpr const UINT NUM_VERTICES = 24u;
        static constexpr const WORD INDICES[] =
        {
            3,1,0,
            2,1,3,

            6,4,5,
            7,4,6,

            11,9,8,
            10,9,11,

            14,12,13,
            15,12,14,

            19,17,16,
            18,17,19,

            22,20,21,
            23,20,22
        };
        static constexpr const UINT NUM_INDICES = 36u;
    };
}