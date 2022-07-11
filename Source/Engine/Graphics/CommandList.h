#pragma once

#include "pch.h"

#include "Texture/TextureUsage.h"

namespace pr
{
	class Buffer;
	class ByteAddressBuffer;
	class DynamicDescriptorHeap;
	class GenerateMipsPso;
	class IndexBuffer;
	class PanoToCubeMapPso;
	class RenderTarget;
	class Resource;
	class ResourceStateTracker;
	class RootSignature;
	class StructuredBuffer;
	class Texture;
	class UploadBuffer;
	class VertexBuffer;

	class CommandList
	{
	public:
		CommandList() = delete;
		explicit CommandList(_In_ D3D12_COMMAND_LIST_TYPE type) noexcept;
		explicit CommandList(_In_ const CommandList& other) noexcept = default;
		explicit CommandList(_In_ CommandList&& other) noexcept = default;
		CommandList& operator=(_In_ const CommandList& other) noexcept = default;
		CommandList& operator=(_In_ CommandList&& other) noexcept = default;
		virtual ~CommandList() noexcept = default;

		HRESULT Initialize(_In_ ID3D12Device2* pDevice) noexcept;

		D3D12_COMMAND_LIST_TYPE GetType() const noexcept;
		ComPtr<ID3D12GraphicsCommandList2>& GetCommandList() noexcept;
		const ComPtr<ID3D12GraphicsCommandList2>& GetCommandList() const noexcept;

		void TransitBarrier(_In_ const Resource& resource, _In_ D3D12_RESOURCE_STATES stateAfter, _In_opt_ UINT uSubresource, _In_opt_ BOOL bFlushBarriers) noexcept;
		void TransitBarrier(_In_ const Resource& resource, _In_ D3D12_RESOURCE_STATES stateAfter, _In_opt_ UINT uSubresource) noexcept;
		void TransitBarrier(_In_ const Resource& resource, _In_ D3D12_RESOURCE_STATES stateAfter) noexcept;
		void AddUavBarrier(_In_ const Resource& resource, _In_opt_ BOOL bFlushBarriers) noexcept;
		void AddUavBarrier(_In_ const Resource& resource) noexcept;
		void AddAliasingBarrier(const Resource& beforeResource, const Resource& afterResource, BOOL bFlushBarriers) noexcept;
		void AddAliasingBarrier(const Resource& beforeResource, const Resource& afterResource) noexcept;
		void FlushResourceBarriers() noexcept;
		void CopyResource(Resource& dst, const Resource& src) noexcept;
		void ResolveSubresource(Resource& dst, const Resource& src, UINT uDstSubresource, UINT uSrcSubresource) noexcept;
		void ResolveSubresource(Resource& dst, const Resource& src, UINT uDstSubresource) noexcept;
		void ResolveSubresource(Resource& dst, const Resource& src) noexcept;

		void CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* pVertexBufferData) noexcept;
		template <typename T>
		void CopyVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData) noexcept;
		void CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* pIndexBufferData) noexcept;
		template <typename T>
		void CopyIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData) noexcept;
		void CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* pBufferData) noexcept;
		template <typename T>
		void CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, const T& data) noexcept;
		void CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* pBufferData) noexcept;
		template <typename T>
		void CopyStructuredBuffer(StructuredBuffer& structuredBuffer, const std::vector<T>& bufferData) noexcept;

		void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology) noexcept;
		HRESULT LoadTextureFromFile(_Out_ Texture& outTexture, _In_ ID3D12Device2* pDevice, _In_ const std::wstring& szFileName, _In_ eTextureUsage textureUsage) noexcept;
		HRESULT LoadTextureFromFile(_Out_ Texture& outTexture, _In_ ID3D12Device2* pDevice, _In_ const std::wstring& szFileName) noexcept;
		void ClearTexture(const Texture& texture, const FLOAT clearColor[4]) noexcept;
		void ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 uStencil) noexcept;
		void ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth) noexcept;
		void ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags) noexcept;
		void GenerateMips(Texture& texture) noexcept;
		void PanoToCubeMap(Texture& cubeMap, const Texture& pano) noexcept;
		void CopyTextureSubresource(Texture& texture, UINT uFirstSubresource, UINT uNumSubresource, D3D12_SUBRESOURCE_DATA* pSubresourceData) noexcept;

		HRESULT SetGraphicsDynamicConstantBuffer(_In_ ID3D12Device2* pDevice, _In_ UINT uRootParameterIndex, _In_ size_t sizeInBytes, _In_ const void* pBufferData) noexcept;
		template <typename T>
		HRESULT SetGraphicsDynamicConstantBuffer(_In_ ID3D12Device2* pDevice, _In_ UINT uRootParameterIndex, _In_ const T& data) noexcept;
		void SetGraphics32BitConstants(UINT uRootParameterIndex, size_t numConstants, const void* pConstants) noexcept;
		template <typename T>
		void SetGraphics32BitConstants(UINT uRootParameterIndex, const T& constants) noexcept;
		void SetCompute32BitConstants(UINT uRootParameterIndex, size_t numConstants, const void* pConstants) noexcept;
		template <typename T>
		void SetCompute32BitConstants(UINT uRootParameterIndex, const T& constants) noexcept;

		void SetVertexBuffer(UINT uSlot, const VertexBuffer& vertexBuffer) noexcept;
		
		void SetDynamicVertexBuffer(UINT uSlot, size_t numVertices, size_t vertexSize, const void* pVertexBufferData) noexcept;
		template <typename T>
		void SetDynamicVertexBuffer(UINT uSlot, const std::vector<T>& vertexBufferData) noexcept;

		void SetIndexBuffer(const IndexBuffer& indexBuffer) noexcept;

		void SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* pIndexBufferData) noexcept;
		template <typename T>
		void SetDynamicIndexBuffer(const std::vector<T>& indexBufferData) noexcept;
		void SetGraphicsDynamicStructuredBuffer(UINT uSlot, size_t numElements, size_t elementSize, const void* pBufferData) noexcept;
		template <typename T>
		void SetGraphicsDynamicStructuredBuffer(UINT uSlot, const std::vector<T>& bufferData) noexcept;

		void SetViewport(const D3D12_VIEWPORT& viewport) noexcept;
		void SetViewports(const std::vector<D3D12_VIEWPORT>& viewports) noexcept;

		void SetScissorRect(const D3D12_RECT& scissorsRect) noexcept;
		void SetScissorRectS(const std::vector<D3D12_RECT>& scissorsRects) noexcept;

		void SetPipelineState(const ComPtr<ID3D12PipelineState>& pPipelineState) noexcept;

		void SetGraphicsRootSignature(const RootSignature& rootSignature) noexcept;
		void SetComputeRootSignature(const RootSignature& rootSignature) noexcept;

		void SetShaderResourceView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT uFirstSubresource,
			UINT uNumSubResources,
			const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrv
		) noexcept;
		void SetShaderResourceView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT uFirstSubresource,
			UINT uNumSubResources
		) noexcept;
		void SetShaderResourceView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT uFirstSubresource
		) noexcept;
		void SetShaderResourceView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter
		) noexcept;
		void SetShaderResourceView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource
		) noexcept;

		void SetUnorderedAccessView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT uFirstSubresource,
			UINT uNumSubResources,
			const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUav
		) noexcept;
		void SetUnorderedAccessView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT uFirstSubresource,
			UINT uNumSubResources
		) noexcept;
		void SetUnorderedAccessView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT uFirstSubresource
		) noexcept;
		void SetUnorderedAccessView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter
		) noexcept;
		void SetUnorderedAccessView(
			UINT uRootParameterIndex,
			UINT uDescriptorOffset,
			const Resource& resource
		) noexcept;

		void SetRenderTarget(const RenderTarget& renderTarget) noexcept;

		HRESULT Draw(_In_ ID3D12Device2* pDevice, _In_ UINT uVertexCount, _In_ UINT uInstanceCount, _In_ UINT uStartVertex, _In_ UINT uStartInstance) noexcept;
		HRESULT Draw(_In_ ID3D12Device2* pDevice, _In_ UINT uVertexCount, _In_ UINT uInstanceCount, _In_ UINT uStartVertex) noexcept;
		HRESULT Draw(_In_ ID3D12Device2* pDevice, _In_ UINT uVertexCount, _In_ UINT uInstanceCount) noexcept;
		HRESULT Draw(_In_ ID3D12Device2* pDevice, _In_ UINT uVertexCount) noexcept;
		void Dispatch(_In_ UINT uNumGroupsX, _In_ UINT uNumGroupsY, _In_ UINT uNumGroupsZ) noexcept;
		void Dispatch(_In_ UINT uNumGroupsX, _In_ UINT uNumGroupsY) noexcept;
		void Dispatch(_In_ UINT uNumGroupsX) noexcept;

		BOOL Close(CommandList& pendingCommandList) noexcept;
		void Close();
		void Reset();
		void ReleaseTrackedObjects() noexcept;
		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* pHeap) noexcept;
		std::shared_ptr<CommandList>& GetGenerateMipsCommandList() noexcept;
		const std::shared_ptr<CommandList>& GetGenerateMipsCommandList() const noexcept;

	private:
		static std::map<std::wstring, ID3D12Resource*> ms_TextureCache;
		static std::mutex ms_TextureCacheMutex;

	private:
		void trackObject(ComPtr<ID3D12Object>& pObject) noexcept;
		void trackResource(const Resource& resource) noexcept;
		void generateMipsUav(Texture& texture) noexcept;
		void generateMipsBgr(Texture& texture) noexcept;
		void generateMipssRgb(Texture& texture) noexcept;
		void copyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* pBufferData, D3D12_RESOURCE_FLAGS flags) noexcept;
		void copyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* pBufferData) noexcept;
		void bindDescriptorHeaps() noexcept;

		using TrackedObjects = std::vector<ComPtr<ID3D12Object>>;

		D3D12_COMMAND_LIST_TYPE m_Type;
		ComPtr<ID3D12GraphicsCommandList2> m_pCommandList;
		ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
		std::shared_ptr<CommandList> m_pComputeCommandList;
		ID3D12RootSignature* m_pRootSignature;
		std::unique_ptr<UploadBuffer> m_pUploadBuffer;
		std::unique_ptr<ResourceStateTracker> m_pResourceStateTracker;
		std::unique_ptr<DynamicDescriptorHeap> m_apDynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		ID3D12DescriptorHeap* m_apDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		std::unique_ptr<GenerateMipsPso> m_pGenerateMipsPso;
		std::unique_ptr<PanoToCubeMapPso> m_pPanoToCubeMapPso;
		TrackedObjects m_TrackedObjects;
	};

	template<typename T>
	inline void CommandList::CopyVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData) noexcept
	{
		CopyVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
	}

	template<typename T>
	inline void CommandList::CopyIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData) noexcept
	{
		assert(sizeof(T) == 2 || sizeof(T) == 4);

		DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		CopyIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData.data());
	}

	template<typename T>
	inline void CommandList::CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, const T& data) noexcept
	{
		CopyByteAddressBuffer(byteAddressBuffer, sizeof(T), &data);
	}

	template<typename T>
	inline void CommandList::CopyStructuredBuffer(StructuredBuffer& structuredBuffer, const std::vector<T>& bufferData) noexcept
	{
		CopyStructuredBuffer(structuredBuffer, bufferData.size(), sizeof(T), bufferData.data());
	}

	template<typename T>
	inline HRESULT CommandList::SetGraphicsDynamicConstantBuffer(_In_ ID3D12Device2* pDevice, UINT uRootParameterIndex, const T& data) noexcept
	{
		return SetGraphicsDynamicConstantBuffer(pDevice, uRootParameterIndex, sizeof(T), &data);
	}

	template<typename T>
	inline void CommandList::SetGraphics32BitConstants(UINT uRootParameterIndex, const T& constants) noexcept
	{
		static_assert(sizeof(T)) % sizeof(UINT) == 0, "Size of type must be a multiple of 4 bytes.");
		SetGraphics32BitConstants(uRootParameterIndex, sizeof(T) / sizeof(UINT), &constants);
	}

	template<typename T>
	inline void CommandList::SetCompute32BitConstants(UINT uRootParameterIndex, const T& constants) noexcept
	{
		static_assert(sizeof(T)) % sizeof(UINT) == 0, "Size of type must be a multiple of 4 bytes.");
		SetCompute32BitConstants(uRootParameterIndex, sizeof(T) / sizeof(UINT), &constants);
	}

	template<typename T>
	inline void CommandList::SetDynamicVertexBuffer(UINT uSlot, const std::vector<T>& vertexBufferData) noexcept
	{
		SetDynamicVertexBuffer(uSlot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
	}

	template<typename T>
	inline void CommandList::SetDynamicIndexBuffer(const std::vector<T>& indexBufferData) noexcept
	{
		assert(sizeof(T) == 2 || sizeof(T) == 4);

		DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
	}

	template<typename T>
	inline void CommandList::SetGraphicsDynamicStructuredBuffer(UINT uSlot, const std::vector<T>& bufferData) noexcept
	{
		SetGraphicsDynamicStructuredBuffer(uSlot, bufferData.size(), sizeof(T), bufferData.data());
	}
}