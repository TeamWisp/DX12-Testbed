#include "DX12CommandList.h"


namespace renderer {
	namespace dx12 {
		namespace utils {

			DX12CommandList::DX12CommandList()
			{
			}


			DX12CommandList::DX12CommandList(ID3D12CommandList * commandList)
			{
				this->commandList = commandList;
			}

			DX12CommandList::~DX12CommandList()
			{
				commandList->Release();
			}

		}
	}
}