#pragma once

#include "pch.h"

namespace pr
{
	class CommandList;
	class Resource;

	class ResourceStateTracker
	{
	public:
		explicit ResourceStateTracker() noexcept = default;
		explicit ResourceStateTracker(const ResourceStateTracker& other) noexcept = default;
		explicit ResourceStateTracker(ResourceStateTracker&& other) noexcept = default;
		ResourceStateTracker& operator=(const ResourceStateTracker& other) noexcept = default;
		ResourceStateTracker& operator=(ResourceStateTracker&& other) noexcept = default;
		virtual ~ResourceStateTracker() noexcept = default;

		static void Lock();
		static void Unlock();
		static void AddGlobalResourceState(ID3D12Resource* pResource, D3D12_RESOURCE_STATES state) noexcept;
		static void RemoveGlobalResourceState(ID3D12Resource* pResource) noexcept;

		void PushResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier) noexcept;
		void TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateAfter, UINT uSubResource) noexcept;
		void TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateAfter) noexcept;
		void TransitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT uSubResource) noexcept;
		void TransitResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter) noexcept;
		void PushUavBarrier(const Resource* pResource) noexcept;
		void PushUavBarrier() noexcept;
		void PushAliasBarrier(const Resource* pResourceBefore, const Resource* pResourceAfter) noexcept;
		void PushAliasBarrier(const Resource* pResourceBefore) noexcept;
		void PushAliasBarrier() noexcept;
		UINT FlushPendingResourceBarriers(CommandList& commandList) noexcept;
		void FlushResourceBarriers(CommandList& commandList) noexcept;
		void CommitFinalResourceStates() noexcept;
		void Reset() noexcept;

	private:
		using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

		ResourceBarriers m_PendingResourceBarriers;
		ResourceBarriers m_ResourceBarriers;

		struct ResourceState final
		{
			explicit ResourceState() noexcept;
			explicit ResourceState(D3D12_RESOURCE_STATES state) noexcept;
			explicit ResourceState(const ResourceState& other) noexcept = default;
			explicit ResourceState(ResourceState&& other) noexcept = default;
			ResourceState& operator=(const ResourceState& other) noexcept = default;
			ResourceState& operator=(ResourceState&& other) noexcept = default;
			~ResourceState() noexcept = default;

			void SetSubresourceState(UINT uSubresource, D3D12_RESOURCE_STATES state) noexcept;
			D3D12_RESOURCE_STATES GetSubresourceState(UINT uSubresource) const noexcept;

			D3D12_RESOURCE_STATES State;
			std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
		};

		using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

	private:
		static ResourceStateMap ms_GlobalResourceState;
		static std::mutex ms_GlobalMutex;
		static BOOL ms_bIsLocked;

	private:
		ResourceStateMap m_FinalResourceState;
	};
}