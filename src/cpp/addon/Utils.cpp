//
// Created by fathy on 06/06/2018.
//

#include "Utils.h"
#include "../SharedMemory.h"
namespace {
	class SerializerDelegate: public v8::ValueSerializer::Delegate {
	public:
		void ThrowDataCloneError(v8::Local<v8::String> message) override {
			throw std::bad_exception();
		}

		void* ReallocateBufferMemory(void* oldBuffer, size_t size, size_t* actualSize) override {
			*actualSize = size;

			if(oldBuffer != nullptr) {
				// nsb::SharedMemory::Shared()->Free(oldBuffer);
			}

			auto ptr = nsb::SharedMemory::Shared()->Allocate(size);

			// log(nsb::Error) << "Delegate allocated " << ptr;

			return ptr;
		}

		void FreeBufferMemory(void* buffer) override {}
	};

	thread_local SerializerDelegate SharedDelegate;
}

std::pair<char*, size_t> nsb::SerializeJSValue(v8::Isolate* isolate, JSArg value) {
	v8::ValueSerializer serializer(isolate);

	serializer.WriteHeader();
	assert(serializer.WriteValue(isolate->GetCallingContext(), value).ToChecked());

	auto r = serializer.Release();
	auto ptr = SharedMemory::Shared()->Allocate(r.second);

	memcpy(ptr, r.first, r.second);

	return std::make_pair(static_cast<char*>(ptr), r.second);
}

v8::Local<v8::Value> nsb::DeserializeValue(v8::Isolate* isolate, char* pointer, size_t size) {
	auto ctx = isolate->GetCurrentContext();
	v8::ValueDeserializer deserializer(isolate, reinterpret_cast<uint8_t*>(pointer), size);

	assert(deserializer.ReadHeader(ctx).ToChecked());
	auto result = deserializer.ReadValue(ctx).ToLocalChecked();

	return result;
}