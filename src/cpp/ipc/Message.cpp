//
// Created by fathy on 11/06/2018.
//

#include "Message.h"
#include "../SharedMemory.h"
#include "../utils/PointerUtils.h"

using namespace nsb;

Message::Message(
	const std::string& name,
	char* ptr,
	size_t size,
	Worker* worker,
	MessageContainer* container,
	int groupId
):
	groupId(groupId),
	callingWorker(worker),
	container(container)
{
	input.pointer = ptr;
	input.size = size;

	assert(name.size() < 32);
	std::copy(name.begin(), name.end(), this->name);
}

Message* Message::Slice(size_t length) {
	auto end = pointer::Offset(this, (length - 1) * sizeof(Message));

	std::lock_guard<SpinLock> thisLock(end->lock);

	auto next = end->next;

	if(next != nullptr) {
		std::lock_guard<SpinLock> nextLock(next->lock);

		next->prev = nullptr;
		end->next = nullptr;
	}

	return next;
}

MessageContainer* Message::Make(size_t size) {
	auto memory = SharedMemory::Shared();
	auto address = memory->Allocate(sizeof(MessageContainer) + sizeof(Message) * size);
	auto container = new (address) MessageContainer();

	container->references = size;

	return container;
}

char* MessageContainer::Start() {
	return pointer::After(this);
}

void MessageContainer::Release() {
	if(--references == 0) {
		SharedMemory::Shared()->Free(this);
	}
}
