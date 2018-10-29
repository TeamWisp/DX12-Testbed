#pragma once

#include "Window.h"

#include <d3d12.h>

#include <wrl\client.h>

namespace renderer {
	namespace dx12 {
		namespace utils {

			class Device
			{
			public:
				Device(Window* window);
				~Device();

				ID3D12Device* GetDevice();

				ID3D12CommandQueue* CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT, 
					D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, 
					D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE);

				bool IsDeviceCreationsuccessful();

				uint32_t GetRtvDescriptorSize();
				uint32_t GetDsvDescriptorSize();
				uint32_t GetCbvSrvDescriptorSize();
				uint32_t GetSamplerDescriptorSize();

			protected:
				Window * window;

				ID3D12Debug* debugController;

				ID3D12Device* device;

				std::vector<ID3D12CommandQueue*> commandQueues;

				bool deviceCreated;

				uint32_t rtvDescriptorSize;
				uint32_t dsvDescriptorSize;
				uint32_t cbvSrvDescriptorSize;
				uint32_t samplerDescriptorSize;
			};

		}
	}
}