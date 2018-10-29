#include "DX12Device.h"
#include "Utils.h"

#include <iostream>

namespace renderer {
	namespace dx12 {
		namespace utils {

			Device::Device(Window * window)
			{
				this->window = window;

#ifndef NDEBUG

				D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
				debugController->EnableDebugLayer();

#endif

				IDXGIFactory5* factory = Window::GetFactory();

				deviceCreated = true;

				HRESULT hardwareResult = D3D12CreateDevice(window->GetDisplayMode().adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));

				if (FAILED(hardwareResult)) {
					Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
					factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

					D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));

					deviceCreated = false;
				}

				rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
				cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

				D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
				msQualityLevels.Format = Window::GetBackBufferFormat();
				msQualityLevels.SampleCount = 4;
				msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
				msQualityLevels.NumQualityLevels = 0;
				ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
					&msQualityLevels, sizeof(msQualityLevels)));

				int m4xMsaaQuality = msQualityLevels.NumQualityLevels;

			}

			Device::~Device()
			{
				for (int i = 0; i < commandQueues.size(); ++i) {
					commandQueues[i]->Release();
				}

				if (debugController != nullptr)
					debugController->Release();

				device->Release();
			}

			ID3D12Device * Device::GetDevice()
			{
				return device;
			}

			ID3D12CommandQueue * Device::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority, D3D12_COMMAND_QUEUE_FLAGS flags)
			{
				D3D12_COMMAND_QUEUE_DESC desc = {};

				desc.Type = type;
				desc.Priority = priority;
				desc.Flags = flags;
				desc.NodeMask = 0;

				ID3D12CommandQueue* queue;

				HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue));

				if (FAILED(hr)) {
					std::cout << "[ERROR] Failed to create command queue with error " << hr << std::endl;

					return nullptr;
				}

				commandQueues.push_back(queue);

				return queue;
			}

			bool Device::IsDeviceCreationsuccessful()
			{
				return deviceCreated;
			}

			uint32_t Device::GetRtvDescriptorSize()
			{
				return rtvDescriptorSize;
			}

			uint32_t Device::GetDsvDescriptorSize()
			{
				return dsvDescriptorSize;
			}

			uint32_t Device::GetCbvSrvDescriptorSize()
			{
				return cbvSrvDescriptorSize;
			}

			uint32_t Device::GetSamplerDescriptorSize()
			{
				return samplerDescriptorSize;
			}

		}
	}
}