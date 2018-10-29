#include "DX12RootSignature.h"

#include "D3dx12.h"

#include <iostream>

namespace renderer {
	namespace dx12 {
		namespace utils {

			DX12RootSignature::DX12RootSignature(Device* device)
			{
				this->device = device;
			}


			DX12RootSignature::~DX12RootSignature()
			{
				rootSignature->Release();
			}

			ID3D12RootSignature * DX12RootSignature::GetRootSignature()
			{
				return rootSignature;
			}

			int DX12RootSignature::Compile(UINT nodeMask)
			{
				CD3DX12_ROOT_SIGNATURE_DESC description;
				description.Init(rootSignatureParameters.size(), rootSignatureParameters.data(),
					rootSignatureStaticSamplerDescriptions.size(), rootSignatureStaticSamplerDescriptions.data(),
					flags);

				ID3DBlob* signature;
				ID3DBlob* error;
				HRESULT hr = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
				if (FAILED(hr)) {
					std::cout << "[ERROR] Serializing root signature failed with error " << hr << std::endl;
					OutputDebugStringA((char*)error->GetBufferPointer());
					return hr;
				}

				hr = device->GetDevice()->CreateRootSignature(nodeMask, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&this->rootSignature));

				return 0;
			}

			void DX12RootSignature::SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags)
			{
				this->flags = flags;
			}

			void DX12RootSignature::AddRootParameter(D3D12_ROOT_PARAMETER parameter)
			{
				rootSignatureParameters.push_back(parameter);
			}

			void DX12RootSignature::AddStaticSampler(D3D12_STATIC_SAMPLER_DESC sampler)
			{
				rootSignatureStaticSamplerDescriptions.push_back(sampler);
			}

		}
	}
}