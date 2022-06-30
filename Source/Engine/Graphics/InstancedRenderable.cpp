#include "Renderer/InstancedRenderable.h"

#include "Texture/Material.h"

namespace library
{
#ifdef LAB10
    namespace lab10
    {
        InstancedRenderable::InstancedRenderable(_In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData()
            , m_padding{ '\0', }
        {
        }

        InstancedRenderable::InstancedRenderable(_In_ std::vector<InstanceData>&& aInstanceData, _In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData(std::move(aInstanceData))
            , m_padding{ '\0', }
        {
        }

        void InstancedRenderable::Update(_In_ FLOAT deltaTime)
        {
            UNREFERENCED_PARAMETER(deltaTime);
        }

        void InstancedRenderable::SetInstanceData(_In_ std::vector<InstanceData>&& aInstanceData)
        {
            m_aInstanceData = std::move(aInstanceData);
        }

        ComPtr<ID3D11Buffer>& InstancedRenderable::GetInstanceBuffer()
        {
            return m_instanceBuffer;
        }

        UINT InstancedRenderable::GetNumInstances() const
        {
            return static_cast<UINT>(m_aInstanceData.size());
        }

        HRESULT InstancedRenderable::initializeInstance(_In_ ID3D11Device* pDevice)
        {
            // Set up the description of the instance buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = static_cast<UINT>(sizeof(InstanceData) * m_aInstanceData.size()),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u,
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = m_aInstanceData.data()
            };

            // Create the instance buffer
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_instanceBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }
    }
#endif

#ifdef LAB09
    namespace lab09
    {
        InstancedRenderable::InstancedRenderable(_In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData()
            , m_padding{ '\0', }
        {
        }

        InstancedRenderable::InstancedRenderable(_In_ std::vector<InstanceData>&& aInstanceData, _In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData(std::move(aInstanceData))
            , m_padding{ '\0', }
        {
        }

        void InstancedRenderable::Update(_In_ FLOAT deltaTime)
        {
            UNREFERENCED_PARAMETER(deltaTime);
        }

        void InstancedRenderable::SetInstanceData(_In_ std::vector<InstanceData>&& aInstanceData)
        {
            m_aInstanceData = std::move(aInstanceData);
        }

        ComPtr<ID3D11Buffer>& InstancedRenderable::GetInstanceBuffer()
        {
            return m_instanceBuffer;
        }

        UINT InstancedRenderable::GetNumInstances() const
        {
            return static_cast<UINT>(m_aInstanceData.size());
        }

        HRESULT InstancedRenderable::initializeInstance(_In_ ID3D11Device* pDevice)
        {
            // Set up the description of the instance buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = static_cast<UINT>(sizeof(InstanceData) * m_aInstanceData.size()),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u,
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = m_aInstanceData.data()
            };

            // Create the instance buffer
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_instanceBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }
    }
#endif

#ifdef LAB08
    namespace lab08
    {
        InstancedRenderable::InstancedRenderable(_In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData()
            , m_padding{ '\0', }
        {
        }

        InstancedRenderable::InstancedRenderable(_In_ std::vector<InstanceData>&& aInstanceData, _In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData(std::move(aInstanceData))
            , m_padding{ '\0', }
        {
        }

        void InstancedRenderable::Update(_In_ FLOAT deltaTime)
        {
            UNREFERENCED_PARAMETER(deltaTime);
        }

        void InstancedRenderable::SetInstanceData(_In_ std::vector<InstanceData>&& aInstanceData)
        {
            m_aInstanceData = std::move(aInstanceData);
        }

        ComPtr<ID3D11Buffer>& InstancedRenderable::GetInstanceBuffer()
        {
            return m_instanceBuffer;
        }

        UINT InstancedRenderable::GetNumInstances() const
        {
            return static_cast<UINT>(m_aInstanceData.size());
        }

        HRESULT InstancedRenderable::initializeInstance(_In_ ID3D11Device* pDevice)
        {
            // Set up the description of the instance buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = static_cast<UINT>(sizeof(InstanceData) * m_aInstanceData.size()),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u,
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = m_aInstanceData.data()
            };

            // Create the instance buffer
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_instanceBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }
    }
#endif

#ifdef ASS02
    namespace ass02
    {
        InstancedRenderable::InstancedRenderable(_In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData()
            , m_padding{ '\0', }
        {
        }

        InstancedRenderable::InstancedRenderable(_In_ std::vector<InstanceData>&& aInstanceData, _In_ const XMFLOAT4& outputColor)
            : Renderable(outputColor)
            , m_instanceBuffer()
            , m_aInstanceData(std::move(aInstanceData))
            , m_padding{ '\0', }
        {
        }

        void InstancedRenderable::Update(_In_ FLOAT deltaTime)
        {
            UNREFERENCED_PARAMETER(deltaTime);
        }

        void InstancedRenderable::SetInstanceData(_In_ std::vector<InstanceData>&& aInstanceData)
        {
            m_aInstanceData = std::move(aInstanceData);
        }

        ComPtr<ID3D11Buffer>& InstancedRenderable::GetInstanceBuffer()
        {
            return m_instanceBuffer;
        }

        UINT InstancedRenderable::GetNumInstances() const
        {
            return static_cast<UINT>(m_aInstanceData.size());
        }

        HRESULT InstancedRenderable::initializeInstance(_In_ ID3D11Device* pDevice)
        {
            // Set up the description of the instance buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = static_cast<UINT>(sizeof(InstanceData) * m_aInstanceData.size()),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u,
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = m_aInstanceData.data()
            };

            // Create the instance buffer
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_instanceBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }
    }
#endif
}