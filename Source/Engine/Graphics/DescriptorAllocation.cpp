#include "pch.h"

#include "Game/Game.h"
#include "Graphics/DescriptorAllocation.h"
#include "Graphics/DescriptorAllocatorPage.h"

namespace pr
{
	DescriptorAllocation::DescriptorAllocation() noexcept
		: m_hDescriptor{ 0 }
		, m_NumHandles(0)
		, m_DescriptorSize(0)
		, m_pPage()
	{
	}

	DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor, size_t numHandles, size_t descriptorSize, const std::shared_ptr<DescriptorAllocatorPage>& pPage) noexcept
		: m_hDescriptor(hDescriptor)
		, m_NumHandles(numHandles)
		, m_DescriptorSize(descriptorSize)
		, m_pPage(pPage)
	{
	}

	DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& other) noexcept
		: m_hDescriptor(other.m_hDescriptor)
		, m_NumHandles(other.m_NumHandles)
		, m_DescriptorSize(other.m_DescriptorSize)
		, m_pPage(std::move(other.m_pPage))
	{
		other.m_hDescriptor.ptr = 0;
		other.m_NumHandles = 0;
		other.m_DescriptorSize = 0;
	}

	DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other) noexcept
	{
		if (this != &other)
		{
			free();

			m_hDescriptor = other.m_hDescriptor;
			m_NumHandles = other.m_NumHandles;
			m_DescriptorSize = other.m_DescriptorSize;
			m_pPage = std::move(other.m_pPage);

			other.m_hDescriptor.ptr = 0;
			other.m_NumHandles = 0;
			other.m_DescriptorSize = 0;
		}

		return *this;
	}

	DescriptorAllocation::~DescriptorAllocation() noexcept
	{
		free();
	}

	BOOL DescriptorAllocation::IsNull() const noexcept
	{
		return m_hDescriptor.ptr == 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle(size_t offset) const noexcept
	{
		assert(offset < m_NumHandles);

		return D3D12_CPU_DESCRIPTOR_HANDLE{ .ptr = m_hDescriptor.ptr + (m_DescriptorSize * offset) };
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle() const noexcept
	{
		return GetDescriptorHandle(0);
	}

	size_t DescriptorAllocation::GetNumHandles() const noexcept
	{
		return m_NumHandles;
	}

	std::shared_ptr<DescriptorAllocatorPage>& DescriptorAllocation::GetDescriptorAllocatorPage() noexcept
	{
		return m_pPage;
	}

	const std::shared_ptr<DescriptorAllocatorPage>& DescriptorAllocation::GetDescriptorAllocatorPage() const noexcept
	{
		return m_pPage;
	}

	void DescriptorAllocation::free() noexcept
	{
		if (!IsNull() && m_pPage)
		{
			m_pPage->Free(std::move(*this), Game::GetNumFrames());

			m_hDescriptor.ptr = 0;
			m_NumHandles = 0;
			m_DescriptorSize = 0;
			m_pPage.reset();
		}
	}
}