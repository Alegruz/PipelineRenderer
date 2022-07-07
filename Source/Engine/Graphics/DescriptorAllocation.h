#pragma once

#include "pch.h"

namespace pr
{
	class DescriptorAllocatorPage;

	class DescriptorAllocation final
	{
	public:
		explicit DescriptorAllocation() noexcept;
		explicit DescriptorAllocation(_In_ D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor, size_t numHandles, size_t descriptorSize, const std::shared_ptr<DescriptorAllocatorPage>& pPage) noexcept;
		explicit DescriptorAllocation(const DescriptorAllocation& other) noexcept = delete;
		explicit DescriptorAllocation(DescriptorAllocation&& other) noexcept;
		DescriptorAllocation& operator=(const DescriptorAllocation& other) noexcept = delete;
		DescriptorAllocation& operator=(DescriptorAllocation&& other) noexcept;
		~DescriptorAllocation() noexcept;

		BOOL IsNull() const noexcept;
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(_In_opt_ size_t offset) const noexcept;
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const noexcept;
		size_t GetNumHandles() const noexcept;
		std::shared_ptr<DescriptorAllocatorPage>& GetDescriptorAllocatorPage() noexcept;
		const std::shared_ptr<DescriptorAllocatorPage>& GetDescriptorAllocatorPage() const noexcept;

	private:
		void free() noexcept;

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_hDescriptor;
		size_t m_NumHandles;
		size_t m_DescriptorSize;
		std::shared_ptr<DescriptorAllocatorPage> m_pPage;
	};
}