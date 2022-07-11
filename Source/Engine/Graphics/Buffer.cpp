#include "pch.h"
#include "Graphics/Buffer.h"

namespace pr
{
	Buffer::Buffer(const std::wstring& szName) noexcept
		: Resource(szName)
	{
	}

	Buffer::Buffer(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, size_t numElements, size_t elementSize, const std::wstring& szName) noexcept
		: Resource(pDevice, desc, nullptr, szName)
	{
		CreateViews(pDevice, numElements, elementSize);
	}

	Buffer::Buffer(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, size_t numElements, size_t elementSize) noexcept
		: Buffer(pDevice, desc, numElements, elementSize, L"")
	{
	}
}
