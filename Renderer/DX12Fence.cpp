#include "DX12Fence.h"
#include <iostream>


namespace renderer {
	namespace dx12 {
		namespace utils {

			DX12Fence::DX12Fence(Device* device, UINT64 initialValue, UINT64 onCompletionValue)
			{
				HRESULT hr = device->GetDevice()->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
				if (FAILED(hr)) {
					std::cout << "[ERROR] Creating fence failed with code " << hr << std::endl;
					return;
				}

				this->value = initialValue;

				fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (fenceEvent == nullptr) {
					std::cout << "[ERROR] Creating fence event failed" << std::endl;
					return;
				}
			}


			DX12Fence::~DX12Fence()
			{
				fence->Release();
			}

			void DX12Fence::SetFenceEvent(UINT64 onCompletionValue, HANDLE fenceEvent)
			{
				this->fenceEvent = fenceEvent;
				fence->SetEventOnCompletion(onCompletionValue, fenceEvent);
			}

			UINT64 DX12Fence::GetValue()
			{
				return value;
			}

			void DX12Fence::IncrementValue()
			{
				value++;
			}

			void DX12Fence::SetValue(UINT64 value)
			{
				this->value = value;
			}

			UINT64 DX12Fence::GetCompletedValue()
			{
				return fence->GetCompletedValue();
			}

			ID3D12Fence * DX12Fence::GetFence()
			{
				return fence;
			}

			void DX12Fence::Signal(UINT64 value)
			{
				fence->Signal(value);
			}

		}
	}
}