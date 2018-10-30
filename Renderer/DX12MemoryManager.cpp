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
				CloseHandle(fenceEvent);
				delete fence;
				delete commandList;
				delete commandAllocator;

				for (UploadHeap* uploadHeap : uploadHeaps) {
					uploadHeap->resource->Release();
					delete uploadHeap;
				}

				for (Heap* heap : heaps)
				{
					MemoryBlock* block = heap->firstBlock;
					while (block != nullptr) {
						if (block->allocated)
							FreeBlock(block);
						MemoryBlock* temp = block;
						block = block->nextBlock;
						delete temp;
					}

					FreeHeap(heap);
					delete heap;
				}
			}

			void DX12MemoryManager::AllocateHeap(Heap * heap)
			{
				if (heap->allocated)
					return;

				D3D12_HEAP_DESC desc = {};
				desc.SizeInBytes = heap->size;
				desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
				switch (heap->resourceType) {
				case ResourceType::BUFFER:
					desc.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
					break;
				case ResourceType::TEXTURE:
					desc.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
					break;
				case ResourceType::RT_DS_TEXTURE:
					desc.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
					break;
				}

				device->GetDevice()->CreateHeap(&desc, IID_PPV_ARGS(&(heap->heap)));

				heap->allocated = true;
			}

			void DX12MemoryManager::FreeHeap(Heap * heap)
			{
				if (!heap->allocated)
					return;

				heap->heap->Release();
				heap->allocated = false;
			}

			void DX12MemoryManager::AllocateBlock(MemoryBlock * block)
			{
				switch (block->heap->resourceType) {
				case ResourceType::BUFFER:
					D3D12_RESOURCE_DESC desc = {};
					desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
					desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
					desc.Width = block->size;
					desc.Height = 1;
					desc.DepthOrArraySize = 1;
					desc.MipLevels = 1;
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.SampleDesc.Count = 1;
					desc.SampleDesc.Quality = 0;
					desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

					device->GetDevice()->CreatePlacedResource(block->heap->heap, block->offset, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&block->resource));

					block->allocated = true;

					break;
				}
			}

			void DX12MemoryManager::FreeBlock(MemoryBlock * block)
			{
				if (!block->allocated)
					return;

				block->resource->Release();
				block->allocated = false;
			}

			size_t DX12MemoryManager::GetAlignment()
			{
				return D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;				
			}

			void DX12MemoryManager::Initialize(Device* device, ID3D12CommandQueue* commandQueue, size_t heapSize, size_t uploadHeapSize)
			{
				this->heapAlignment = GetAlignment();
				this->heapSize = (heapSize / heapAlignment + ((heapSize % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);
				this->uploadHeapSize = (uploadHeapSize / heapAlignment + ((uploadHeapSize % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);
				this->device = device;

				this->commandAllocator = new DX12CommandAllocator(device, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
				this->commandQueue = commandQueue;
				this->commandList = this->commandAllocator->CreateCommandList(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
				this->fence = new DX12Fence(device, 0, 1);

				fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

				/*Heap* bufferHeap = new Heap();
				bufferHeap->allocated = false;
				bufferHeap->firstBlock = nullptr;
				bufferHeap->freeBlocks = {};
				bufferHeap->largestFreeBlock = nullptr;
				bufferHeap->largestFreeBlockSize = 0;
				bufferHeap->size = this->heapSize;

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
				bufferHeap->largestFreeBlockSize = bufferHeap->firstBlock->size;*/


			}

			uint64_t DX12MemoryManager::MallocBuffer(size_t size, bool largeCommit)
			{
				//size = (size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);
				for (int i = 0, size = heaps.size(); i < size && !largeCommit; ++i) {
					if (heaps[i]->largestFreeBlockSize < size || heaps[i]->resourceType!=ResourceType::BUFFER || heaps[i]->largeBlock || !heaps[i]->allocated)
						continue;

					std::vector<MemoryBlock*>::iterator it = heaps[i]->freeBlocks.begin();

					for (; it != heaps[i]->freeBlocks.end(); ++it) {
						if ((*it)->size >= size) {
							MemoryBlock* block = (*it);
							if (size != block->size && (block->size - size) >= heapAlignment) {
								MemoryBlock* newBlock = new MemoryBlock();
								newBlock->allocated = false;
								newBlock->free = true;
								newBlock->heap = block->heap;
								newBlock->nextBlock = block->nextBlock;
								newBlock->prevBlock = block;
								newBlock->nextBlock->prevBlock = newBlock;
								block->nextBlock = newBlock;
								newBlock->offset = block->offset + (size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);
								newBlock->size = block->size - (size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);

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

				size_t heapSize = (size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment)
				> this->heapSize || largeCommit ? 
					(size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment) : this->heapSize;

				Heap* bufferHeap = new Heap();
				bufferHeap->allocated = false;
				bufferHeap->firstBlock = nullptr;
				bufferHeap->freeBlocks = {};
				bufferHeap->largestFreeBlock = nullptr;
				bufferHeap->largestFreeBlockSize = 0;
				bufferHeap->size = heapSize;
				bufferHeap->largeBlock = largeCommit;
				bufferHeap->resourceType = ResourceType::BUFFER;

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
				bufferHeap->firstBlock->largeCommit = largeCommit;

				if (bufferHeap->size > size) {
					MemoryBlock* freeBlock = new MemoryBlock();
					freeBlock->allocated = false;
					freeBlock->free = true;
					freeBlock->heap = bufferHeap;
					freeBlock->largeCommit = false;
					freeBlock->nextBlock = nullptr;
					freeBlock->offset = size;
					freeBlock->prevBlock = bufferHeap->firstBlock;
					freeBlock->resource = nullptr;
					freeBlock->size = bufferHeap->size - (size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);

					bufferHeap->firstBlock->size = size;
					bufferHeap->firstBlock->nextBlock = freeBlock;

					bufferHeap->freeBlocks.push_back(freeBlock);
					bufferHeap->largestFreeBlock = freeBlock;
					bufferHeap->largestFreeBlockSize = freeBlock->size;
				}

				MemoryBlock* block = bufferHeap->firstBlock;

				AllocateBlock(block);

				uint64_t blockId = (uint64_t)block;

				allocatedBlocks.push_back(blockId);

				return blockId;

			}

			void DX12MemoryManager::Free(MemoryAllocation memoryBlock)
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
				block->size = (block->size / heapAlignment + ((block->size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);
				block->heap->freeBlocks.push_back(block);
				if (block->heap->largestFreeBlockSize < block->size) {
					block->heap->largestFreeBlock = block;
					block->heap->largestFreeBlockSize = block->size;
				}
				if (block->largeCommit) {
					Heap* blockHeap = block->heap;
					delete block;
					FreeHeap(blockHeap);
					delete blockHeap;
				}

			}

			void DX12MemoryManager::SetBufferData(MemoryAllocation memoryBlock, size_t offset, size_t size, void * data,
				D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
			{

				//size_t memSize = (size / heapAlignment + ((size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);;

				std::vector<uint64_t>::iterator it = allocatedBlocks.begin();
				bool found = false;
				MemoryBlock* block = nullptr;
				for (; it != allocatedBlocks.end(); ++it) {
					if ((*it) == memoryBlock) {
						found = true;
						block = (MemoryBlock*)memoryBlock;
						break;
					}
				}

				if (block == nullptr)
					return;

				if (block->heap->resourceType != ResourceType::BUFFER)
					return;

				UploadHeap* uploadHeap = nullptr;

				for (int i = 0; i < uploadHeaps.size(); ++i) {
					if (uploadHeaps[i]->resourceType != ResourceType::BUFFER)
						continue;

					if (uploadHeaps[i]->size - uploadHeaps[i]->currentOffset >= size) {
						uploadHeap = uploadHeaps[i];
						break;
					}
				}

				if (uploadHeap == nullptr) {
					uploadHeap = new UploadHeap();
					uploadHeaps.push_back(uploadHeap);

					uploadHeap->currentOffset = 0;
					uploadHeap->resourceType = ResourceType::BUFFER;
					uploadHeap->size = size > uploadHeapSize ? size : uploadHeapSize;

					uploadHeap->size = (uploadHeap->size / heapAlignment + ((uploadHeap->size % heapAlignment) != 0 ? 1 : 0))*(heapAlignment);

					device->GetDevice()->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(uploadHeap->size),
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&uploadHeap->resource)
					);

					CD3DX12_RANGE readRange(0, 0);

					uploadHeap->resource->Map(0, &readRange, &(uploadHeap->CPUAddress));
				}

				memcpy((char*)uploadHeap->CPUAddress + uploadHeap->currentOffset, data, size);

				commandList->Reset(commandAllocator, NULL);

				D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(block->resource, currentState, D3D12_RESOURCE_STATE_COPY_DEST);

				commandList->ResourceBarrier({ barrier });

				//UpdateSubresources(commandList->GetCommandList(), block->resource, uploadHeap->resource, uploadHeap->currentOffset, 0, 1, &bufferData);

				commandList->CopyBufferRegion(block->resource, offset, uploadHeap->resource, uploadHeap->currentOffset, size);

				barrier = CD3DX12_RESOURCE_BARRIER::Transition(block->resource, D3D12_RESOURCE_STATE_COPY_DEST, finalState);

				commandList->ResourceBarrier({ barrier });

				commandList->Close();

				ID3D12CommandList* ppCommandLists[] = { commandList->GetCommandList() };
				commandQueue->ExecuteCommandLists(1, ppCommandLists);

				uploadHeap->currentOffset += size;

				fenceValue++;
				commandQueue->Signal(fence->GetFence(), fenceValue);
			}

			void DX12MemoryManager::WaitForMemoryOperations()
			{
				if (fence->GetCompletedValue() < fenceValue) {
					fence->SetFenceEvent(fenceValue, fenceEvent);

					WaitForSingleObject(fenceEvent, INFINITE);
				}

				for (int i = 0; i < uploadHeaps.size(); ++i) {
					uploadHeaps[i]->currentOffset = 0;
				}
			}

			ID3D12Resource * DX12MemoryManager::GetResource(MemoryAllocation memoryBlock)
			{
				std::vector<uint64_t>::iterator it = allocatedBlocks.begin();
				bool found = false;
				MemoryBlock* block = nullptr;
				for (; it != allocatedBlocks.end(); ++it) {
					if ((*it) == memoryBlock) {
						found = true;
						block = (MemoryBlock*)memoryBlock;
						break;
					}
				}
				if (block != nullptr)
					return block->resource;
				else
					return nullptr;
			}

		}
	}
}