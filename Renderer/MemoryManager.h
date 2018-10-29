#pragma once
#include <vector>

namespace renderer {

	class MemoryManager
	{
	public:
		MemoryManager();
		virtual ~MemoryManager();

		virtual void Initialize(size_t bufferHeapSize, size_t constantBufferHeapSize, size_t imageHeapSize, MemoryType type);
		
		uint64_t MallocBuffer(size_t size, MemoryUsage bufferType);

		uint64_t MallocImage(size_t x, size_t y, size_t z, size_t pixelSize);

		void Free(uint64_t memoryBlock);

		virtual void SetBufferData(uint64_t memoryBlock, size_t offset, size_t size, char* data) = 0;

		virtual std::vector<char> ReadData(uint64_t memoryBlock) = 0;

		virtual void* GetImageMemoryData(uint64_t memoryBlock) = 0;

		virtual void* GetBufferImplementationData(uint64_t memoryBlock) = 0;

	protected:
		struct MemoryBlock;

		struct Heap {
			size_t size;
			MemoryUsage usage;
			std::vector<MemoryBlock*> freeBlocks;
			size_t largestFreeBlockSize;
			MemoryBlock* largestFreeBlock;
			MemoryBlock* firstBlock;
			bool allocated;
			void* implementationData;
		};

		struct MemoryBlock {
			size_t size;
			size_t offset;
			bool free;
			Heap* heap;
			MemoryBlock* prevBlock;
			MemoryBlock* nextBlock;
			bool allocated;
			void* implementationData;
		};

		virtual void AllocateHeap(Heap* heap) = 0;

		virtual void FreeHeap(Heap* heap) = 0;

		virtual void AllocateBlock(MemoryBlock* block) = 0;

		virtual void FreeBlock(MemoryBlock* block) = 0;

		virtual size_t GetAlignment(MemoryType type, MemoryUsage usage) = 0;

		std::vector<Heap*> heaps;
		std::vector<Heap*> imageHeaps;

		std::vector<uint64_t> allocatedBlocks;

		size_t bufferHeapSize, constantBufferHeapSize, imageHeapSize;
		size_t bufferAlignment, constantBufferAlignment, imageAlignment;

		MemoryType type;

	};

}