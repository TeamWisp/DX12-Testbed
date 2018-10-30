#include "DX12CommandList.h"
#include "DX12CommandAllocator.h"
#include "DX12RootSignature.h"

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

			void DX12CommandList::SetGraphicsRootSignature(DX12RootSignature * rootSignature)
			{
				if (!closed) {
					commandList->SetGraphicsRootSignature(rootSignature->GetRootSignature());
				}
			}

			void DX12CommandList::RSSetViewports(std::vector<D3D12_VIEWPORT> viewports)
			{
				if (!closed) {
					commandList->RSSetViewports(viewports.size(), viewports.data());
				}
			}

			void DX12CommandList::RSSetScissorRects(std::vector<D3D12_RECT> scissorRects)
			{
				if (!closed) {
					commandList->RSSetScissorRects(scissorRects.size(), scissorRects.data());
				}
			}

			void DX12CommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
			{
				if (!closed) {
					commandList->IASetPrimitiveTopology(topology);
				}
			}

			void DX12CommandList::IASetVertexBuffers(unsigned int startSlot, std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews)
			{
				if (!closed) {
					commandList->IASetVertexBuffers(startSlot, vertexBufferViews.size(), vertexBufferViews.data());
				}
			}

			void DX12CommandList::DrawInstanced(unsigned int vertexCount, unsigned int instanceCount, unsigned int startVertex, unsigned int startInstance)
			{
				if (!closed) {
					commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
				}
			}

			void DX12CommandList::CopyBufferRegion(ID3D12Resource * pDstBuffer, uint64_t dstOffset, ID3D12Resource * pSrcBuffer, uint64_t srcOffset, uint64_t numBytes)
			{
				if (!closed) {
					commandList->CopyBufferRegion(pDstBuffer, dstOffset, pSrcBuffer, srcOffset, numBytes);
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