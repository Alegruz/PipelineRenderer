#include "pch.h"

#include "Graphics/CommandList.h"
#include "Graphics/Resource.h"
#include "Graphics/ResourceStateTracker.h"

namespace pr
{
	ResourceStateTracker::ResourceStateMap ResourceStateTracker::ms_GlobalResourceState;
	std::mutex ResourceStateTracker::ms_GlobalMutex;
	BOOL ResourceStateTracker::ms_bIsLocked = FALSE;

	void ResourceStateTracker::Lock()
	{
		ms_GlobalMutex.lock();
		ms_bIsLocked = TRUE;
	}

	void ResourceStateTracker::Unlock()
	{
		ms_GlobalMutex.unlock();
		ms_bIsLocked = FALSE;
	}

	void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* pResource, D3D12_RESOURCE_STATES state) noexcept
	{
		if (pResource)
		{
			std::lock_guard<std::mutex> lock(ms_GlobalMutex);
			ms_GlobalResourceState[pResource].SetSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
		}
	}

	void ResourceStateTracker::RemoveGlobalResourceState(ID3D12Resource* pResource) noexcept
	{
		if (pResource)
		{
			std::lock_guard<std::mutex> lock(ms_GlobalMutex);
			ms_GlobalResourceState.erase(pResource);
		}
	}

	void ResourceStateTracker::PushResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier) noexcept
	{
		if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

			const auto iter = m_FinalResourceState.find(transitionBarrier.pResource);

			if (iter != m_FinalResourceState.end())
			{
				ResourceState& resourceState = iter->second;

				if (transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !resourceState.SubresourceState.empty())
				{
					for (std::pair<UINT, D3D12_RESOURCE_STATES> subresourceState : resourceState.SubresourceState)
					{
						if (transitionBarrier.StateAfter != subresourceState.second)
						{
							D3D12_RESOURCE_BARRIER newBarrier = barrier;
							newBarrier.Transition.Subresource = subresourceState.first;
							newBarrier.Transition.StateBefore = subresourceState.second;
							m_ResourceBarriers.push_back(newBarrier);
						}
					}
				}
				else
				{
					D3D12_RESOURCE_STATES finalState = resourceState.GetSubresourceState(transitionBarrier.Subresource);
					if (transitionBarrier.StateAfter != finalState)
					{
						D3D12_RESOURCE_BARRIER newBarrier = barrier;
						newBarrier.Transition.StateBefore = finalState;
						m_ResourceBarriers.push_back(newBarrier);
					}
				}
			}
			else
			{
				m_PendingResourceBarriers.push_back(barrier);
			}

			m_FinalResourceState[transitionBarrier.pResource].SetSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);
		}
		else
		{
			m_ResourceBarriers.push_back(barrier);
		}
	}

	void ResourceStateTracker::TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateAfter, UINT uSubResource) noexcept
	{
		if (pResource)
		{
			PushResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(pResource, D3D12_RESOURCE_STATE_COMMON, stateAfter, uSubResource));
		}
	}

	void ResourceStateTracker::TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		TransitResource(pResource, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	}

	void ResourceStateTracker::TransitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT uSubResource) noexcept
	{
		TransitResource(resource.GetD3D12Resource().Get(), stateAfter, uSubResource);
	}

	void ResourceStateTracker::TransitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		TransitResource(resource, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	}

	void ResourceStateTracker::PushUavBarrier(const Resource* pResource) noexcept
	{
		ID3D12Resource* pD3D12Resource = pResource ? pResource->GetD3D12Resource().Get() : nullptr;

		PushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pD3D12Resource));
	}

	void ResourceStateTracker::PushUavBarrier() noexcept
	{
		PushUavBarrier(nullptr);
	}

	void ResourceStateTracker::PushAliasBarrier(const Resource* pResourceBefore, const Resource* pResourceAfter) noexcept
	{
		ID3D12Resource* pD3D12ResourceBefore = pResourceBefore ? pResourceBefore->GetD3D12Resource().Get() : nullptr;
		ID3D12Resource* pD3D12ResourceAfter = pResourceAfter ? pResourceAfter->GetD3D12Resource().Get() : nullptr;

		PushResourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pD3D12ResourceBefore, pD3D12ResourceAfter));
	}

	void ResourceStateTracker::PushAliasBarrier(const Resource* pResourceBefore) noexcept
	{
		PushAliasBarrier(pResourceBefore, nullptr);
	}

	void ResourceStateTracker::PushAliasBarrier() noexcept
	{
		PushAliasBarrier(nullptr, nullptr);
	}

	UINT ResourceStateTracker::FlushPendingResourceBarriers(CommandList& commandList) noexcept
	{
		assert(ms_bIsLocked);

		ResourceBarriers resourceBarriers;
		resourceBarriers.reserve(m_PendingResourceBarriers.size());

		for (D3D12_RESOURCE_BARRIER& pendingBarrier : m_PendingResourceBarriers)
		{
			if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
			{
				D3D12_RESOURCE_TRANSITION_BARRIER& pendingTransition = pendingBarrier.Transition;
				const auto& iter = ms_GlobalResourceState.find(pendingTransition.pResource);

				if (iter != ms_GlobalResourceState.end())
				{
					//<ID3D12Resource*, ResourceState>
					ResourceState& resourceState = iter->second;

					if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !resourceState.SubresourceState.empty())
					{
						for (const std::pair<UINT, D3D12_RESOURCE_STATES>& subresourceState : resourceState.SubresourceState)
						{
							if (pendingTransition.StateAfter != subresourceState.second)
							{
								D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
								newBarrier.Transition.Subresource = subresourceState.first;
								newBarrier.Transition.StateBefore = subresourceState.second;
								resourceBarriers.push_back(newBarrier);
							}
						}
					}
					else
					{
						D3D12_RESOURCE_STATES globalState = (iter->second).GetSubresourceState(pendingTransition.Subresource);
						if (pendingTransition.StateAfter != globalState)
						{
							pendingBarrier.Transition.StateBefore = globalState;
							resourceBarriers.push_back(pendingBarrier);
						}
					}
				}
			}
		}

		UINT uNumBarriers = static_cast<UINT>(resourceBarriers.size());
		if (uNumBarriers > 0)
		{
			ComPtr<ID3D12GraphicsCommandList2>& pCommandList = commandList.GetCommandList();
			pCommandList->ResourceBarrier(uNumBarriers, resourceBarriers.data());
		}

		m_PendingResourceBarriers.clear();

		return uNumBarriers;
	}

	void ResourceStateTracker::FlushResourceBarriers(CommandList& commandList) noexcept
	{
		UINT uNumBarriers = static_cast<UINT>(m_ResourceBarriers.size());

		if (uNumBarriers > 0)
		{
			ComPtr<ID3D12GraphicsCommandList2>& pCommandList = commandList.GetCommandList();
			pCommandList->ResourceBarrier(uNumBarriers, m_ResourceBarriers.data());
			m_ResourceBarriers.clear();
		}
	}

	void ResourceStateTracker::CommitFinalResourceStates() noexcept
	{
		assert(ms_bIsLocked);

		for (const std::pair<ID3D12Resource* const, ResourceState>& resourceState : m_FinalResourceState)
		{
			ms_GlobalResourceState[resourceState.first] = resourceState.second;
		}

		m_FinalResourceState.clear();
	}

	void ResourceStateTracker::Reset() noexcept
	{
		m_PendingResourceBarriers.clear();
		m_ResourceBarriers.clear();
		m_FinalResourceState.clear();
	}
}