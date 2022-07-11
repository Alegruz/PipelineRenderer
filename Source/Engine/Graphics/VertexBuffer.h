#pragma once

#include "Graphics/Buffer.h"

namespace pr
{
	class VertexBuffer : public Buffer
	{
	public:
		explicit VertexBuffer() noexcept;
		explicit VertexBuffer(const std::wstring& szName) noexcept;
		explicit VertexBuffer(const VertexBuffer& other) noexcept = default;
		explicit VertexBuffer(VertexBuffer&& other) noexcept = default;
		VertexBuffer& operator=(const VertexBuffer& other) noexcept = default;
		VertexBuffer& operator=(VertexBuffer&& other) noexcept = default;
		virtual ~VertexBuffer() noexcept = default;

		virtual void CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept override;

		size_t GetNumVertices() const noexcept;
		size_t GetStride() const noexcept;
		virtual D3D12_VERTEX_BUFFER_VIEW GetView() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept override;

	private:
		size_t m_NumVertices;
		size_t m_Stride;

		D3D12_VERTEX_BUFFER_VIEW m_View;
	};
}