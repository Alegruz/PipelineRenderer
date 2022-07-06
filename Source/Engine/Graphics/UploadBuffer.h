#pragma once

#include "pch.h"

#include <deque>

namespace pr
{
	class UploadBuffer
	{
	public:
		// Use to upload data to the GPU
		struct Allocation
		{
			void* pCpu;
			D3D12_GPU_VIRTUAL_ADDRESS Gpu;
		};

	public:
		explicit UploadBuffer() noexcept;
		explicit UploadBuffer(_In_ size_t pageSize) noexcept;
		explicit UploadBuffer(_In_ const UploadBuffer& other) noexcept = default;
		explicit UploadBuffer(_In_ UploadBuffer&& other) noexcept = default;
		UploadBuffer& operator=(_In_ const UploadBuffer& other) noexcept = default;
		UploadBuffer& operator=(_In_ UploadBuffer&& other) noexcept = default;
		virtual ~UploadBuffer() noexcept = default;

		size_t GetPageSize() const;
		HRESULT Allocate(_Out_ Allocation& outAllocation, _In_ ID3D12Device2* pDevice, _In_ size_t sizeInBytes, _In_ size_t alignment) noexcept;
		void Reset();

	private:
		struct Page final
		{
		public:
			explicit Page(_In_ size_t sizeInBytes) noexcept;
			explicit Page(_In_ const Page& other) noexcept = default;
			explicit Page(_In_ Page&& other) noexcept = default;
			Page& operator=(_In_ const Page& other) noexcept = default;
			Page& operator=(_In_ Page&& other) noexcept = default;
			~Page() noexcept;

			HRESULT Initialize(_In_ ID3D12Device2* pDevice) noexcept;
			HRESULT Destroy() noexcept;

			BOOL HasSpace(_In_ size_t sizeInBytes, _In_ size_t alignment) const noexcept;
			HRESULT Allocate(_Out_ Allocation& outAllocation, _In_ size_t sizeInBytes, _In_ size_t alignment) noexcept;
			void Reset() noexcept;

		private:
			ComPtr<ID3D12Resource> m_pResource;
			void* m_pCpuPtr;
			D3D12_GPU_VIRTUAL_ADDRESS m_GpuPtr;
			size_t m_PageSize;
			size_t m_Offset;
		};

	private:
		using PagePool = std::deque<std::shared_ptr<Page>>;

	private:
		HRESULT requestPage(_Out_ std::shared_ptr<Page>& pOutPage, _In_ ID3D12Device2* pDevice) noexcept;

	private:
		PagePool m_PagePool;
		PagePool m_AvailablePages;
		std::shared_ptr<Page> m_pCurrentPage;
		size_t m_PageSize;
	};
}
