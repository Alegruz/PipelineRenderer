/*+===================================================================
  File:      V0XEL.H

  Summary:   Voxel header file contains declarations of Voxel class
             used for the lab samples of Game Graphics Programming
             course.

  Classes: Voxel

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include <fstream>

//#include "Model/Model.h"
//#include "Light/PointLight.h"
#include "Graphics/Renderable.h"
#include "Texture/Material.h"

namespace pr
{
    class Scene
    {
    public:
        explicit Scene() noexcept = default;
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = delete;
        virtual ~Scene() = default;

        virtual HRESULT Initialize(_In_ ID3D12Device2* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList);

        HRESULT AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable);
        HRESULT AddMaterial(_In_ const std::shared_ptr<Material>& material);

        void Update(_In_ FLOAT deltaTime);

        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>& GetRenderables() noexcept;
        const std::unordered_map<std::wstring, std::shared_ptr<Renderable>>& GetRenderables() const noexcept;
        std::unordered_map<std::wstring, std::shared_ptr<Material>>& GetMaterials();

    private:
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>> m_renderables;
        std::unordered_map<std::wstring, std::shared_ptr<Material>> m_materials;
    };
}