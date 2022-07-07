#include "pch.h"

#include "Graphics/RootSignature.h"
#include "Utility/Utility.h"

namespace pr
{
	RootSignature::RootSignature() noexcept
		: m_Desc{}
		, m_RootSignature()
		, m_aNumDescriptorsPerTable{ 0, }
		, m_uSamplerTableBitMask(0)
		, m_uDescriptorTableBitMask(0)
	{
	}

	RootSignature::RootSignature(ID3D12Device2* pDevice, const D3D12_ROOT_SIGNATURE_DESC1& desc, D3D_ROOT_SIGNATURE_VERSION version) noexcept
		: RootSignature()
	{
		HRESULT hr = SetRootSignatureDesc(pDevice, desc, version);
		AssertHresult(hr, L"RootSignature Constructor >> Setting RS description");
	}

	RootSignature::~RootSignature() noexcept
	{
		Destroy();
	}

	void RootSignature::Destroy()
	{
		for (UINT i = 0; i < m_Desc.NumParameters; ++i)
		{
			const D3D12_ROOT_PARAMETER1& rootParameter = m_Desc.pParameters[i];
			if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				delete[] rootParameter.DescriptorTable.pDescriptorRanges;
			}
		}

		delete[] m_Desc.pParameters;
		m_Desc.pParameters = nullptr;
		m_Desc.NumParameters = 0;

		delete[] m_Desc.pStaticSamplers;
		m_Desc.pStaticSamplers = nullptr;
		m_Desc.NumStaticSamplers = 0;

		m_uDescriptorTableBitMask = 0;
		m_uSamplerTableBitMask = 0;

		memset(m_aNumDescriptorsPerTable, 0, sizeof(m_aNumDescriptorsPerTable));
	}

	ComPtr<ID3D12RootSignature>& RootSignature::GetRootSignature() noexcept
	{
		return m_RootSignature;
	}
	const ComPtr<ID3D12RootSignature>& RootSignature::GetRootSignature() const noexcept
	{
		return m_RootSignature;
	}

	const D3D12_ROOT_SIGNATURE_DESC1& RootSignature::GetRootSignatureDesc() const noexcept
	{
		return m_Desc;
	}

	UINT RootSignature::GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type) const noexcept
	{
		UINT descriptorTableBitMask = 0;
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			descriptorTableBitMask = m_uDescriptorTableBitMask;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			descriptorTableBitMask = m_uSamplerTableBitMask;
			break;
		default:
			assert(false);
			break;
		}

		return descriptorTableBitMask;
	}

	size_t RootSignature::GetNumDescriptors(size_t rootIndex) const noexcept
	{
		assert(rootIndex < NUM_ITEM);
		return m_aNumDescriptorsPerTable[rootIndex];
	}

	HRESULT RootSignature::SetRootSignatureDesc(ID3D12Device2* pDevice, const D3D12_ROOT_SIGNATURE_DESC1& desc, D3D_ROOT_SIGNATURE_VERSION version) noexcept
	{
		HRESULT hr = S_OK;
		Destroy();

		UINT uNumParameters = desc.NumParameters;
		D3D12_ROOT_PARAMETER1* pParameters = uNumParameters > 0 ? new D3D12_ROOT_PARAMETER1[uNumParameters] : nullptr;

		for (UINT i = 0; i < uNumParameters; ++i)
		{
			const D3D12_ROOT_PARAMETER1& rootParameter = desc.pParameters[i];
			pParameters[i] = rootParameter;

			if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				UINT uNumDescriptorRanges = rootParameter.DescriptorTable.NumDescriptorRanges;
				D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = uNumDescriptorRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[uNumDescriptorRanges] : nullptr;

				memcpy(pDescriptorRanges, rootParameter.DescriptorTable.pDescriptorRanges, sizeof(D3D12_DESCRIPTOR_RANGE1) * uNumDescriptorRanges);

				pParameters[i].DescriptorTable.NumDescriptorRanges = uNumDescriptorRanges;
				pParameters[i].DescriptorTable.pDescriptorRanges = pDescriptorRanges;

				if (uNumDescriptorRanges > 0)
				{
					switch (pDescriptorRanges[0].RangeType)
					{
					case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
						[[fallthrough]];
					case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
						[[fallthrough]];
					case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
						m_uDescriptorTableBitMask |= (1 << i);
						break;
					case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
						m_uSamplerTableBitMask |= (1 << i);
						break;
					default:
						assert(false);
						break;
					}
				}

				for (UINT j = 0; j < uNumDescriptorRanges; ++j)
				{
					m_aNumDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors;
				}
			}
		}

		m_Desc.NumParameters = uNumParameters;
		m_Desc.pParameters = pParameters;

		UINT uNumStaticSamplers = desc.NumStaticSamplers;
		D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = uNumStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[uNumStaticSamplers] : nullptr;

		if (pStaticSamplers)
		{
			memcpy(pStaticSamplers, desc.pStaticSamplers, sizeof(D3D12_STATIC_SAMPLER_DESC) * uNumStaticSamplers);
		}

		m_Desc.NumStaticSamplers = uNumStaticSamplers;
		m_Desc.pStaticSamplers = pStaticSamplers;

		D3D12_ROOT_SIGNATURE_FLAGS flags = desc.Flags;
		m_Desc.Flags = flags;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc;
		versionedDesc.Init_1_1(uNumParameters, pParameters, uNumStaticSamplers, pStaticSamplers, flags);

		ComPtr<ID3DBlob> pRootSignatureBlob;
		ComPtr<ID3DBlob> pErrorBlob;
		hr = D3DX12SerializeVersionedRootSignature(&versionedDesc, version, &pRootSignatureBlob, &pErrorBlob);
		CHECK_AND_RETURN_HRESULT(hr, L"RootSignature::SetRootSignatureDesc >> Serializing versioned RS");

		hr = pDevice->CreateRootSignature(0, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		CHECK_AND_RETURN_HRESULT(hr, L"RootSignature::SetRootSignatureDesc >> Creating RS");

		return hr;
	}
}