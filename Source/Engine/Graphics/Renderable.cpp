#include "pch.h"

#include "Graphics/Renderable.h"

#include "Graphics/GraphicsCommon.h"

//#include "assimp/Importer.hpp"	// C++ importer interface
//#include "assimp/scene.h"		// output data structure
//#include "assimp/postprocess.h"	// post processing flags

//#include "Texture/DDSTextureLoader.h"

namespace pr
{

    Renderable::Renderable() noexcept
        : Renderable(eVertexType::POS)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::Renderable

      Summary:  Constructor

      Args:     const XMFLOAT4& outputColor
                  Default color to shader the renderable

      Modifies: [m_pVertexBuffer, m_pIndexBuffer, m_constantBuffer,
                 m_aMeshes, m_aMaterials, m_vertexShader,
                 m_pixelShader, m_outputColor, m_World].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderable::Renderable(_In_ eVertexType vertexType) noexcept
        : m_World(XMMatrixIdentity())
        , m_VertexType(vertexType)
        , m_pVertexBuffer()
        , m_pIndexBuffer()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::SetVertexShader

      Summary:  Sets the vertex shader to be used for this renderable
                object

      Args:     const std::shared_ptr<VertexShader>& vertexShader
                  Vertex shader to set to

      Modifies: [m_vertexShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    //void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
    //{
    //    m_vertexShader = vertexShader;
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::SetPixelShader

      Summary:  Sets the pixel shader to be used for this renderable
                object

      Args:     const std::shared_ptr<PixelShader>& pixelShader
                  Pixel shader to set to

      Modifies: [m_pixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    //void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
    //{
    //    m_pixelShader = pixelShader;
    //}

    //void Renderable::AddMaterial(_In_ const std::shared_ptr<Material>& material)
    //{
    //    m_aMaterials.push_back(material);
    //}

    //HRESULT Renderable::SetMaterialOfMesh(_In_ const UINT uMeshIndex, _In_ const UINT uMaterialIndex)
    //{
    //    if (uMeshIndex >= m_aMeshes.size() || uMaterialIndex >= m_aMaterials.size())
    //    {
    //        return E_FAIL;
    //    }

    //    m_aMeshes[uMeshIndex].uMaterialIndex = uMaterialIndex;

    //    if (m_aMaterials[uMeshIndex]->pNormal)
    //    {
    //        m_bHasNormalMap = true;
    //    }

    //    return S_OK;
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11VertexShader>&
                  Vertex shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    //ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
    //{
    //    return m_vertexShader->GetVertexShader();
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetPixelShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11PixelShader>&
                  Pixel shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    //ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
    //{
    //    return m_pixelShader->GetPixelShader();
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexLayout

      Summary:  Returns the vertex input layout

      Returns:  ComPtr<ID3D11InputLayout>&
                  Vertex input layout
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    //ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
    //{
    //    return m_vertexShader->GetVertexLayout();
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexBuffer

      Summary:  Returns the vertex buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Vertex buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D12Resource>& Renderable::GetVertexBuffer()
    {
        return m_pVertexBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetIndexBuffer

      Summary:  Returns the index buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Index buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D12Resource>& Renderable::GetIndexBuffer()
    {
        return m_pIndexBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetConstantBuffer

      Summary:  Returns the constant buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Constant buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    //ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
    //{
    //    return m_constantBuffer;
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetWorldMatrix

      Summary:  Returns the world matrix

      Returns:  const XMMATRIX&
                  World matrix
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMMATRIX& Renderable::GetWorldMatrix() const
    {
        return m_World;
    }

    //const XMFLOAT4& Renderable::GetOutputColor() const
    //{
    //    return m_outputColor;
    //}

    //BOOL Renderable::HasTexture() const
    //{
    //    return m_aMaterials.size() > 0;
    //}

    //const std::shared_ptr<Material>& Renderable::GetMaterial(UINT uIndex) const
    //{
    //    assert(uIndex < m_aMaterials.size());

    //    return m_aMaterials[uIndex];
    //}

    //const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
    //{
    //    assert(uIndex < m_aMeshes.size());

    //    return m_aMeshes[uIndex];
    //}

    void Renderable::RotateX(_In_ FLOAT angle)
    {
        m_World *= XMMatrixRotationX(angle);
    }

    void Renderable::RotateY(_In_ FLOAT angle)
    {
        m_World *= XMMatrixRotationY(angle);
    }

    void Renderable::RotateZ(_In_ FLOAT angle)
    {
        m_World *= XMMatrixRotationZ(angle);
    }

    void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
    {
        m_World *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    }

    void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
    {
        m_World *= XMMatrixScaling(scaleX, scaleY, scaleZ);
    }

    void Renderable::Translate(_In_ const XMVECTOR& offset)
    {
        m_World *= XMMatrixTranslationFromVector(offset);
    }

    eVertexType Renderable::GetVertexType() const noexcept
    {
        return m_VertexType;
    }

    //UINT Renderable::GetNumMeshes() const
    //{
    //    return static_cast<UINT>(m_aMeshes.size());
    //}

    //UINT Renderable::GetNumMaterials() const
    //{
    //    return static_cast<UINT>(m_aMaterials.size());
    //}
    //BOOL Renderable::HasNormalMap() const
    //{
    //    return m_bHasNormalMap;
    //}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::initialize

      Summary:  Initializes the buffers and the world matrix

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                PCWSTR pszTextureFileName
                  File name of the texture to usen

      Modifies: [m_pVertexBuffer, m_pIndexBuffer, m_constantBuffer].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderable::initialize(
        _In_ ID3D12Device* pDevice
    )
    {
        HRESULT hr = S_OK;

        // Create vertex buffer
        ComPtr<ID3D12Resource> pIntermediateVertexBuffer;
        //UpdateBufferResource();

        // Create index buffer
        // 
        // Create the constant buffers

        return hr;
    }
}
