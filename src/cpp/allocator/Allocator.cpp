//
// Created by fathy on 23/06/2018.
//

#include <cassert>
#include "Allocator.h"

using namespace nsb;

Allocator::Allocator(void* space, size_t size):
	allocator(space, size)
{}

void* Allocator::Allocate(size_t size) {
	std::lock_guard<SpinLock> guard(lock);

	return allocator.Allocate(size);
}

void Allocator::Free(void *pointer) {
	std::lock_guard<SpinLock> guard(lock);

	return allocator.Free(pointer);
}
