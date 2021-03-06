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
				if (fences[i]->GetCompletedValue() < fences[i]->GetValue()) {
					fences[i]->SetFenceEvent(fences[i]->GetValue(), fenceEvent);

					WaitForSingleObject(fenceEvent, INFINITE);
				}
			}
			delete memoryManager;
			delete defaultPSO;
			delete defaultRootSignature;

			CloseHandle(fenceEvent);

			for (int i = 0; i < fences.size(); ++i) {
				delete fences[i];
			}


			for (int i = 0; i < commandAllocators.size(); ++i) {
				delete commandAllocators[i];
				delete primaryCommandLists[i];
			}

			delete bundleAllocator;

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

			CreateCommandAllocators();

			if (CreateFences() != 0) {
				std::cout << "[FATAL] Failed to create fences" << std::endl;
			}

			frameIndex = 0;

			defaultRootSignature = new utils::DX12RootSignature(device);

			defaultRootSignature->SetFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			CD3DX12_ROOT_PARAMETER projection_constant;

			projection_constant.InitAsConstants(16, 0, 0);

			defaultRootSignature->AddRootParameter(projection_constant);

			defaultRootSignature->Compile(0);

			defaultPSO = new utils::DX12PipelineStateObject(device);

			defaultPSO->LoadShader(L"VertexShader.hlsl", utils::ShaderType::VERTEX_SHADER, {}, nullptr, NULL);
			defaultPSO->LoadShader(L"PixelShader.hlsl", utils::ShaderType::PIXEL_SHADER, {}, nullptr, NULL);

			defaultPSO->AddInputElementDescription("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
			defaultPSO->AddInputElementDescription("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);

			defaultPSO->SetNumberOfRenderTargets(1);
			defaultPSO->SetRenderTargetFormat(0, window->GetDisplayMode().description.Format);

			defaultPSO->SetRootSignature(defaultRootSignature);

			defaultPSO->Compile();

			memoryManager = new utils::DX12MemoryManager();

			memoryManager->Initialize(device, commandQueue, 32 * 1024 * 1024, 4 * 1024 * 1024);

			std::vector<Vertex> vertices = {
				{{-0.5f,-0.5f,0.5f}, {1.f,0.f,0.f,1.f}},
				{{0.5f,-0.5f,0.5f},{0.f,1.f,0.f,1.f}},
				{{0.0f,0.5f,0.5f},{0.f,0.f,1.f,1.f}}
			};

			vertexBuffer = memoryManager->MallocBuffer(sizeof(Vertex)*vertices.size());

			memoryManager->SetBufferData(vertexBuffer, 0,
				sizeof(Vertex)*vertices.size(), (vertices.data()));

			vertexBufferView.BufferLocation = memoryManager->GetResource(vertexBuffer)->GetGPUVirtualAddress();
			vertexBufferView.StrideInBytes = sizeof(Vertex);
			vertexBufferView.SizeInBytes = sizeof(Vertex)*vertices.size();

			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = window->GetDisplayMode().width;
			viewport.Height = window->GetDisplayMode().height;
			viewport.MinDepth = 0.f;
			viewport.MaxDepth = 1.f;

			scissorRect.left = 0;
			scissorRect.top = 0;
			scissorRect.right = window->GetDisplayMode().width;
			scissorRect.bottom = window->GetDisplayMode().height;

			return 0;
		}

		void DX12Renderer::RenderStart()
		{
			frameIndex = swapChain->GetCurrentBackBufferIndex();

			if (fences[frameIndex]->GetCompletedValue() < fences[frameIndex]->GetValue()) {
				fences[frameIndex]->SetFenceEvent(fences[frameIndex]->GetValue(), fenceEvent);

				WaitForSingleObject(fenceEvent, INFINITE);
			}

			fences[frameIndex]->Signal(0);

			fences[frameIndex]->SetValue(1);

			commandAllocators[frameIndex]->Reset();

			primaryCommandLists[frameIndex]->Reset(commandAllocators[frameIndex], defaultPSO->GetPSO());

			primaryCommandLists[frameIndex]->ResourceBarrier({ CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex],D3D12_RESOURCE_STATE_PRESENT,D3D12_RESOURCE_STATE_RENDER_TARGET) });

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, device->GetRtvDescriptorSize());

			primaryCommandLists[frameIndex]->OMSetRenderTargets({ rtvHandle }, nullptr);

			float clearColor[] = { 0.0f,0.2f,0.4f,1.f };

			primaryCommandLists[frameIndex]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			primaryCommandLists[frameIndex]->SetGraphicsRootSignature(defaultRootSignature);
			primaryCommandLists[frameIndex]->RSSetViewports({ viewport });
			primaryCommandLists[frameIndex]->RSSetScissorRects({ scissorRect });
			primaryCommandLists[frameIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			primaryCommandLists[frameIndex]->IASetVertexBuffers(0, { vertexBufferView });
			primaryCommandLists[frameIndex]->DrawInstanced(3, 1, 0, 0);

		}

		void DX12Renderer::RenderEnd()
		{

			primaryCommandLists[frameIndex]->ResourceBarrier({ CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex],D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT) });

			primaryCommandLists[frameIndex]->Close();

			ID3D12CommandList* ppCommandLists[] = { primaryCommandLists[frameIndex]->GetCommandList() };

			memoryManager->WaitForMemoryOperations();

			commandQueue->ExecuteCommandLists(1, ppCommandLists);

			commandQueue->Signal(fences[frameIndex]->GetFence(), 1);

			swapChain->Present(0, 0);
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

		int DX12Renderer::CreateCommandAllocators()
		{
			commandAllocators.resize(FRAME_BUFFER_COUNT);
			primaryCommandLists.resize(FRAME_BUFFER_COUNT);
			for (int i = 0; i < commandAllocators.size(); ++i) {
				commandAllocators[i] = new utils::DX12CommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
				primaryCommandLists[i] = commandAllocators[i]->CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
			}
			bundleAllocator = new utils::DX12CommandAllocator(device, D3D12_COMMAND_LIST_TYPE_BUNDLE);

			return 0;
		}

		int DX12Renderer::CreateFences()
		{
			fences.resize(FRAME_BUFFER_COUNT);
			for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
				fences[i] = new utils::DX12Fence(device, 0, 1);
			}

			fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (fenceEvent == nullptr) {
				std::cout << "[ERROR] Creating fence event failed" << std::endl;
				return 1;
			}

			return 0;
		}

	}
}
