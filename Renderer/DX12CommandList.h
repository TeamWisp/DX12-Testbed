#pragma once

#include <d3d12.h>

#include <vector>

namespace renderer {
	namespace dx12 {
		namespace utils {
			class DX12CommandAllocator;

			class DX12RootSignature;

			class DX12CommandList
			{
			public:
				DX12CommandList(ID3D12GraphicsCommandList* commandList);
				~DX12CommandList();

				void Reset(DX12CommandAllocator* allocator, ID3D12PipelineState* pipelineState);

				void ResourceBarrier(std::vector<D3D12_RESOURCE_BARRIER> barriers);

				/// <summary>
				/// This function sets the list of render targets to the list provided by the vector. This is slower than using a contiguous chunk of descriptors.
				/// </summary>
				/// <param name="renderTargetDescriptors">A vector of descriptor handles for render targets.</param>
				/// <param name="depthStencilDescriptor">A pointer to a descriptor handle for the depth stencil attachment.</param>
				void OMSetRenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);
				
				/// /// <summary>
				/// This function sets the list of render targets by using the renderTargetDescriptors variable as a pointer to the beginning of a contiguous chunk of descriptors
				/// in a descriptor heap. This is faster than providing a vector with descriptors.
				/// </summary>
				/// <param name="descriptorCount">The number of descriptors.</param>
				/// <param name="renderTargetDescriptors">A pointer to the beginning of a continguous chunk of descriptors in a heap.</param>
				/// <param name="depthStencilDescriptor">A pointer to a descriptor handle for the depth stencil attachment.</param>
				void OMSetRenderTargets(UINT descriptorCount, D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);

				void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, FLOAT colorRGBA[4], UINT numRects, D3D12_RECT* rects);

				void SetGraphicsRootSignature(DX12RootSignature* rootSignature);

				void RSSetViewports(std::vector<D3D12_VIEWPORT> viewports);

				void RSSetScissorRects(std::vector<D3D12_RECT> scissorRects);

				void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

				void IASetVertexBuffers(unsigned int startSlot, std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews);

				void DrawInstanced(unsigned int vertexCount, unsigned int instanceCount, unsigned int startVertex, unsigned int startInstance);

				void CopyBufferRegion(ID3D12Resource* pDstBuffer, uint64_t dstOffset, ID3D12Resource* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes);

				void Close();

				ID3D12GraphicsCommandList* GetCommandList();

			protected:
				ID3D12GraphicsCommandList * commandList;

				bool closed;
			};

		}
	}
}