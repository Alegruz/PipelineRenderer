#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/DescriptorAllocation.h"

namespace pr
{
	class DescriptorAllocator;

	class ConstantBuffer : public Buffer
	{
	public:
		ConstantBuffer() = delete;
		explicit ConstantBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept;
		explicit ConstantBuffer(const std::wstring& szName, ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept;
		explicit ConstantBuffer(const ConstantBuffer& other) noexcept = default;
		explicit ConstantBuffer(ConstantBuffer&& other) noexcept = default;
		ConstantBuffer& operator=(const ConstantBuffer& other) noexcept = default;
		ConstantBuffer& operator=(ConstantBuffer&& other) noexcept = default;
		virtual ~ConstantBuffer() noexcept = default;

		virtual void CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept override;

		size_t GetSizeInBytes() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetView() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept override;

	private:
		size_t m_SizeInBytes;
		DescriptorAllocation m_Cbv;
	};
}