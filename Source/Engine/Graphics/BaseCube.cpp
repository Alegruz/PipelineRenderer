#include "Graphics/BaseCube.h"

namespace pr
{
    BaseCube::BaseCube() noexcept
        : Renderable(eVertexType::POS_COLOR)
    {
    }

    HRESULT BaseCube::Initialize(_In_ ID3D12Device* pDevice)
    {
        return initialize(pDevice);
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
