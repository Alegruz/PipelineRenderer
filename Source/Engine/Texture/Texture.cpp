#include "pch.h"

#include "Texture/Texture.h"

#include "DirectXTex/DirectXTex.h"
#include "Texture/DDSTextureLoader.h"
#include "Texture/WICTextureLoader.h"
#include "Utility/Utility.h"

namespace pr
{
	Texture::Texture(_In_ const std::filesystem::path& filePath)
		: m_filePath(filePath)
		//, m_textureRV()
		//, m_samplerLinear()
		, m_pTextureResource()
		, m_pTextureUploadHeap()
	{
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
}