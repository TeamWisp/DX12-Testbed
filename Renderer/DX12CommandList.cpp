#include "DX12CommandList.h"
#include "DX12CommandAllocator.h"

namespace renderer {
	namespace dx12 {
		namespace utils {
			
			DX12CommandList::DX12CommandList(ID3D12GraphicsCommandList * commandList)
			{
				this->commandList = commandList;
				this->commandList->Close();
				closed = true;
			}

			DX12CommandList::~DX12CommandList()
			{
				commandList->Release();
			}

			void DX12CommandList::Reset(DX12CommandAllocator* allocator, ID3D12PipelineState* pipelineState)
			{
				commandList->Reset(allocator->GetCommandAllocator(), pipelineState);
				closed = false;
			}

			void DX12CommandList::ResourceBarrier(std::vector<D3D12_RESOURCE_BARRIER> barriers)
			{
				if (!closed) {
					commandList->ResourceBarrier(barriers.size(), barriers.data());
				}
			}

			void DX12CommandList::OMSetRenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE * depthStencilDescriptor)
			{
				if (!closed) {
					commandList->OMSetRenderTargets(renderTargetDescriptors.size(), renderTargetDescriptors.data(), FALSE, depthStencilDescriptor);
				}
			}

			void DX12CommandList::OMSetRenderTargets(UINT descriptorCount, D3D12_CPU_DESCRIPTOR_HANDLE * renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE * depthStencilDescriptor)
			{
				if (!closed) {
					commandList->OMSetRenderTargets(descriptorCount, renderTargetDescriptors, TRUE, depthStencilDescriptor);
				}
			}

			void DX12CommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, FLOAT colorRGBA[4], UINT numRects, D3D12_RECT * rects)
			{
				if (!closed) {
					commandList->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, rects);
				}
			}

			void DX12CommandList::Close()
			{
				commandList->Close();
				closed = true;
			}

			ID3D12GraphicsCommandList * DX12CommandList::GetCommandList()
			{
				return commandList;
			}

		}
	}
}