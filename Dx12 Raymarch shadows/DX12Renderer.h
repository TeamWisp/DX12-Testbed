#pragma once
#include "Renderer.h"
#include "DX12Device.h"

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

			utils::Device* device;

			IDXGISwapChain4* swapChain;

			int frameIndex;

			ID3D12CommandQueue* commandQueue;

			ID3D12DescriptorHeap* rtvDescriptorHeap;

			ID3D12Resource* renderTargets[FRAME_BUFFER_COUNT];

			
		};
	}
}