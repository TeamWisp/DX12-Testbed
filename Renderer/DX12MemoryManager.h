#pragma once

#include <vector>

#include <d3d12.h>
#include "D3dx12.h"

#include "DX12Device.h"
#include "DX12CommandAllocator.h"
#include "DX12CommandList.h"
#include "DX12Fence.h"

namespace renderer {
	namespace dx12 {
		namespace utils {

			enum ResourceType {
				BUFFER = 0,
				TEXTURE,
				RT_DS_TEXTURE
			};

			class DX12MemoryManager
			{
			public:
				DX12MemoryManager();
				virtual ~DX12MemoryManager();

				using MemoryAllocation = uint64_t;

				virtual void Initialize(Device* device, ID3D12CommandQueue* commandQueue, size_t heapSize, size_t uploadHeapSize);

				//Memory blocks are initialized into D3D12_RESOURCE_STATE_COMMON
				MemoryAllocation MallocBuffer(size_t size, bool largeCommit = false);

				MemoryAllocation MallocImage(size_t x, size_t y, size_t z, size_t pixelSize);

				void Free(MemoryAllocation memoryBlock);
				
				void SetBufferData(MemoryAllocation memoryBlock, size_t offset, size_t size, void* data,
					D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_COMMON, 
					D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

				void WaitForMemoryOperations();

				std::vector<char> ReadData(MemoryAllocation memoryBlock);

				void* GetImageMemoryData(MemoryAllocation memoryBlock);

				ID3D12Resource* GetResource(MemoryAllocation memoryBlock);

			protected:
				struct MemoryBlock;

				struct Heap {
					size_t size;
					std::vector<MemoryBlock*> freeBlocks;
					size_t largestFreeBlockSize;
					MemoryBlock* largestFreeBlock;
					MemoryBlock* firstBlock;
					bool allocated;
					bool largeBlock;
					ResourceType resourceType;
					ID3D12Heap* heap;
				};

				struct MemoryBlock {
					size_t size;
					size_t offset;
					bool free;
					bool largeCommit;
					Heap* heap;
					MemoryBlock* prevBlock;
					MemoryBlock* nextBlock;
					bool allocated;
					ID3D12Resource* resource;
				};

				struct UploadHeap {
					ID3D12Resource* resource;
					size_t size;
					size_t currentOffset;
					ResourceType resourceType;
					void* CPUAddress;
				};

				std::vector<Heap*> heaps;

				std::vector<MemoryAllocation> allocatedBlocks;

				std::vector<UploadHeap*> uploadHeaps;

				Device* device;
				DX12CommandAllocator* commandAllocator;
				DX12CommandList* commandList;
				DX12Fence* fence;
				HANDLE fenceEvent;
				uint64_t fenceValue;

				ID3D12CommandQueue* commandQueue;

				size_t heapSize;
				size_t heapAlignment;

				size_t uploadHeapSize;

				void AllocateHeap(Heap* heap);

				void FreeHeap(Heap* heap);

				void AllocateBlock(MemoryBlock* block);

				void FreeBlock(MemoryBlock* block);

				size_t GetAlignment();
			};

		}
	}
}