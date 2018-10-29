#include "DX12MemoryManager.h"
#include <d3d12.h>

namespace renderer {
	namespace dx12 {
		namespace utils {

			DX12MemoryManager::DX12MemoryManager()
			{
			}


			DX12MemoryManager::~DX12MemoryManager()
			{
			}

			void DX12MemoryManager::AllocateHeap(Heap * heap)
			{
				D3D12_HEAP_DESC desc = {};
				desc.SizeInBytes = heap->size;
				desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

			}

			size_t DX12MemoryManager::GetAlignment()
			{
				return D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;				
			}

			void DX12MemoryManager::Initialize(size_t bufferHeapSize)
			{
				this->bufferAlignment = GetAlignment();
				this->bufferHeapSize = (bufferHeapSize / bufferAlignment)*(bufferAlignment + ((bufferHeapSize % bufferAlignment) != 0 ? 1 : 0));

				Heap* bufferHeap = new Heap();
				bufferHeap->allocated = false;
				bufferHeap->firstBlock = nullptr;
				bufferHeap->freeBlocks = {};
				bufferHeap->largestFreeBlock = nullptr;
				bufferHeap->largestFreeBlockSize = 0;
				bufferHeap->size = this->bufferHeapSize;

				AllocateHeap(bufferHeap);

				heaps.push_back(bufferHeap);

				bufferHeap->firstBlock = new MemoryBlock();
				bufferHeap->firstBlock->allocated = false;
				bufferHeap->firstBlock->free = true;
				bufferHeap->firstBlock->heap = bufferHeap;
				bufferHeap->firstBlock->prevBlock = nullptr;
				bufferHeap->firstBlock->nextBlock = nullptr;
				bufferHeap->firstBlock->offset = 0;
				bufferHeap->firstBlock->size = bufferHeap->size;

				bufferHeap->freeBlocks.push_back(bufferHeap->firstBlock);
				bufferHeap->largestFreeBlock = bufferHeap->firstBlock;
				bufferHeap->largestFreeBlockSize = bufferHeap->firstBlock->size;


			}

			uint64_t DX12MemoryManager::MallocBuffer(size_t size, MemoryUsage bufferType)
			{
				if (bufferType == MemoryUsage::IMAGE) {
					return 0;
				}
				size = bufferType == MemoryUsage::CONSTANT_BUFFER ?
					(size / constantBufferAlignment)*(constantBufferAlignment + ((size % constantBufferAlignment) != 0 ? 1 : 0)) :
					(size / bufferAlignment)*(bufferAlignment + ((size % bufferAlignment) != 0 ? 1 : 0));
				for (int i = 0, size = heaps.size(); i < size; ++i) {
					if (heaps[i]->largestFreeBlockSize < size || heaps[i]->usage != bufferType)
						continue;

					std::vector<MemoryBlock*>::iterator it = heaps[i]->freeBlocks.begin();

					for (; it != heaps[i]->freeBlocks.end(); ++it) {
						if ((*it)->size > size) {
							MemoryBlock* block = (*it);
							if (size != block->size && (block->size - size) >= bufferAlignment) {
								MemoryBlock* newBlock = new MemoryBlock();
								newBlock->allocated = false;
								newBlock->free = true;
								newBlock->heap = block->heap;
								newBlock->nextBlock = block->nextBlock;
								newBlock->prevBlock = block;
								newBlock->nextBlock->prevBlock = newBlock;
								block->nextBlock = newBlock;
								newBlock->offset = block->offset + size;
								newBlock->size = block->size - size;

								block->size = size;

								(*it) = newBlock;
							}
							else {
								heaps[i]->freeBlocks.erase(it);
							}
							if (heaps[i]->largestFreeBlock == block) {
								std::vector<MemoryBlock*>::iterator it2;
								heaps[i]->largestFreeBlock = nullptr;
								heaps[i]->largestFreeBlockSize = 0;
								for (it2 = heaps[i]->freeBlocks.begin(); it2 != heaps[i]->freeBlocks.end(); ++it2) {
									if ((*it2)->size > heaps[i]->largestFreeBlockSize) {
										heaps[i]->largestFreeBlockSize = (*it2)->size;
										heaps[i]->largestFreeBlock = (*it2);
									}
								}
							}

							block->free = false;

							AllocateBlock(block);

							uint64_t blockId = (uint64_t)block;

							allocatedBlocks.push_back(blockId);

							return blockId;
						}
					}
				}

				size_t heapSize = size > bufferHeapSize ? size : bufferHeapSize;

				Heap* bufferHeap = new Heap();
				bufferHeap->allocated = false;
				bufferHeap->firstBlock = nullptr;
				bufferHeap->freeBlocks = {};
				bufferHeap->largestFreeBlock = nullptr;
				bufferHeap->largestFreeBlockSize = 0;
				bufferHeap->size = heapSize;
				bufferHeap->usage = bufferType;

				AllocateHeap(bufferHeap);

				heaps.push_back(bufferHeap);

				bufferHeap->firstBlock = new MemoryBlock();
				bufferHeap->firstBlock->allocated = false;
				bufferHeap->firstBlock->free = false;
				bufferHeap->firstBlock->heap = bufferHeap;
				bufferHeap->firstBlock->prevBlock = nullptr;
				bufferHeap->firstBlock->nextBlock = nullptr;
				bufferHeap->firstBlock->offset = 0;
				bufferHeap->firstBlock->size = bufferHeap->size;

				MemoryBlock* block = bufferHeap->firstBlock;

				AllocateBlock(block);

				uint64_t blockId = (uint64_t)block;

				allocatedBlocks.push_back(blockId);

				return blockId;

			}

			void DX12MemoryManager::Free(uint64_t memoryBlock)
			{
				std::vector<uint64_t>::iterator it = allocatedBlocks.begin();
				bool found = false;
				MemoryBlock* block = nullptr;
				for (; it != allocatedBlocks.end(); ++it) {
					if ((*it) == memoryBlock) {
						found = true;
						block = (MemoryBlock*)memoryBlock;
						allocatedBlocks.erase(it);
						break;
					}
				}
				if (!found) return;
				FreeBlock(block);

				block->free = true;
				block->heap->freeBlocks.push_back(block);
				if (block->heap->largestFreeBlockSize < block->size) {
					block->heap->largestFreeBlock = block;
					block->heap->largestFreeBlockSize = block->size;
				}

			}

		}
	}
}