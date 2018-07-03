//
// Created by fathy on 06/06/2018.
//

#include <nan.h>
#include <utility>
#include "Store.h"
#include "../SharedMemory.h"
#include "Types.h"
#include "Utils.h"

using namespace nsb;
using namespace v8;
using namespace Nan;
using namespace std;


namespace nsb::addon::store {
	struct SetData {
		SetData(
			IPC::StoreEntry* entry,
			Callback* callback
		):
			entry(entry),
			callback(callback),
			resource("SetData")
		{}

		IPC::StoreEntry* entry;
		Callback* callback;
		AsyncResource resource;
	};

	void StoreSave(const v8::FunctionCallbackInfo<Value>& info) {
		auto result = SerializeJSValue(info.GetIsolate(), info[0]);
		auto data = static_cast<SetData*>(
			Handle<External>::Cast(info.Data())->Value()
		);

		SharedMemory::Shared()->GetIPC()->Set(data->entry, result.first, result.second);
		Local<Value> args[1] = {info[0]};

		data->callback->Call(1, args, &data->resource);
	}

	class Worker: public AsyncWorker {
	public:
		Worker(
			std::string name,
			IPC::StoreEntry* entry,
			Callback* callback
		):
			AsyncWorker(callback),
			name(std::move(name)),
			entry(entry)
		{}

		void Execute() override {
			log(Debug) << "Waiting for " << name;
			platform::Condition::Guard lock(entry->condition);

			while(entry->state == IPC::StoreEntry::State::Pending) {
				entry->condition.Wait();
			}
			log(Debug) << name << " arrived";
		}

	protected:
		void HandleOKCallback() override {
			Local<Value> args[1] = {
				DeserializeValue(Isolate::GetCurrent(), entry->value, entry->size)
			};

			assert(entry->state == IPC::StoreEntry::State::Resolved);

			log(Debug) << "Calling callback " << name;
			callback->Call(1, args, async_resource);
		}

	private:
		std::string name;
		IPC::StoreEntry* entry;
	};

	class TestWorker: public AsyncWorker {
	public:
		explicit TestWorker(Callback* callback, IPC::StoreEntry* entry):
			AsyncWorker(callback),
			entry(entry)
		{}

		void Execute() override {}

	protected:
		void HandleOKCallback() override {
			auto isolate = Isolate::GetCurrent();
			auto doneCallback = v8::FunctionTemplate::New(
				isolate,
				&store::StoreSave,
				External::New(isolate, entry)
			);
			Local<Value> args[1] = {doneCallback->GetFunction()};

			log(Debug) << "Calling set()";
			callback->Call(1, args, async_resource);
			log(Debug) << "Called set()";
		}
	private:
		IPC::StoreEntry* entry;
	};
}


void addon::GetOrSet(Isolate *isolate, JSArg name, JSArg getCallback, JSArg setCallback) {
	log(Debug) << "In GetOrSet";
	Nan::Utf8String utfName(name);
	bool set = false;
	auto entry = SharedMemory::Shared()->GetIPC()->GetOrSet(
		*utfName,
		[&](IPC::StoreEntry* entry) {
			auto data = new store::SetData(
				entry,
				new Callback(Handle<Function>::Cast(getCallback))
			);

			auto doneCallback = v8::FunctionTemplate::New(
				isolate,
				&store::StoreSave,
				External::New(isolate, data)
			);
			Local<Value> args[1] = {doneCallback->GetFunction()};

			Handle<Function>::Cast(setCallback)->Call(isolate->GetCurrentContext()->Global(), 1, args);
			set = true;
		}
	);

	if(!set) {
		auto callback = Handle<Function>::Cast(getCallback);

		if(entry->state == IPC::StoreEntry::State::Resolved) {
			Local<Value> args[1] = {
				DeserializeValue(Isolate::GetCurrent(), entry->value, entry->size)
			};

			Call(Callback(callback), 1, args);
		}
		else {
			AsyncQueueWorker(
				new store::Worker(*utfName, entry, new Callback(callback))
			);
		}
	}
	log(Debug) << "Out GetOrSet";
}
