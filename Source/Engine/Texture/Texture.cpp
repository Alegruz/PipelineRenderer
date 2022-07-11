#include "pch.h"

#include "Texture/Texture.h"

#include "DirectXTex/DirectXTex.h"
#include "Graphics/DescriptorAllocator.h"
#include "Graphics/ResourceStateTracker.h"
#include "Texture/DDSTextureLoader.h"
#include "Texture/WICTextureLoader.h"
#include "Utility/Utility.h"

namespace pr
{
	BOOL Texture::CheckSrvSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept
	{
		return ((formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0 || (formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) != 0);
	}

	BOOL Texture::CheckRtvSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept
	{
		return ((formatSupport & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) != 0);
	}

	BOOL Texture::CheckUavSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept
	{
		return ((formatSupport & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0);
	}

	BOOL Texture::CheckDsvSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept
	{
		return ((formatSupport & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) != 0);
	}

	BOOL Texture::IsUavCompatibleFormat(DXGI_FORMAT format) noexcept
	{
		return 0;
	}

	BOOL Texture::IsSRgbFormat(DXGI_FORMAT format) noexcept
	{
		return 0;
	}

	BOOL Texture::IsBgrFormat(DXGI_FORMAT format) noexcept
	{
		return 0;
	}

	BOOL Texture::IsDepthFormat(DXGI_FORMAT format) noexcept
	{
		return 0;
	}

	DXGI_FORMAT Texture::GetTypelessFormat(DXGI_FORMAT format) noexcept
	{
		return DXGI_FORMAT();
	}

	Texture::Texture() noexcept
		: Texture(eTextureUsage::ALBEDO, L"")
	{
	}

	Texture::Texture(eTextureUsage textureUsage) noexcept
		: Texture(textureUsage, L"")
	{
	}

	Texture::Texture(eTextureUsage textureUsage, const std::wstring& szName) noexcept
		: Resource(szName)
		, m_Srvs()
		, m_Uavs()
		, m_SrvMutex()
		, m_UavMutex()
		, m_Rtv()
		, m_Dsv()
		, m_TextureUsage(textureUsage)
	{
	}

	Texture::Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc) noexcept
		: Texture(pDevice, desc, nullptr, eTextureUsage::ALBEDO, L"")
	{
	}

	Texture::Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue) noexcept
		: Texture(pDevice, desc, pClearValue, eTextureUsage::ALBEDO, L"")
	{
	}

	Texture::Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue, eTextureUsage textureUsage) noexcept
		: Texture(pDevice, desc, pClearValue, textureUsage, L"")
	{
	}

	Texture::Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue, eTextureUsage textureUsage, const std::wstring& szName) noexcept
		: Resource(pDevice, desc, pClearValue, szName)
		, m_Srvs()
		, m_Uavs()
		, m_SrvMutex()
		, m_UavMutex()
		, m_Rtv()
		, m_Dsv()
		, m_TextureUsage(textureUsage)
	{
	}

	Texture::Texture(ComPtr<ID3D12Resource> resource) noexcept
		: Texture(resource, eTextureUsage::ALBEDO, L"")
	{
	}

	Texture::Texture(ComPtr<ID3D12Resource> resource, eTextureUsage textureUsage) noexcept
		: Texture(resource, textureUsage, L"")
	{
	}

	Texture::Texture(ComPtr<ID3D12Resource> resource, eTextureUsage textureUsage, const std::wstring& szName) noexcept
		: Resource(resource, szName)
		, m_Srvs()
		, m_Uavs()
		, m_SrvMutex()
		, m_UavMutex()
		, m_Rtv()
		, m_Dsv()
		, m_TextureUsage(textureUsage)
	{
	}

	Texture::Texture(const Texture& other) noexcept
		: Resource(other)
		, m_Srvs()
		, m_Uavs()
		, m_SrvMutex()
		, m_UavMutex()
		, m_Rtv()
		, m_Dsv()
		, m_TextureUsage()
	{
	}

	Texture::Texture(Texture&& other) noexcept
		: Resource(std::move(other))
		, m_Srvs()
		, m_Uavs()
		, m_SrvMutex()
		, m_UavMutex()
		, m_Rtv()
		, m_Dsv()
		, m_TextureUsage()
	{
		CreateViews();
	}

	Texture& Texture::operator=(const Texture& other) noexcept
	{
		Resource::operator=(other);

		if (this != &other)
		{
			CreateViews();
		}

		return *this;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		Resource::operator=(std::move(other));

		if (this != &other)
		{
			CreateViews();
		}

		return *this;
	}

	HRESULT Texture::Initialize(_In_ ID3D12Device* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList)
	{
		ScratchImage image;
		TexMetadata metadata;
		HRESULT hr = S_OK;

		WCHAR ext[_MAX_EXT] = {};
		_wsplitpath_s(m_filePath.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

		if (_wcsicmp(ext, L".dds") == 0)
		{
			hr = GetMetadataFromDDSFile(m_filePath.c_str(), DDS_FLAGS_NONE, metadata);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Getting metadata from DDS file");
			hr = LoadFromDDSFile(m_filePath.c_str(), DDS_FLAGS_NONE, nullptr, image);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Loading from DDS file");
		}
		else if (_wcsicmp(ext, L".tga") == 0)
		{
			hr = GetMetadataFromTGAFile(m_filePath.c_str(), TGA_FLAGS_NONE, metadata);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Getting metadata from TGA file");
			hr = LoadFromTGAFile(m_filePath.c_str(), nullptr, image);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Loading from TGA file");
		}
		else if (_wcsicmp(ext, L".hdr") == 0)
		{
			hr = GetMetadataFromHDRFile(m_filePath.c_str(), metadata);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Getting metadata from HDR file");
			hr = LoadFromHDRFile(m_filePath.c_str(), nullptr, image);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Loading from HDR file");
		}
		else
		{
			hr = GetMetadataFromWICFile(m_filePath.c_str(), WIC_FLAGS_NONE, metadata);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Getting metadata from HDR file");
			hr = LoadFromWICFile(m_filePath.c_str(), WIC_FLAGS_NONE, nullptr, image);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Loading from WIC file");
		}

		//ID3D12Resource* pResource = nullptr;
		hr = CreateTexture(pDevice, image.GetMetadata(), &m_pTextureResource);
		CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Creating texture");

		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		hr = PrepareUpload(
			pDevice, 
			image.GetImages(), 
			image.GetImageCount(), 
			metadata,
			subresources
		);
		CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Prepare uploading");

		// upload is implemented by application developer. Here's one solution using <d3dx12.h>
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(
			m_pTextureResource.Get(),
			0, 
			static_cast<unsigned int>(subresources.size())
		);

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		hr = pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pTextureUploadHeap));
		CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Creating committed resource");

		UpdateSubresources(
			pCommandList,
			m_pTextureResource.Get(),
			m_pTextureUploadHeap.Get(),
			0, 
			0, 
			static_cast<unsigned int>(subresources.size()),
			subresources.data()
		);

		return hr;
	}

	HRESULT Texture::Initialize(ID3D12Device2* pDevice, DescriptorAllocator& allocator)
	{
		HRESULT hr = S_OK;

		if (m_pResource)
		{
			CD3DX12_RESOURCE_DESC desc(m_pResource->GetDesc());

			D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport =
			{
				.Format = desc.Format
			};
			hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));

			if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 &&
				CheckRtvSupport(formatSupport.Support1))
			{
				hr = allocator.Allocate(m_Rtv, pDevice);
				CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Allocating render target view")

				pDevice->CreateRenderTargetView(m_pResource.Get(), nullptr, m_Rtv.GetDescriptorHandle());
			}
			if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 &&
				CheckDsvSupport(formatSupport.Support1))
			{
				hr = allocator.Allocate(m_Dsv, pDevice);
				CHECK_AND_RETURN_HRESULT(hr, L"Texture::Initialize >> Allocating depth stencil view")
				pDevice->CreateDepthStencilView(m_pResource.Get(), nullptr, m_Dsv.GetDescriptorHandle());
			}
		}

		std::lock_guard<std::mutex> lock(m_SrvMutex);
		std::lock_guard<std::mutex> guard(m_UavMutex);

		m_Srvs.clear();
		m_Uavs.clear();

		return hr;
	}

	eTextureUsage Texture::GetTextureUsage() const noexcept
	{
		return m_TextureUsage;
	}

	void Texture::SetTextureUsage(eTextureUsage textureUsage) noexcept
	{
		m_TextureUsage = textureUsage;
	}

	HRESULT Texture::Resize(ID3D12Device2* pDevice, UINT uWidth, UINT uHeight, UINT uDepthOrArraySize) noexcept
	{
		HRESULT hr = S_OK;

		if (m_pResource)
		{
			ResourceStateTracker::RemoveGlobalResourceState(m_pResource.Get());

			CD3DX12_RESOURCE_DESC resDesc(m_pResource->GetDesc());

			resDesc.Width = std::max(uWidth, 1u);
			resDesc.Height = std::max(uHeight, 1u);
			resDesc.DepthOrArraySize = uDepthOrArraySize;

			hr = pDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_COMMON,
				m_pClearValue.get(),
				IID_PPV_ARGS(&m_pResource)
			);
			CHECK_AND_RETURN_HRESULT(hr, L"Texture::Resize >> Creating committed resource");

			// Retain the name of the resource if one was already specified.
			m_pResource->SetName(m_szResourceName.c_str());

			ResourceStateTracker::AddGlobalResourceState(m_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);

			CreateViews(pDevice);
		}

		return hr;
	}

	HRESULT Texture::Resize(ID3D12Device2* pDevice, UINT uWidth, UINT uHeight) noexcept
	{
		return Resize(pDevice, uWidth, uHeight, 1);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		HRESULT hr = S_OK;
		size_t hash = 0;

		if (pSrvDesc)
		{
			hash = std::hash<D3D12_SHADER_RESOURCE_VIEW_DESC>{}(*pSrvDesc);
		}

		std::lock_guard<std::mutex> lock(m_SrvMutex);

		auto iter = m_Srvs.find(hash);
		if (iter == m_Srvs.end())
		{
			DescriptorAllocation srv;
			hr = createSrv(srv, pDevice, allocator, pSrvDesc);
		}

		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
	
	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc) const noexcept
	{
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
	
	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetRtv() const noexcept
	{
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
	
	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetDsv() const noexcept
	{
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	HRESULT Texture::createSrv(DescriptorAllocation& srv, ID3D12Device2* pDevice, DescriptorAllocator& allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		HRESULT hr = S_OK;
		hr = allocator.Allocate(srv, pDevice);
		CHECK_AND_RETURN_HRESULT(hr, L"Texture::createSrv >> Creating shader resource view");

		pDevice->CreateShaderResourceView(m_pResource.Get(), pSrvDesc, srv.GetDescriptorHandle());

		return hr;
	}

	HRESULT Texture::createUav(DescriptorAllocation& uav, ID3D12Device2* pDevice, DescriptorAllocator& allocator, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc) const noexcept
	{
		HRESULT hr = S_OK;
		hr = allocator.Allocate(uav, pDevice);
		CHECK_AND_RETURN_HRESULT(hr, L"Texture::createSrv >> Creating unordered access view");

		pDevice->CreateShaderResourceView(m_pResource.Get(), nullptr, pUavDesc, uav.GetDescriptorHandle());

		return hr;
	}
}