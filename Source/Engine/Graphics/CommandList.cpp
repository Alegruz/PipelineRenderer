#include "pch.h"

#include "Graphics/ByteAddressBuffer.h"
#include "Graphics/CommandList.h"
#include "Graphics/DynamicDescriptorHeap.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Resource.h"
#include "Graphics/ResourceStateTracker.h"
#include "Graphics/StructuredBuffer.h"
#include "Graphics/UploadBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Texture/Texture.h"
#include "Utility/Utility.h"

namespace pr
{
	std::map<std::wstring, ID3D12Resource*> CommandList::ms_TextureCache;
	std::mutex CommandList::ms_TextureCacheMutex;

	CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type) noexcept
		: m_Type(type)
		, m_pCommandList()
		, m_pCommandAllocator()
		, m_pComputeCommandList()
		, m_pRootSignature()
		, m_pUploadBuffer()
		, m_pResourceStateTracker()
		, m_apDynamicDescriptorHeap{}
		, m_apDescriptorHeaps{ nullptr, }
		, m_pGenerateMipsPso()
		, m_pPanoToCubeMapPso()
		, m_TrackedObjects()
	{
	}

	HRESULT CommandList::Initialize(ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		hr = pDevice->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&m_pCommandAllocator));
		CHECK_AND_RETURN_HRESULT(hr, L"CommandList::Initialize >> Creating command allocator");

		hr = pDevice->CreateCommandList(0, m_Type, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList));
		CHECK_AND_RETURN_HRESULT(hr, L"CommandList::Initialize >> Creating command list");

		m_pUploadBuffer = std::make_unique<UploadBuffer>();
		m_pResourceStateTracker = std::make_unique<ResourceStateTracker>();

		for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_apDynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
		}

		return hr;
	}

	D3D12_COMMAND_LIST_TYPE CommandList::GetType() const noexcept
	{
		return m_Type;
	}

	ComPtr<ID3D12GraphicsCommandList2>& CommandList::GetCommandList() noexcept
	{
		return m_pCommandList;
	}

	const ComPtr<ID3D12GraphicsCommandList2>& CommandList::GetCommandList() const noexcept
	{
		return m_pCommandList;
	}

	void CommandList::TransitBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT uSubresource, BOOL bFlushBarriers) noexcept
	{
		const ComPtr<ID3D12Resource>& pResource = resource.GetD3D12Resource();

		if (pResource)
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, uSubresource);
			m_pResourceStateTracker->PushResourceBarrier(barrier);
		}

		if (bFlushBarriers)
		{
			FlushResourceBarriers();
		}
	}

	void CommandList::TransitBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT uSubresource) noexcept
	{
		TransitBarrier(resource, stateAfter, uSubresource, FALSE);
	}

	void CommandList::TransitBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		TransitBarrier(resource, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, FALSE);
	}

	void CommandList::AddUavBarrier(const Resource& resource, BOOL bFlushBarriers) noexcept
	{
		const ComPtr<ID3D12Resource>& pResource = resource.GetD3D12Resource();
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(pResource.Get());

		m_pResourceStateTracker->PushResourceBarrier(barrier);

		if (bFlushBarriers)
		{
			FlushResourceBarriers();
		}
	}

	void CommandList::AddUavBarrier(const Resource& resource) noexcept
	{
		AddUavBarrier(resource, FALSE);
	}

	void CommandList::AddAliasingBarrier(const Resource& beforeResource, const Resource& afterResource, BOOL bFlushBarriers) noexcept
	{
		const ComPtr<ID3D12Resource>& pBeforeResource = beforeResource.GetD3D12Resource();
		const ComPtr<ID3D12Resource>& pAfterResource = afterResource.GetD3D12Resource();
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(pBeforeResource.Get(), pAfterResource.Get());

		m_pResourceStateTracker->PushResourceBarrier(barrier);

		if (bFlushBarriers)
		{
			FlushResourceBarriers();
		}
	}

	void CommandList::AddAliasingBarrier(const Resource& beforeResource, const Resource& afterResource) noexcept
	{
		AddAliasingBarrier(beforeResource, afterResource, FALSE);
	}

	void CommandList::FlushResourceBarriers() noexcept
	{
		m_pResourceStateTracker->FlushPendingResourceBarriers(*this);
	}

	void CommandList::CopyResource(Resource& dst, const Resource& src) noexcept
	{
		TransitBarrier(dst, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE);

		FlushResourceBarriers();

		m_pCommandList->CopyResource(dst.GetD3D12Resource().Get(), src.GetD3D12Resource().Get());

		trackResource(dst);
		trackResource(src);
	}

	void CommandList::ResolveSubresource(Resource& dst, const Resource& src, UINT uDstSubresource, UINT uSrcSubresource) noexcept
	{
		TransitBarrier(dst, D3D12_RESOURCE_STATE_RESOLVE_DEST, uDstSubresource);
		TransitBarrier(src, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, uSrcSubresource);

		FlushResourceBarriers();

		m_pCommandList->ResolveSubresource(
			dst.GetD3D12Resource().Get(),
			uDstSubresource,
			src.GetD3D12Resource().Get(),
			uSrcSubresource,
			dst.GetD3D12ResourceDesc().Format
		);

		trackResource(src);
		trackResource(dst);
	}

	void CommandList::ResolveSubresource(Resource& dst, const Resource& src, UINT uDstSubresource) noexcept
	{
		ResolveSubresource(dst, src, uDstSubresource, 0);
	}

	void CommandList::ResolveSubresource(Resource& dst, const Resource& src) noexcept
	{
		ResolveSubresource(dst, src, 0, 0);
	}

	void CommandList::CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* pVertexBufferData) noexcept
	{
		copyBuffer(vertexBuffer, numVertices, vertexStride, pVertexBufferData);
	}

	void CommandList::CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* pIndexBufferData) noexcept
	{
		size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
		copyBuffer(indexBuffer, numIndices, indexSizeInBytes, pIndexBufferData);
	}

	void CommandList::CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* pBufferData) noexcept
	{
		copyBuffer(byteAddressBuffer, 1, bufferSize, pBufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}

	void CommandList::CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* pBufferData) noexcept
	{
		copyBuffer(structuredBuffer, numElements, elementSize, pBufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}

	void CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology) noexcept
	{
		m_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	}

	HRESULT CommandList::LoadTextureFromFile(Texture& outTexture, ID3D12Device2* pDevice, const std::wstring& szFileName, eTextureUsage textureUsage) noexcept
	{
		HRESULT hr = S_OK;

		std::filesystem::path filePath(szFileName);

		if (!std::filesystem::exists(filePath))
		{
			hr = E_INVALIDARG;
			CHECK_AND_RETURN_HRESULT(hr, L"CommandList::LoadTextureFromFile >> file path does not exist");
		}

		auto iter = ms_TextureCache.find(szFileName);
		if (iter != ms_TextureCache.end())
		{
			texture.Set
		}

		return hr;
	}

	HRESULT CommandList::LoadTextureFromFile(Texture& outTexture, ID3D12Device2* pDevice, const std::wstring& szFileName) noexcept
	{
		return LoadTextureFromFile(outTexture, pDevice, szFileName, eTextureUsage::ALBEDO);
	}

	void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth) noexcept
	{
		ClearDepthStencilTexture(texture, clearFlags, depth, 0);
	}

	void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags) noexcept
	{
		ClearDepthStencilTexture(texture, clearFlags, 1.0f, 0);
	}

	HRESULT CommandList::SetGraphicsDynamicConstantBuffer(_In_ ID3D12Device2* pDevice, UINT uRootParameterIndex, size_t sizeInBytes, const void* pBufferData) noexcept
	{
		HRESULT hr = S_OK;
		
		UploadBuffer::Allocation heapAllocation;
		hr = m_pUploadBuffer->Allocate(heapAllocation, pDevice, sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		CHECK_AND_RETURN_HRESULT(hr, L"CommandList::SetGraphicsDynamicConstantBuffer >> Allocating from upload buffer");

		memcpy(heapAllocation.pCpu, pBufferData, sizeInBytes);

		m_pCommandList->SetGraphicsRootConstantBufferView(uRootParameterIndex, heapAllocation.Gpu);

		return hr;
	}

	void CommandList::SetShaderResourceView(
		UINT uRootParameterIndex, 
		UINT uDescriptorOffset, 
		const Resource& resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT uFirstSubresource, 
		UINT uNumSubResources, 
		const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrv
	) noexcept
	{
		if (uNumSubResources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			for (UINT i = 0; i < uNumSubResources; ++i)
			{
				TransitBarrier(resource, stateAfter, uFirstSubresource + i);
			}
		}
		else
		{
			TransitBarrier(resource, stateAfter);
		}

		m_apDynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(uRootParameterIndex, uDescriptorOffset, 1, resource.GetSrv(pSrv));

		trackResource(resource);
	}

	void CommandList::SetShaderResourceView(
		UINT uRootParameterIndex, 
		UINT uDescriptorOffset, 
		const Resource& resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT uFirstSubresource, 
		UINT uNumSubResources
	) noexcept
	{
		SetShaderResourceView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			stateAfter,
			uFirstSubresource,
			uNumSubResources,
			nullptr
		);
	}

	void CommandList::SetShaderResourceView(
		UINT uRootParameterIndex, 
		UINT uDescriptorOffset, 
		const Resource& resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT uFirstSubresource
	) noexcept
	{
		SetShaderResourceView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			stateAfter,
			uFirstSubresource,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			nullptr
		);
	}

	void CommandList::SetShaderResourceView(UINT uRootParameterIndex, UINT uDescriptorOffset, const Resource& resource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		SetShaderResourceView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			stateAfter,
			0,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			nullptr
		);
	}

	void CommandList::SetShaderResourceView(UINT uRootParameterIndex, UINT uDescriptorOffset, const Resource& resource) noexcept
	{
		SetShaderResourceView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			0,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			nullptr
		);
	}

	void CommandList::SetUnorderedAccessView(
		UINT uRootParameterIndex, 
		UINT uDescriptorOffset, 
		const Resource& resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT uFirstSubresource, 
		UINT uNumSubResources, 
		const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUav
	) noexcept
	{
	}

	void CommandList::SetUnorderedAccessView(
		UINT uRootParameterIndex, 
		UINT uDescriptorOffset, 
		const Resource& resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT uFirstSubresource, 
		UINT uNumSubResources
	) noexcept
	{
		SetUnorderedAccessView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			stateAfter,
			uFirstSubresource,
			uNumSubResources,
			nullptr
		);
	}

	void CommandList::SetUnorderedAccessView(
		UINT uRootParameterIndex, 
		UINT uDescriptorOffset, 
		const Resource& resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT uFirstSubresource
	) noexcept
	{
		SetUnorderedAccessView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			stateAfter,
			uFirstSubresource,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			nullptr
		);
	}

	void CommandList::SetUnorderedAccessView(UINT uRootParameterIndex, UINT uDescriptorOffset, const Resource& resource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		SetUnorderedAccessView(
			uRootParameterIndex,
			uDescriptorOffset,
			resource,
			stateAfter,
			0,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			nullptr
		);
	}

	void CommandList::SetUnorderedAccessView(UINT uRootParameterIndex, UINT uDescriptorOffset, const Resource& resource) noexcept
	{
		SetUnorderedAccessView(
			uRootParameterIndex, 
			uDescriptorOffset, 
			resource, 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
			0, 
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, 
			nullptr
		);
	}

	HRESULT CommandList::Draw(ID3D12Device2* pDevice, UINT uVertexCount, UINT uInstanceCount, UINT uStartVertex, UINT uStartInstance) noexcept
	{
		HRESULT hr = S_OK;
		FlushResourceBarriers();

		for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			hr = m_apDynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this, pDevice);
			CHECK_AND_RETURN_HRESULT(hr, L"CommandList::Draw >> Commit staged descriptors for draw");
		}

		m_pCommandList->DrawInstanced(uVertexCount, uInstanceCount, uStartVertex, uStartInstance);

		return hr;
	}

	HRESULT CommandList::Draw(ID3D12Device2* pDevice, UINT uVertexCount, UINT uInstanceCount, UINT uStartVertex) noexcept
	{
		return Draw(pDevice, uVertexCount, uInstanceCount, uStartVertex, 0);
	}

	HRESULT CommandList::Draw(ID3D12Device2* pDevice, UINT uVertexCount, UINT uInstanceCount) noexcept
	{
		return Draw(pDevice, uVertexCount, uInstanceCount, 0, 0);
	}

	HRESULT CommandList::Draw(ID3D12Device2* pDevice, UINT uVertexCount) noexcept
	{
		return Draw(pDevice, uVertexCount, 1, 0, 0);
	}

	void CommandList::Dispatch(UINT uNumGroupsX, UINT uNumGroupsY) noexcept
	{
		Dispatch(uNumGroupsX, uNumGroupsY, 1);
	}

	void CommandList::Dispatch(UINT uNumGroupsX) noexcept
	{
		Dispatch(uNumGroupsX, 1, 1);
	}

	void CommandList::copyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* pBufferData) noexcept
	{
		copyBuffer(buffer, numElements, elementSize, pBufferData, D3D12_RESOURCE_FLAG_NONE);
	}
}