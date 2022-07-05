/*+===================================================================
  File:      MATERIAL.H

  Summary:   Material header file contains declaration of class
			 Material used to abstract multiple texture data.

  Classes:  Material

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include "Texture/Texture.h"

namespace pr
{
	class Material
	{
	public:
		Material() = delete;
		Material(_In_ std::wstring szName);
		Material(const Material& other) = default;
		Material(Material&& other) = default;
		Material& operator=(const Material& other) = default;
		Material& operator=(Material&& other) = default;
		virtual ~Material() = default;

		virtual HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList);

		std::wstring GetName() const;

	public:
		std::shared_ptr<Texture> pDiffuse;
		std::shared_ptr<Texture> pSpecularExponent;
		std::shared_ptr<Texture> pNormal;

	private:
		std::wstring m_szName;
	};
}