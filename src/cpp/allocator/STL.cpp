//
// Created by fathy on 06/06/2018.
//

#include "STL.h"

#include "../SharedMemory.h"

void *nsb::memory::AllocateShared(size_t size) {
	return SharedMemory::Shared()->Allocate(size);
}

void nsb::memory::FreeShared(void *ptr) {
	return SharedMemory::Shared()->Free(ptr);
}
