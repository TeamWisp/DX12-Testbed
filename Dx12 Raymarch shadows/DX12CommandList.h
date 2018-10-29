#pragma once

#include <d3d12.h>

namespace renderer {
	namespace dx12 {
		namespace utils {
			class DX12CommandList
			{
			public:
				DX12CommandList(ID3D12CommandList* commandList);
				~DX12CommandList();

			protected:
				ID3D12CommandList * commandList;
			};

		}
	}
}