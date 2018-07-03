//
// Created by fathy on 24/06/2018.
//

#ifndef NODE_SHARED_BUFFER_LIFEISPAIN_H
#define NODE_SHARED_BUFFER_LIFEISPAIN_H

#include <cstdlib>

namespace nsb {
	class PoolAllocator {
	public:
		PoolAllocator(void* space, size_t size);

		void* Allocate(size_t bytes);
		void Free(void* pointer);
	private:
		union MemoryHeaderUnion {
			struct {
				// Pointer to the next block in the free list
				union MemoryHeaderUnion* next;

				// Size of the block (in quantas of sizeof(MemoryHeader))
				size_t size;
			} s;

			// Used to align headers in memory to a boundary
			size_t alignDummy;
		};

		typedef union MemoryHeaderUnion MemoryHeader;

		MemoryHeader base;
		MemoryHeader* freep = nullptr;

		char* space;
		size_t size;
		size_t position = 0;

		MemoryHeader* GetMemoryFromPoll(size_t quantas);
	};
}



#endif //NODE_SHARED_BUFFER_LIFEISPAIN_H
