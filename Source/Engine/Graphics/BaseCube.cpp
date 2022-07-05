#include "pch.h"

#include "Graphics/BaseCube.h"

namespace pr
{
    BaseCube::BaseCube() noexcept
        : Renderable(eVertexType::POS_NORM_TEXCOORD)
    {
    }

    HRESULT BaseCube::Initialize(_In_ ID3D12Device2* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList)
    {
        BasicMeshEntry basicMeshEntry;
        basicMeshEntry.uNumIndices = NUM_INDICES;

        m_aMeshes.push_back(basicMeshEntry);

        return initialize(pDevice, pCommandList);
    }

    void BaseCube::Update(FLOAT deltaTime)
    {
        UNREFERENCED_PARAMETER(deltaTime);
    }

    const void* BaseCube::getVertices() const
    {
        return VERTICES;
    }

    UINT BaseCube::GetNumVertices() const
    {
        return NUM_VERTICES;
    }

    const WORD* BaseCube::getIndices() const
    {
        return INDICES;
    }

    UINT BaseCube::GetNumIndices() const
    {
        return NUM_INDICES;
    }
}
