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

        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice) override;
        virtual void Update(_In_ FLOAT deltaTime);

        UINT GetNumVertices() const override;
        UINT GetNumIndices() const override;
    protected:
        const void* getVertices() const override;
        const WORD* getIndices() const override;

        static constexpr const VertexPC VERTICES[] =
        {
            { .Position = XMFLOAT3(-1.0f,  1.0f, -1.0f), .Color = XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 0
            { .Position = XMFLOAT3( 1.0f,  1.0f, -1.0f), .Color = XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 1
            { .Position = XMFLOAT3( 1.0f,  1.0f,  1.0f), .Color = XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 2
            { .Position = XMFLOAT3(-1.0f,  1.0f,  1.0f), .Color = XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 3
            { .Position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .Color = XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 4
            { .Position = XMFLOAT3( 1.0f, -1.0f, -1.0f), .Color = XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 5
            { .Position = XMFLOAT3( 1.0f, -1.0f,  1.0f), .Color = XMFLOAT3(1.0f, 0.0f, 1.0f) }, // 6
            { .Position = XMFLOAT3(-1.0f, -1.0f,  1.0f), .Color = XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 7
        };

        static constexpr const UINT NUM_VERTICES = 8u;
        static constexpr const WORD INDICES[] =
        {
            3,1,0,
            2,1,3,

            0,5,4,
            1,5,0,

            3,4,7,
            0,4,3,

            1,6,5,
            2,6,1,

            2,7,6,
            3,7,2,

            6,4,5,
            7,4,6,
        };
        static constexpr const UINT NUM_INDICES = 36u;
    };
}