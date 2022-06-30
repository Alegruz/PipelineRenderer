#include "Renderer/Renderable.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

#include "Texture/DDSTextureLoader.h"

namespace library
{
#ifdef LAB10
    namespace lab10
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const XMFLOAT4& outputColor
                      Default color to shader the renderable

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_aMeshes, m_aMaterials, m_vertexShader,
                     m_pixelShader, m_outputColor, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const XMFLOAT4& outputColor)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_normalBuffer()
            , m_aMeshes()
            , m_aMaterials()
            , m_vertexShader()
            , m_pixelShader()
            , m_outputColor(outputColor)
            , m_padding{ '\0' }
            , m_world(XMMatrixIdentity())
            , m_bHasNormalMap(false)
            , m_aNormalData()
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers
                    PCWSTR pszTextureFileName
                      File name of the texture to usen

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice,
            _In_ ID3D11DeviceContext* pImmediateContext
        )
        {
            UNREFERENCED_PARAMETER(pImmediateContext);

            // Create vertex buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices()
            };

            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Calculate tangent and bitanget, if not calculated
            if (m_aNormalData.empty()) {
                calculateNormalMapVectors();
            }

            // Create normal buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(NormalData) * GetNumVertices();
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = m_aNormalData.data();
            hr = pDevice->CreateBuffer(&bd, &initData, m_normalBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        void Renderable::calculateNormalMapVectors()
        {
            UINT uNumFaces = GetNumIndices() / 3;
            const SimpleVertex* aVertices = getVertices();
            const WORD* aIndices = getIndices();

            m_aNormalData.resize(GetNumVertices(), NormalData());

            XMFLOAT3 tangent, bitangent;

            for (int i = 0; i < uNumFaces; ++i)
            {
                calculateTangentBitangent(aVertices[aIndices[i * 3]], aVertices[aIndices[i * 3 + 1]], aVertices[aIndices[i * 3 + 2]], tangent, bitangent);

                m_aNormalData[aIndices[i * 3]].Tangent = tangent;
                m_aNormalData[aIndices[i * 3]].Bitangent = bitangent;

                m_aNormalData[aIndices[i * 3 + 1]].Tangent = tangent;
                m_aNormalData[aIndices[i * 3 + 1]].Bitangent = bitangent;

                m_aNormalData[aIndices[i * 3 + 2]].Tangent = tangent;
                m_aNormalData[aIndices[i * 3 + 2]].Bitangent = bitangent;
            }
        }

        void Renderable::calculateTangentBitangent(_In_ const SimpleVertex& v1, _In_ const SimpleVertex& v2, _In_ const SimpleVertex& v3, _Out_ XMFLOAT3& tangent, _Out_ XMFLOAT3& bitangent)
        {
            XMFLOAT3 vector1, vector2;
            XMFLOAT2 tuVector, tvVector;

            // Calculate the two vectors for this face.
            vector1.x = v2.Position.x - v1.Position.x;
            vector1.y = v2.Position.y - v1.Position.y;
            vector1.z = v2.Position.z - v1.Position.z;

            vector2.x = v3.Position.x - v1.Position.x;
            vector2.y = v3.Position.y - v1.Position.y;
            vector2.z = v3.Position.z - v1.Position.z;

            // Calculate the tu and tv texture space vectors.
            tuVector.x = v2.TexCoord.x - v1.TexCoord.x;
            tvVector.x = v2.TexCoord.y - v1.TexCoord.y;

            tuVector.y = v3.TexCoord.x - v1.TexCoord.x;
            tvVector.y = v3.TexCoord.y - v1.TexCoord.y;

            // Calculate the denominator of the tangent/binormal equation.
            float den = 1.0f / (tuVector.x * tvVector.y - tuVector.y * tvVector.x);

            // Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
            tangent.x = (tvVector.y * vector1.x - tvVector.x * vector2.x) * den;
            tangent.y = (tvVector.y * vector1.y - tvVector.x * vector2.y) * den;
            tangent.z = (tvVector.y * vector1.z - tvVector.x * vector2.z) * den;

            bitangent.x = (tuVector.x * vector2.x - tuVector.y * vector1.x) * den;
            bitangent.y = (tuVector.x * vector2.y - tuVector.y * vector1.y) * den;
            bitangent.z = (tuVector.x * vector2.z - tuVector.y * vector1.z) * den;

            // Calculate the length of this normal.
            float length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

            // Normalize the normal and then store it
            tangent.x = tangent.x / length;
            tangent.y = tangent.y / length;
            tangent.z = tangent.z / length;

            // Calculate the length of this normal.
            length = sqrt((bitangent.x * bitangent.x) + (bitangent.y * bitangent.y) + (bitangent.z * bitangent.z));

            // Normalize the normal and then store it
            bitangent.x = bitangent.x / length;
            bitangent.y = bitangent.y / length;
            bitangent.z = bitangent.z / length;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        void Renderable::AddMaterial(_In_ const std::shared_ptr<Material>& material)
        {
            m_aMaterials.push_back(material);
        }

        HRESULT Renderable::SetMaterialOfMesh(_In_ const UINT uMeshIndex, _In_ const UINT uMaterialIndex)
        {
            if (uMeshIndex >= m_aMeshes.size() || uMaterialIndex >= m_aMaterials.size())
            {
                return E_FAIL;
            }

            m_aMeshes[uMeshIndex].uMaterialIndex = uMaterialIndex;

            if (m_aMaterials[uMeshIndex]->pNormal)
            {
                m_bHasNormalMap = true;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        ComPtr<ID3D11Buffer>& Renderable::GetNormalBuffer()
        {
            return m_normalBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        const XMFLOAT4& Renderable::GetOutputColor() const
        {
            return m_outputColor;
        }

        BOOL Renderable::HasTexture() const
        {
            return m_aMaterials.size() > 0;
        }

        const std::shared_ptr<Material>& Renderable::GetMaterial(UINT uIndex) const
        {
            assert(uIndex < m_aMaterials.size());

            return m_aMaterials[uIndex];
        }

        const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
        {
            assert(uIndex < m_aMeshes.size());

            return m_aMeshes[uIndex];
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }

        UINT Renderable::GetNumMeshes() const
        {
            return static_cast<UINT>(m_aMeshes.size());
        }

        UINT Renderable::GetNumMaterials() const
        {
            return static_cast<UINT>(m_aMaterials.size());
        }
        BOOL Renderable::HasNormalMap() const
        {
            return m_bHasNormalMap;
        }
    }
#endif

#ifdef LAB09
    namespace lab09
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const XMFLOAT4& outputColor
                      Default color to shader the renderable

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_aMeshes, m_aMaterials, m_vertexShader,
                     m_pixelShader, m_outputColor, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const XMFLOAT4& outputColor)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_normalBuffer()
            , m_aMeshes()
            , m_aMaterials()
            , m_vertexShader()
            , m_pixelShader()
            , m_outputColor(outputColor)
            , m_padding{ '\0' }
            , m_world(XMMatrixIdentity())
            , m_bHasNormalMap(false)
            , m_aNormalData()
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers
                    PCWSTR pszTextureFileName
                      File name of the texture to usen

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice,
            _In_ ID3D11DeviceContext* pImmediateContext
        )
        {
            UNREFERENCED_PARAMETER(pImmediateContext);

            // Create vertex buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices()
            };

            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Calculate tangent and bitanget, if not calculated
            if (HasTexture() && m_aNormalData.empty()) {
                calculateNormalMapVectors();
            }

            // Create normal buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(NormalData) * GetNumVertices();
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = m_aNormalData.data();
            hr = pDevice->CreateBuffer(&bd, &initData, m_normalBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        void Renderable::calculateNormalMapVectors()
        {
            UINT uNumFaces = GetNumIndices() / 3;
            const SimpleVertex* aVertices = getVertices();
            const WORD* aIndices = getIndices();

            m_aNormalData.resize(GetNumVertices(), NormalData());

            XMFLOAT3 tangent, bitangent;

            for (int i = 0; i < uNumFaces; ++i)
            {
                calculateTangentBitangent(aVertices[aIndices[i * 3]], aVertices[aIndices[i * 3 + 1]], aVertices[aIndices[i * 3 + 2]], tangent, bitangent);

                m_aNormalData[aIndices[i * 3]].Tangent = tangent;
                m_aNormalData[aIndices[i * 3]].Bitangent = bitangent;

                m_aNormalData[aIndices[i * 3 + 1]].Tangent = tangent;
                m_aNormalData[aIndices[i * 3 + 1]].Bitangent = bitangent;

                m_aNormalData[aIndices[i * 3 + 2]].Tangent = tangent;
                m_aNormalData[aIndices[i * 3 + 2]].Bitangent = bitangent;
            }
        }

        void Renderable::calculateTangentBitangent(_In_ const SimpleVertex& v1, _In_ const SimpleVertex& v2, _In_ const SimpleVertex& v3, _Out_ XMFLOAT3& tangent, _Out_ XMFLOAT3& bitangent)
        {
            XMFLOAT3 vector1, vector2;
            XMFLOAT2 tuVector, tvVector;
            
            // Calculate the two vectors for this face.
            vector1.x = v2.Position.x - v1.Position.x;
            vector1.y = v2.Position.y - v1.Position.y;
            vector1.z = v2.Position.z - v1.Position.z;

            vector2.x = v3.Position.x - v1.Position.x;
            vector2.y = v3.Position.y - v1.Position.y;
            vector2.z = v3.Position.z - v1.Position.z;

            // Calculate the tu and tv texture space vectors.
            tuVector.x = v2.TexCoord.x - v1.TexCoord.x;
            tvVector.x = v2.TexCoord.y - v1.TexCoord.y;

            tuVector.y = v3.TexCoord.x - v1.TexCoord.x;
            tvVector.y = v3.TexCoord.y - v1.TexCoord.y;

            // Calculate the denominator of the tangent/binormal equation.
            float den = 1.0f / (tuVector.x * tvVector.y - tuVector.y * tvVector.x);
            
            // Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
            tangent.x = (tvVector.y * vector1.x - tvVector.x * vector2.x) * den;
            tangent.y = (tvVector.y * vector1.y - tvVector.x * vector2.y) * den;
            tangent.z = (tvVector.y * vector1.z - tvVector.x * vector2.z) * den;

            bitangent.x = (tuVector.x * vector2.x - tuVector.y * vector1.x) * den;
            bitangent.y = (tuVector.x * vector2.y - tuVector.y * vector1.y) * den;
            bitangent.z = (tuVector.x * vector2.z - tuVector.y * vector1.z) * den;

            // Calculate the length of this normal.
            float length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

            // Normalize the normal and then store it
            tangent.x = tangent.x / length;
            tangent.y = tangent.y / length;
            tangent.z = tangent.z / length;

            // Calculate the length of this normal.
            length = sqrt((bitangent.x * bitangent.x) + (bitangent.y * bitangent.y) + (bitangent.z * bitangent.z));

            // Normalize the normal and then store it
            bitangent.x = bitangent.x / length;
            bitangent.y = bitangent.y / length;
            bitangent.z = bitangent.z / length;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        void Renderable::AddMaterial(_In_ const std::shared_ptr<Material>& material)
        {
            m_aMaterials.push_back(material);
        }

        HRESULT Renderable::SetMaterialOfMesh(_In_ const UINT uMeshIndex, _In_ const UINT uMaterialIndex)
        {
            if (uMeshIndex >= m_aMeshes.size() || uMaterialIndex >= m_aMaterials.size())
            {
                return E_FAIL;
            }

            m_aMeshes[uMeshIndex].uMaterialIndex = uMaterialIndex;

            if (m_aMaterials[uMeshIndex]->pNormal)
            {
                m_bHasNormalMap = true;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        ComPtr<ID3D11Buffer>& Renderable::GetNormalBuffer()
        {
            return m_normalBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        const XMFLOAT4& Renderable::GetOutputColor() const
        {
            return m_outputColor;
        }

        BOOL Renderable::HasTexture() const
        {
            return m_aMaterials.size() > 0;
        }

        const std::shared_ptr<Material>& Renderable::GetMaterial(UINT uIndex) const
        {
            assert(uIndex < m_aMaterials.size());

            return m_aMaterials[uIndex];
        }

        const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
        {
            assert(uIndex < m_aMeshes.size());

            return m_aMeshes[uIndex];
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }

        UINT Renderable::GetNumMeshes() const
        {
            return static_cast<UINT>(m_aMeshes.size());
        }

        UINT Renderable::GetNumMaterials() const
        {
            return static_cast<UINT>(m_aMaterials.size());
        }
        BOOL Renderable::HasNormalMap() const
        {
            return m_bHasNormalMap;
        }
    }
#endif

#ifdef LAB08
    namespace lab08
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const XMFLOAT4& outputColor
                      Default color to shader the renderable

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_aMeshes, m_aMaterials, m_vertexShader,
                     m_pixelShader, m_outputColor, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const XMFLOAT4& outputColor)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_aMeshes()
            , m_aMaterials()
            , m_vertexShader()
            , m_pixelShader()
            , m_outputColor(outputColor)
            //, m_padding{ '\0' }
            , m_world(XMMatrixIdentity())
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers
                    PCWSTR pszTextureFileName
                      File name of the texture to use

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice,
            _In_ ID3D11DeviceContext* pImmediateContext
        )
        {
            UNREFERENCED_PARAMETER(pImmediateContext);

            // Create vertex buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        const XMFLOAT4& Renderable::GetOutputColor() const
        {
            return m_outputColor;
        }

        BOOL Renderable::HasTexture() const
        {
            return m_aMaterials.size() > 0;
        }

        const Material& Renderable::GetMaterial(UINT uIndex) const
        {
            assert(uIndex < m_aMaterials.size());

            return m_aMaterials[uIndex];
        }

        const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
        {
            assert(uIndex < m_aMeshes.size());

            return m_aMeshes[uIndex];
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }

        UINT Renderable::GetNumMeshes() const
        {
            return static_cast<UINT>(m_aMeshes.size());
        }

        UINT Renderable::GetNumMaterials() const
        {
            return static_cast<UINT>(m_aMaterials.size());
        }
    }
#endif

#ifdef ASS02
    namespace ass02
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const XMFLOAT4& outputColor
                      Default color to shader the renderable

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_aMeshes, m_aMaterials, m_vertexShader,
                     m_pixelShader, m_outputColor, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const XMFLOAT4& outputColor)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_aMeshes()
            , m_aMaterials()
            , m_vertexShader()
            , m_pixelShader()
            , m_outputColor(outputColor)
            //, m_padding{ '\0' }
            , m_world(XMMatrixIdentity())
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers
                    PCWSTR pszTextureFileName
                      File name of the texture to use

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice,
            _In_ ID3D11DeviceContext* pImmediateContext
        )
        {
            UNREFERENCED_PARAMETER(pImmediateContext);

            // Create vertex buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        const XMFLOAT4& Renderable::GetOutputColor() const
        {
            return m_outputColor;
        }

        BOOL Renderable::HasTexture() const
        {
            return m_aMaterials.size() > 0;
        }

        const Material& Renderable::GetMaterial(UINT uIndex) const
        {
            assert(uIndex < m_aMaterials.size());

            return m_aMaterials[uIndex];
        }

        const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
        {
            assert(uIndex < m_aMeshes.size());

            return m_aMeshes[uIndex];
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }

        UINT Renderable::GetNumMeshes() const
        {
            return static_cast<UINT>(m_aMeshes.size());
        }

        UINT Renderable::GetNumMaterials() const
        {
            return static_cast<UINT>(m_aMaterials.size());
        }
    }
#endif

#ifdef LAB07
    namespace lab07
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const XMFLOAT4& outputColor
                      Default color to shader the renderable

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_aMeshes, m_aMaterials, m_vertexShader,
                     m_pixelShader, m_outputColor, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const XMFLOAT4& outputColor)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_aMeshes()
            , m_aMaterials()
            , m_vertexShader()
            , m_pixelShader()
            , m_outputColor(outputColor)
            //, m_padding{ '\0' }
            , m_world(XMMatrixIdentity())
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers
                    PCWSTR pszTextureFileName
                      File name of the texture to use

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice,
            _In_ ID3D11DeviceContext* pImmediateContext
        )
        {
            UNREFERENCED_PARAMETER(pImmediateContext);

            // Create vertex buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        const XMFLOAT4& Renderable::GetOutputColor() const
        {
            return m_outputColor;
        }

        BOOL Renderable::HasTexture() const
        {
            return m_aMaterials.size() > 0;
        }

        const Material& Renderable::GetMaterial(UINT uIndex) const
        {
            assert(uIndex < m_aMaterials.size());

            return m_aMaterials[uIndex];
        }

        const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
        {
            assert(uIndex < m_aMeshes.size());

            return m_aMeshes[uIndex];
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }

        UINT Renderable::GetNumMeshes() const
        {
            return static_cast<UINT>(m_aMeshes.size());
        }

        UINT Renderable::GetNumMaterials() const
        {
            return static_cast<UINT>(m_aMaterials.size());
        }
    }
#endif

#ifdef LAB06
    namespace lab06
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const std::filesystem::path& textureFilePath
                      Path to the texture to use

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_textureRV, m_samplerLinear, m_vertexShader,
                     m_pixelShader, m_textureFilePath, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const std::filesystem::path& textureFilePath)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_textureRV()
            , m_samplerLinear()
            , m_vertexShader()
            , m_pixelShader()
            , m_textureFilePath(textureFilePath)
            , m_outputColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
            , m_world(XMMatrixIdentity())
            , m_bHasTextures(TRUE)
            , m_padding{ '\0' }
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const XMFLOAT4& outputColor
                      Default color to shader the renderable

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_textureRV, m_samplerLinear, m_vertexShader,
                     m_pixelShader, m_textureFilePath, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(_In_ const XMFLOAT4& outputColor)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_textureRV()
            , m_samplerLinear()
            , m_vertexShader()
            , m_pixelShader()
            , m_textureFilePath()
            , m_outputColor(outputColor)
            , m_world(XMMatrixIdentity())
            , m_bHasTextures(FALSE)
            , m_padding{ '\0', }
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice,
            _In_ ID3D11DeviceContext* pImmediateContext
            )
        {
            // Create vertex buffer
            D3D11_BUFFER_DESC bd = 
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData = 
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;
            pImmediateContext->IASetVertexBuffers(0u, 1u, m_vertexBuffer.GetAddressOf(), &uStride, &uOffset);

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set index buffer
            pImmediateContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            if (m_bHasTextures)
            {
                // Load the Texture
                hr = CreateDDSTextureFromFile(pDevice, m_textureFilePath.filename().wstring().c_str(), nullptr, m_textureRV.GetAddressOf());
                if (FAILED(hr))
                {
                    return hr;
                }

                // Create the sample state
                D3D11_SAMPLER_DESC sampDesc =
                {
                    .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
                    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
                    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
                    .ComparisonFunc = D3D11_COMPARISON_NEVER,
                    .MinLOD = 0,
                    .MaxLOD = D3D11_FLOAT32_MAX
                };
                hr = pDevice->CreateSamplerState(&sampDesc, m_samplerLinear.GetAddressOf());
                if (FAILED(hr))
                {
                    return hr;
                }
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetTextureResourceView

          Summary:  Returns the texture resource view

          Returns:  ComPtr<ID3D11ShaderResourceView>&
                      The texture resource view
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11ShaderResourceView>& Renderable::GetTextureResourceView()
        {
            return m_textureRV;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetSamplerState

          Summary:  Returns the sampler state

          Returns:  ComPtr<ID3D11SamplerState>&
                      The sampler state
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11SamplerState>& Renderable::GetSamplerState()
        {
            return m_samplerLinear;
        }

        const XMFLOAT4& Renderable::GetOutputColor() const
        {
            return m_outputColor;
        }

        BOOL Renderable::HasTexture() const
        {
            return m_bHasTextures;
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }
    }
#endif

#ifdef LAB05
    namespace lab05
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::Renderable

          Summary:  Constructor

          Args:     const std::filesystem::path& textureFilePath
                      Path to the texture to use

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_textureRV, m_samplerLinear, m_vertexShader,
                     m_pixelShader, m_textureFilePath, m_world].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderable::Renderable(const std::filesystem::path& textureFilePath)
            : m_vertexBuffer()
            , m_indexBuffer()
            , m_constantBuffer()
            , m_textureRV()
            , m_samplerLinear()
            , m_vertexShader()
            , m_pixelShader()
            , m_textureFilePath(textureFilePath)
            //, m_padding{ '\0' }
            , m_world(XMMatrixIdentity())
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers
                    PCWSTR pszTextureFileName
                      File name of the texture to use

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_world].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(
            _In_ ID3D11Device* pDevice, 
            _In_ ID3D11DeviceContext* pImmediateContext
            )
        {
            // Create vertex buffer
            D3D11_BUFFER_DESC bd = 
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData = 
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;
            pImmediateContext->IASetVertexBuffers(0u, 1u, m_vertexBuffer.GetAddressOf(), &uStride, &uOffset);

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set index buffer
            pImmediateContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

            // Create the constant buffers
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(CBChangesEveryFrame);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Load the Texture
            hr = CreateDDSTextureFromFile(pDevice, m_textureFilePath.filename().wstring().c_str(), nullptr, m_textureRV.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the sample state
            D3D11_SAMPLER_DESC sampDesc =
            {
                .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
                .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
                .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
                .ComparisonFunc = D3D11_COMPARISON_NEVER,
                .MinLOD = 0,
                .MaxLOD = D3D11_FLOAT32_MAX
            };
            hr = pDevice->CreateSamplerState(&sampDesc, m_samplerLinear.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the world matrix
            m_world = XMMatrixIdentity();

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetTextureResourceView

          Summary:  Returns the texture resource view

          Returns:  ComPtr<ID3D11ShaderResourceView>&
                      The texture resource view
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11ShaderResourceView>& Renderable::GetTextureResourceView()
        {
            return m_textureRV;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetSamplerState

          Summary:  Returns the sampler state

          Returns:  ComPtr<ID3D11SamplerState>&
                      The sampler state
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11SamplerState>& Renderable::GetSamplerState()
        {
            return m_samplerLinear;
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }
    }
#endif

#ifdef ASS01
    namespace ass01
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                     m_world].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
        {
            // Create vertex buffer
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;
            pImmediateContext->IASetVertexBuffers(0u, 1u, m_vertexBuffer.GetAddressOf(), &uStride, &uOffset);

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set index buffer
            pImmediateContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

            // Create the constant buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(ConstantBuffer);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the world matrix
            m_world = XMMatrixIdentity();

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }
    }
#endif

#ifdef LAB04
    namespace lab04
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::initialize

          Summary:  Initializes the buffers and the world matrix

          Args:     ID3D11Device* pDevice
                      The Direct3D device to create the buffers
                    ID3D11DeviceContext* pImmediateContext
                      The Direct3D context to set buffers

          Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer, 
                     m_world].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderable::initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
        {
            UNREFERENCED_PARAMETER(pImmediateContext);

            // Create vertex buffer
            D3D11_BUFFER_DESC bd = 
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData = 
            {
                .pSysMem = getVertices()
            };
            HRESULT hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create index buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * GetNumIndices();        // 36 vertices needed for 12 triangles in a triangle list
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            initData.pSysMem = getIndices();
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the constant buffer
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(ConstantBuffer);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            hr = pDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the world matrix
            m_world = XMMatrixIdentity();

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetVertexShader

          Summary:  Sets the vertex shader to be used for this renderable 
                    object

          Args:     const std::shared_ptr<VertexShader>& vertexShader
                      Vertex shader to set to

          Modifies: [m_vertexShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            m_vertexShader = vertexShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::SetPixelShader

          Summary:  Sets the pixel shader to be used for this renderable
                    object

          Args:     const std::shared_ptr<PixelShader>& pixelShader
                      Pixel shader to set to

          Modifies: [m_pixelShader].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            m_pixelShader = pixelShader;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11VertexShader>&
                      Vertex shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
        {
            return m_vertexShader->GetVertexShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetPixelShader

          Summary:  Returns the vertex shader

          Returns:  ComPtr<ID3D11PixelShader>&
                      Pixel shader. Could be a nullptr
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
        {
            return m_pixelShader->GetPixelShader();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexLayout

          Summary:  Returns the vertex input layout

          Returns:  ComPtr<ID3D11InputLayout>&
                      Vertex input layout
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
        {
            return m_vertexShader->GetVertexLayout();
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetVertexBuffer

          Summary:  Returns the vertex buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Vertex buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
        {
            return m_vertexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetIndexBuffer

          Summary:  Returns the index buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Index buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
        {
            return m_indexBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetConstantBuffer

          Summary:  Returns the constant buffer

          Returns:  ComPtr<ID3D11Buffer>&
                      Constant buffer
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
        {
            return m_constantBuffer;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderable::GetWorldMatrix

          Summary:  Returns the world matrix

          Returns:  const XMMATRIX&
                      World matrix
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        const XMMATRIX& Renderable::GetWorldMatrix() const
        {
            return m_world;
        }

        void Renderable::RotateX(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationX(angle);
        }

        void Renderable::RotateY(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationY(angle);
        }

        void Renderable::RotateZ(_In_ FLOAT angle)
        {
            m_world *= XMMatrixRotationZ(angle);
        }

        void Renderable::RotateRollPitchYaw(_In_ FLOAT pitch, _In_ FLOAT yaw, _In_ FLOAT roll)
        {
            m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        }

        void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
        {
            m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
        }

        void Renderable::Translate(_In_ const XMVECTOR& offset)
        {
            m_world *= XMMatrixTranslationFromVector(offset);
        }
    }
#endif
}
