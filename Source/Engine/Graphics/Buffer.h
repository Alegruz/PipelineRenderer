#pragma once

#include "pch.h"

#include "Graphics/Resource.h"

namespace pr
{
	class Buffer : public Resource
	{
	public:
		explicit Buffer() noexcept = default;
		explicit Buffer(const std::wstring& szName) noexcept;
		explicit Buffer(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc,
			size_t numElements,
			size_t elementSize,
			const std::wstring& szName
		) noexcept;
		explicit Buffer(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc,
			size_t numElements,
			size_t elementSize
		) noexcept;
		explicit Buffer(const Buffer& other) noexcept = default;
		explicit Buffer(Buffer&& other) noexcept = default;
		Buffer& operator=(const Buffer& other) noexcept = default;
		Buffer& operator=(Buffer&& other) noexcept = default;
		virtual ~Buffer() noexcept = default;

		virtual void CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept = 0;
	};
}