//
// Created by fathy on 05/06/2018.
//

#include "./Buffer.h"

#include <nan.h>
#include "../SharedMemory.h"

using namespace nsb;
using namespace std;
using namespace v8;

namespace {
	void FreeBuffer(char* data, void* hint) {
		SharedMemory::Shared()->Free(data);
	}
}

JSValue addon::ImportBuffer(JSArg name) {
	/*
	string identifier(*Nan::Utf8String(name));
	string prefix("memory://");
	char* pointer = nullptr;
	auto memory = SharedMemory::Shared();

	if(!identifier.compare(0, prefix.size(), prefix)) {
		pointer = (char*)(stol(identifier.substr(prefix.size())) + (uintptr_t)memory->Pointer());
	}

	// Increment the reference counter
	auto size = 1024; //memory->Acquire(pointer);

	return Nan::NewBuffer(pointer, size, &FreeBuffer, nullptr).ToLocalChecked();*/
}

JSValue addon::AllocBuffer(JSArg jsSize) {
	// Cast the size JS argument
	auto size = jsSize->Uint32Value();
	// Allocate the size in the virtual heap
	auto pointer = SharedMemory::Shared()->Allocate(size);

	// Create a Node.js buffer
	return Nan::NewBuffer(static_cast<char*>(pointer), size, &FreeBuffer, nullptr).ToLocalChecked();
}

JSValue addon::ExportBuffer(Isolate* isolate, JSArg buffer) {
	/*auto address = (uintptr_t)node::Buffer::Data(buffer);
	// Get an address relative to the start of the shared memory
	auto identifier = "memory://" + to_string(address - (uintptr_t)SharedMemory::Shared()->Pointer());*/

	return String::NewFromUtf8(isolate, "not implemented");
}
