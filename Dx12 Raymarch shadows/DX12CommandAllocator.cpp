#include "DX12CommandAllocator.h"

#include <iostream>


namespace renderer {
	namespace dx12 {
		namespace utils {
			
			DX12CommandAllocator::DX12CommandAllocator(Device * device, D3D12_COMMAND_LIST_TYPE type)
			{
				HRESULT hr = device->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator));
				if (FAILED(hr)) {
					std::cout << "[ERROR] Creating command allocator failed with error code " << hr << std::endl;
				}
			}

			DX12CommandAllocator::~DX12CommandAllocator()
			{
				allocator->Release();
			}

			ID3D12CommandAllocator * DX12CommandAllocator::GetCommandAllocator()
			{
				return allocator;
			}

		}
	}
}