#include "DX12CommandAllocator.h"

#include <iostream>


namespace renderer {
	namespace dx12 {
		namespace utils {
			
			DX12CommandAllocator::DX12CommandAllocator(Device * device, D3D12_COMMAND_LIST_TYPE type)
			{
				this->device = device;
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

			DX12CommandList * DX12CommandAllocator::CreateCommandList(D3D12_COMMAND_LIST_TYPE type)
			{
				ID3D12GraphicsCommandList* commandList;
				HRESULT hr = device->GetDevice()->CreateCommandList(0, type, allocator, NULL, IID_PPV_ARGS(&commandList));
				if (FAILED(hr)) {
					std::cout << "[ERROR] Creating command list failed with error code " << hr << std::endl;
					return nullptr;
				}

				return new DX12CommandList(commandList);
			}

			void DX12CommandAllocator::Reset()
			{
				allocator->Reset();
			}

		}
	}
}