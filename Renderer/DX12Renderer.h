#pragma once
#include "Renderer.h"
#include "DX12Device.h"
#include "DX12CommandAllocator.h"
#include "DX12CommandList.h"
#include "DX12Fence.h"

#include "DX12PipelineStateObject.h"
#include "DX12RootSignature.h"

#include "DX12MemoryManager.h"

#include "Defines.h"

#pragma comment(lib, "d3d12.lib")

namespace renderer {
	namespace dx12 {

		class DX12Renderer :
			public Renderer
		{
		public:
			DX12Renderer(Window* window);
			virtual ~DX12Renderer();

			virtual int Initialize();

			virtual void RenderStart();
			virtual void RenderEnd();

		protected:
			int CreateSwapChain();

			int CreateRtvDescriptorHeap();

			int CreateCommandAllocators();

			int CreateFences();

			utils::Device* device;

			IDXGISwapChain4* swapChain;

			int frameIndex;

			ID3D12CommandQueue* commandQueue;

			ID3D12DescriptorHeap* rtvDescriptorHeap;

			ID3D12Resource* renderTargets[FRAME_BUFFER_COUNT];
			
			std::vector<utils::DX12CommandAllocator*> commandAllocators;

			std::vector<utils::DX12Fence*> fences;

			HANDLE fenceEvent;

			utils::DX12CommandAllocator* bundleAllocator;

			utils::DX12CommandList* primaryCommandList;

			utils::DX12PipelineStateObject* defaultPSO;
			utils::DX12RootSignature* defaultRootSignature;

			MemoryManager* memoryManager;
		};
	}
}