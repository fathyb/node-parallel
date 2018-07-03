//
// Created by fathy on 24/06/2018.
//

#include "PoolAllocator.h"
#include "../utils/Logger.h"

// Internally, the memory manager allocates memory in
// quantas roughly the size of two ulong objects. To
// minimize pool fragmentation in case of multiple allocations
// and deallocations, it is advisable to not allocate
// blocks that are too small.
// This flag sets the minimal ammount of quantas for
// an allocation. If the size of a ulong is 4 and you
// set this flag to 16, the minimal size of an allocation
// will be 4 * 2 * 16 = 128 bytes
// If you have a lot of small allocations, keep this value
// low to conserve memory. If you have mostly large
// allocations, it is best to make it higher, to avoid
// fragmentation.
//
#define MIN_POOL_ALLOC_QUANTAS 16

using namespace nsb;

PoolAllocator::MemoryHeader* PoolAllocator::GetMemoryFromPoll(size_t quantas) {
	size_t requiredSize;
	MemoryHeader* h;

	if(quantas < MIN_POOL_ALLOC_QUANTAS) {
		quantas = MIN_POOL_ALLOC_QUANTAS;
	}

	requiredSize = quantas * sizeof(MemoryHeader);

	if(position + requiredSize <= size) {
		h = (MemoryHeader*)(space + position);
		h->s.size = quantas;

		Free((void*)(h + 1));
		position += requiredSize;
	}
	else {
		return nullptr;
	}

	return freep;
}

nsb::PoolAllocator::PoolAllocator(void *space, size_t size):
	space(static_cast<char*>(space)),
	size(size)
{
	base.s.next = nullptr;
	base.s.size = 0;
}

// Allocations are done in 'quantas' of header size.
// The search for a free block of adequate size begins at the point 'freep'
// where the last block was found.
// If a too-big block is found, it is split and the tail is returned (this
// way the header of the original needs only to have its size adjusted).
// The pointer returned to the user points to the free space within the block,
// which begins one quanta after the header.
void* PoolAllocator::Allocate(size_t bytes) {
	MemoryHeader* p;
	MemoryHeader* prevp;

	// Calculate how many quantas are required: we need enough to house all
	// the requested bytes, plus the header. The -1 and +1 are there to make sure
	// that if nbytes is a multiple of nquantas, we don't allocate too much
	//
	size_t quantas = (bytes + sizeof(MemoryHeader) - 1) / sizeof(MemoryHeader) + 1;

	// First alloc call, and no free list yet ? Use 'base' for an initial
	// denegerate block of size 0, which points to itself
	//
	if ((prevp = freep) == nullptr) {
		base.s.next = freep = prevp = &base;
		base.s.size = 0;
	}

	for (p = prevp->s.next; ; prevp = p, p = p->s.next) {
		// big enough ?
		if (p->s.size >= quantas) {
			// exactly ?
			if (p->s.size == quantas) {
				// just eliminate this block from the free list by pointing
				// its prev's next to its next
				//
				prevp->s.next = p->s.next;
			}
			else {
				p->s.size -= quantas;
				p += p->s.size;
				p->s.size = quantas;
			}

			freep = prevp;
			return (void*) (p + 1);
		}
		// Reached end of free list ?
		// Try to allocate the block from the pool. If that succeeds,
		// get_mem_from_pool adds the new block to the free list and
		// it will be found in the following iterations. If the call
		// to get_mem_from_pool doesn't succeed, we've run out of
		// memory
		else if (p == freep) {
			if ((p = GetMemoryFromPoll(quantas)) == nullptr) {
				log(Error) << "Memory allocation failed";

				return nullptr;
			}
		}
	}
}

// Scans the free list, starting at freep, looking the the place to insert the
// free block. This is either between two existing blocks or at the end of the
// list. In any case, if the block being freed is adjacent to either neighbor,
// the adjacent blocks are combined.
void nsb::PoolAllocator::Free(void *pointer) {
	MemoryHeader* block = ((MemoryHeader*)pointer) - 1;
	MemoryHeader* p;

	// Find the correct place to place the block in (the free list is sorted by
	// address, increasing order)
	for(p = freep; !(block > p && block < p->s.next); p = p->s.next) {
		// Since the free list is circular, there is one link where a
		// higher-addressed block points to a lower-addressed block.
		// This condition checks if the block should be actually
		// inserted between them
		if(p >= p->s.next && (block > p || block < p->s.next)) {
			break;
		}
	}

	// Try to combine with the higher neighbor
	if(block + block->s.size == p->s.next) {
		block->s.size += p->s.next->s.size;
		block->s.next = p->s.next->s.next;
	}
	else {
		block->s.next = p->s.next;
	}

	// Try to combine with the lower neighbor
	if (p + p->s.size == block) {
		p->s.size += block->s.size;
		p->s.next = block->s.next;
	}
	else {
		p->s.next = block;
	}

	freep = p;
}
