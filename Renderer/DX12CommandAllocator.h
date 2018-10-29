#pragma once

#include <d3d12.h>

#include "DX12Device.h"
#include "DX12CommandList.h"

namespace renderer {
	namespace dx12 {
		namespace utils {

			class DX12CommandAllocator
			{
			public:
				DX12CommandAllocator(Device* device, D3D12_COMMAND_LIST_TYPE type);
				~DX12CommandAllocator();

				ID3D12CommandAllocator* GetCommandAllocator();

				DX12CommandList* CreateCommandList(D3D12_COMMAND_LIST_TYPE type);

				void Reset();

			protected:
				Device * device;
				ID3D12CommandAllocator * allocator;
			};

		}
	}
}