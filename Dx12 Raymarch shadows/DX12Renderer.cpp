#include "DX12Renderer.h"

#include <iostream>
#include "D3dx12.h"

namespace renderer {
	namespace dx12 {

		DX12Renderer::DX12Renderer(Window* window) : Renderer(window)
		{
		}


		DX12Renderer::~DX12Renderer()
		{
			for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
				renderTargets[i]->Release();
			}
			rtvDescriptorHeap->Release();
			swapChain->Release();

			delete device;
		}

		int DX12Renderer::Initialize()
		{
			this->device = new utils::Device(window);

			if (!device->IsDeviceCreationsuccessful()) {
				std::cout << "[FATAL] Device creation failed" << std::endl;
				return -1;
			}

			this->commandQueue = device->CreateCommandQueue();

			if (this->commandQueue == nullptr) {
				std::cout << "[FATAL] Failed to create main command queue" << std::endl;
			}

			if (CreateSwapChain() != 0) {
				std::cout << "[FATAL] Failed to create swap chain" << std::endl;
			}

			if (CreateRtvDescriptorHeap() != 0) {
				std::cout << "[FATAL] Failed to create rtv descriptors" << std::endl;
			}

			return 0;
		}

		void DX12Renderer::RenderStart()
		{
		}

		void DX12Renderer::RenderEnd()
		{
		}

		int DX12Renderer::CreateSwapChain()
		{
			DXGI_SWAP_CHAIN_DESC desc = {};
			desc.BufferDesc = window->GetDisplayMode().description;
			desc.SampleDesc = { 1,0 };
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.BufferCount = FRAME_BUFFER_COUNT;
			desc.OutputWindow = window->GetWindowhandle();
			desc.Windowed = true;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.Flags = 0;

			IDXGISwapChain* tempSwapChain;

			HRESULT hr = Window::GetFactory()->CreateSwapChain(commandQueue, &desc, &tempSwapChain);

			if (FAILED(hr)) {
				std::cout << "[ERROR] Swap chain creation failed with error " << hr << std::endl;
				return hr;
			}

			swapChain = static_cast<IDXGISwapChain4*>(tempSwapChain);

			frameIndex = swapChain->GetCurrentBackBufferIndex();

			return 0;
		}

		int DX12Renderer::CreateRtvDescriptorHeap()
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			HRESULT hr = device->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
			if (FAILED(hr)) {
				std::cout << "[ERROR] Creating rtv descriptor heap failed with error code " << hr << std::endl;
				return hr;
			}

			uint32_t rtvDescriptorSize = device->GetRtvDescriptorSize();

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
				hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));

				if (FAILED(hr)) {
					std::cout << "[ERROR] Failed to aquire render target buffer " << i << " with error code " << hr << std::endl;
					return hr;
				}

				device->GetDevice()->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

				rtvHandle.Offset(1, device->GetRtvDescriptorSize());
			}

			return 0;
		}

	}
}