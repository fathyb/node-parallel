//
// Created by fathy on 01/06/2018.
//

#include <string>
#include <cerrno>
#include <random>
#include <iostream>

#include "SharedMemory.h"
#include "utils/Configuration.h"
#include "utils/PointerUtils.h"

#include <jemalloc/jemalloc.h>

using namespace std;
using namespace nsb;


SharedMemory::SharedMemory(size_t memory, size_t workers) {
	auto address = platform::CreateMemoryRegion(sizeof(MemoryHeader));
	header = new(address) MemoryHeader(this, workers);

	LinkedList<MemoryHeader::Region>* prevRegion = nullptr;
	size_t regions = workers + 2;

	for(Worker::Id i = 0; i < regions; i++) {
		auto sizeofList = sizeof(MemoryHeader::Region::List);
		auto mapped = reinterpret_cast<char*>(platform::CreateMemoryRegion(memory + sizeofList));
		auto region = new (mapped) MemoryHeader::Region::List(
			sizeofList + mapped,
			sizeofList - memory,
			i
		);

		prevRegion = prevRegion
			? prevRegion->next = region
			: header->regions = region;

		log(Debug) << "New region of " << memory << " bytes created";
	}

	GlobalConfiguration.heap = header;
}

SharedMemory::SharedMemory() {
	header = static_cast<MemoryHeader*>(GlobalConfiguration.heap);
}

namespace {
	struct MemoryBlock {
		inline explicit MemoryBlock(Worker* owner):
			owner(owner)
		{}

		Worker* owner;
	};
}

void* SharedMemory::Allocate(size_t size) {
	auto worker = GlobalConfiguration.worker;
	assert(worker != nullptr);
	log(Debug) << "Allocating " << size << " bytes";
	auto pointer = worker->allocator->Allocate(size + sizeof(MemoryBlock));
	auto block = new (pointer) MemoryBlock(worker);

	return pointer::After(block);
}

void SharedMemory::Free(void* pointer) {return;
	auto block = pointer::Before<MemoryBlock>(pointer);

	return block->owner->allocator->Free(block);
}

IPC* SharedMemory::GetIPC() {
	return &header->ipc;
}

thread_local SharedMemory* SharedMemory::sharedInstance;

SharedMemory* SharedMemory::Shared() {
	if(sharedInstance == nullptr) {
		sharedInstance = new SharedMemory();
	}

	return sharedInstance;
}

void SharedMemory::Stop() {
	header->stop = true;
}

bool SharedMemory::IsStopped() {
	return header->stop;
}

int SharedMemory::GetId() {
	return header->counter++;
}

Allocator* SharedMemory::GetAllocator(Worker::Id id) {
	for(auto region = header->regions; region != nullptr; region = region->next) {
		if(region->id == id) {
			return &region->allocator;
		}
	}

	return nullptr;
}

