//
// Created by fathy on 23/06/2018.
//

#ifndef NODE_SHARED_BUFFER_ALLOCATOR_TEST_H
#define NODE_SHARED_BUFFER_ALLOCATOR_TEST_H

#include "PoolAllocator.h"
#include "../utils/SpinLock.h"

namespace nsb {
	class Allocator {
	public:
		Allocator(void* space, size_t size);

		void* Allocate(size_t size);
		void Free(void* pointer);
	private:
		PoolAllocator allocator;
		SpinLock lock;
	};


}

#endif //NODE_SHARED_BUFFER_ALLOCATOR_TEST_H
