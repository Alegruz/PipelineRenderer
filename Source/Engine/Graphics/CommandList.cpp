#include "pch.h"

#include "Graphics/CommandList.h"

namespace pr
{
	std::map<std::wstring, ID3D12Resource*> CommandList::ms_TextureCache;
	std::mutex CommandList::ms_TextureCacheMutex;

	CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type) noexcept
		: m_Type(type)
	{
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

	void CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT uSubresource) noexcept
	{
		TransitionBarrier(resource, stateAfter, uSubresource, FALSE);
	}

	void CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		TransitionBarrier(resource, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, FALSE);
	}

	void CommandList::UavBarrier(const Resource& resource) noexcept
	{
		UavBarrier(resource, FALSE);
	}

	void CommandList::AliasingBarrier(const Resource& beforeResource, const Resource& afterResource) noexcept
	{
		AliasingBarrier(beforeResource, afterResource, FALSE);
	}

	void CommandList::ResolveSubresource(Resource& dst, const Resource& src, UINT uDstSubresource) noexcept
	{
		ResolveSubresource(dst, src, uDstSubresource, 0);
	}

	void CommandList::ResolveSubresource(Resource& dst, const Resource& src) noexcept
	{
		ResolveSubresource(dst, src, 0, 0);
	}

	void CommandList::LoadTextureFromFile(Texture& texture, const std::wstring& szFileName) noexcept
	{
		LoadTextureFromFile(texture, szFileName, eTextureUsage::ALBEDO);
	}

	void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth) noexcept
	{
		ClearDepthStencilTexture(texture, clearFlags, depth, 0);
	}

	void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags) noexcept
	{
		ClearDepthStencilTexture(texture, clearFlags, 1.0f, 0);
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

	void CommandList::Draw(UINT uVertexCount, UINT uInstanceCount, UINT uStartVertex) noexcept
	{
		Draw(uVertexCount, uInstanceCount, uStartVertex, 0);
	}

	void CommandList::Draw(UINT uVertexCount, UINT uInstanceCount) noexcept
	{
		Draw(uVertexCount, uInstanceCount, 0, 0);
	}

	void CommandList::Draw(UINT uVertexCount) noexcept
	{
		Draw(uVertexCount, 1, 0, 0);
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