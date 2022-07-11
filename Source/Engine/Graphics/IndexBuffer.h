#pragma once

#include "Graphics/Buffer.h"

namespace pr
{
	class IndexBuffer : public Buffer
	{
	public:
		explicit IndexBuffer() noexcept;
		explicit IndexBuffer(const std::wstring& szName) noexcept;
		explicit IndexBuffer(const IndexBuffer& other) noexcept = default;
		explicit IndexBuffer(IndexBuffer&& other) noexcept = default;
		IndexBuffer& operator=(const IndexBuffer& other) noexcept = default;
		IndexBuffer& operator=(IndexBuffer&& other) noexcept = default;
		virtual ~IndexBuffer() noexcept = default;

		virtual void CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept override;

		size_t GetNumIndices() const noexcept;
		DXGI_FORMAT GetFormat() const noexcept;
		virtual D3D12_INDEX_BUFFER_VIEW GetView() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept override;

	private:
		size_t m_NumIndices;
		D3D12_INDEX_BUFFER_VIEW m_View;
	};
}