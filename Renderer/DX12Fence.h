#pragma once

#include "DX12Device.h"
#include <d3d12.h>

namespace renderer {
	namespace dx12 {
		namespace utils {

			class DX12Fence
			{
			public:
				DX12Fence(Device* device, UINT64 initialValue, UINT64 onCompletionValue);
				~DX12Fence();

				void SetFenceEvent(UINT64 onCompletionValue, HANDLE fenceEvent);

				UINT64 GetValue();

				void IncrementValue();

				void SetValue(UINT64 value);

				UINT64 GetCompletedValue();

				ID3D12Fence* GetFence();

				void Signal(UINT64 value);
			protected:
				ID3D12Fence * fence;
				HANDLE fenceEvent;

				UINT64 value;

			};

		}
	}
}