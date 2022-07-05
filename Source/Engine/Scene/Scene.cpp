#include "pch.h"

#include "Scene/Scene.h"

namespace pr
{
    HRESULT Scene::Initialize(_In_ ID3D12Device2* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList)
    {
        for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
        {
            HRESULT hr = it->second->Initialize(pDevice, pCommandList);
            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
        {
            HRESULT hr = it->second->Initialize(pDevice, pCommandList);
            if (FAILED(hr))
            {
                return hr;
            }
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object and initialize the object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Scene::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        if (m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }

        m_renderables[pszRenderableName] = renderable;

        return S_OK;
    }

    HRESULT Scene::AddMaterial(_In_ const std::shared_ptr<Material>& material)
    {
        if (m_materials.contains(material->GetName()))
        {
            return E_FAIL;
        }

        m_materials[material->GetName()] = material;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Scene::Update(_In_ FLOAT deltaTime)
    {
        for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
        {
            it->second->Update(deltaTime);
        }
    }

    std::unordered_map<std::wstring, std::shared_ptr<Renderable>>& Scene::GetRenderables() noexcept
    {
        return m_renderables;
    }

    const std::unordered_map<std::wstring, std::shared_ptr<Renderable>>& Scene::GetRenderables() const noexcept
    {
        return m_renderables;
    }

    std::unordered_map<std::wstring, std::shared_ptr<Material>>& Scene::GetMaterials()
    {
        return m_materials;
    }
}