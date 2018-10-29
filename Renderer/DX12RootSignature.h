#pragma once
#include <d3d12.h>

#include "DX12Device.h"

namespace renderer {
	namespace dx12 {
		namespace utils {
			class DX12RootSignature
			{
			public:
				DX12RootSignature(Device* device);
				~DX12RootSignature();

				ID3D12RootSignature* GetRootSignature();

				int Compile(UINT nodeMask);

				void SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags);

				void AddRootParameter(D3D12_ROOT_PARAMETER parameter);

				void AddStaticSampler(D3D12_STATIC_SAMPLER_DESC sampler);

			protected:
				Device* device;

				std::vector<D3D12_ROOT_PARAMETER> rootSignatureParameters;
				std::vector<D3D12_STATIC_SAMPLER_DESC> rootSignatureStaticSamplerDescriptions;
				D3D12_ROOT_SIGNATURE_FLAGS flags;

				ID3D12RootSignature * rootSignature;
			};

		}
	}
}