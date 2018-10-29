#pragma once
#include "MemoryManager.h"

#include <d3d12.h>
#include "D3dx12.h"

namespace renderer {
	namespace dx12 {
		namespace utils {

			class DX12MemoryManager
			{
			public:
				DX12MemoryManager();
				virtual ~DX12MemoryManager();

				virtual void Initialize(size_t bufferHeapSize);

				uint64_t MallocBuffer(size_t size);

				uint64_t MallocImage(size_t x, size_t y, size_t z, size_t pixelSize);

				void Free(uint64_t memoryBlock);
				
				void SetBufferData(uint64_t memoryBlock, size_t offset, size_t size, char* data);

				std::vector<char> ReadData(uint64_t memoryBlock);

				void* GetImageMemoryData(uint64_t memoryBlock);

				void* GetBufferImplementationData(uint64_t memoryBlock);

			protected:
				struct MemoryBlock;

				struct Heap {
					size_t size;
					std::vector<MemoryBlock*> freeBlocks;
					size_t largestFreeBlockSize;
					MemoryBlock* largestFreeBlock;
					MemoryBlock* firstBlock;
					bool allocated;
					ID3D12Heap* heap;
				};

				struct MemoryBlock {
					size_t size;
					size_t offset;
					bool free;
					Heap* heap;
					MemoryBlock* prevBlock;
					MemoryBlock* nextBlock;
					bool allocated;
					ID3D12Resource* resource;
				};

				std::vector<Heap*> heaps;

				std::vector<uint64_t> allocatedBlocks;

				size_t bufferHeapSize;
				size_t bufferAlignment;

				void AllocateHeap(Heap* heap);

				void FreeHeap(Heap* heap);

				void AllocateBlock(MemoryBlock* block);

				void FreeBlock(MemoryBlock* block);

				size_t GetAlignment();
			};

		}
	}
}