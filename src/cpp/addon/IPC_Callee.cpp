//
// Created by fathy on 11/06/2018.
//

#include "IPC_Callee.h"

#include <functional>

#include "../SharedMemory.h"
#include "../ipc/IPC.h"
#include "../utils/Logger.h"

#include "Utils.h"

using namespace nsb;
using namespace std;

using v8::Local;
using v8::Handle;
using v8::Value;
using v8::External;
using v8::Array;
using v8::Function;
using v8::Isolate;

namespace {
	void LoopCallback(uv_async_t *async);
	struct WaitForMessageData {
		explicit WaitForMessageData(const Local<v8::Function>& callback):
			callback(callback),
			asyncResource("nsb::MessageToProcess")
		{
			async.data = this;
			uv_async_init(uv_default_loop(), &async, &LoopCallback);
		}

		Nan::Callback callback;
		size_t queueSize = 0;
		bool scheduled = false;
		std::mutex mutex;
		std::queue<Message::WithSize> queue;

		uv_async_t async;
		Nan::AsyncResource asyncResource;
	};

	void OnMessageProcessed(const v8::FunctionCallbackInfo<Value>& info) {
		auto array = Handle<Array>::Cast(info[0]);
		auto length = array->Length();
		queue<Message*> messages;

		for(uint32_t i = 0; i < length; i++) {
			auto object = Handle<Array>::Cast(array->Get(i));
			auto external = Handle<External>::Cast(object->Get(0))->Value();
			auto message = static_cast<Message*>(external);
			auto buffer = SerializeJSValue(info.GetIsolate(), object->Get(1));

			message->output.pointer = buffer.first;
			message->output.size = buffer.second;

			messages.push(message);
		}

		log(Debug) << "Marking " << messages.size() << " messages as processed";
		SharedMemory::Shared()->GetIPC()->MessagesProcessed(messages);
	}

	void LoopCallback(uv_async_t *async) {
		log(Debug) << "Message processing libuv task launched";
		auto data = static_cast<WaitForMessageData*>(async->data);
		lock_guard<mutex> lock(data->mutex);

		Nan::HandleScope scope;
		Nan::Callback* callback =  &data->callback;
		auto& queue = data->queue;

		auto isolate = Isolate::GetCurrent();
		auto ctx = isolate->GetCurrentContext();
		auto size = int(data->queueSize);
		auto memory = SharedMemory::Shared();
		auto array = Array::New(isolate, size);
		uint32_t position = 0;

		while(!queue.empty()) {
			for(auto message = queue.front().first; message != nullptr; message = message->next) {
				auto external = External::New(isolate, static_cast<void*>(message));
				auto payload = &message->input;
				auto value = DeserializeValue(isolate, payload->pointer, payload->size);
				auto object = Array::New(isolate, 3);

				object->Set(0, value);
				object->Set(1, v8::String::NewFromUtf8(isolate, message->name));
				object->Set(2, external);

				array->Set(position++, object);

				// memory->Free(pointer);
			}

			queue.pop();
		}

		assert(position == size);

		log(Debug) << "Calling JavaScript worker with " << size << " messages";

		auto done = v8::FunctionTemplate::New(isolate, OnMessageProcessed)->GetFunction();

		Local<Value> args[2] = {array, done};

		data->asyncResource.runInAsyncScope(ctx->Global(), callback->GetFunction(), 2, args);
		data->scheduled = false;
		data->queueSize = 0;
	}

	void WaitForMessageToProcess(uv_work_t* req) {
		auto data = static_cast<WaitForMessageData*>(req->data);
		auto worker = SharedMemory::Shared()->GetIPC()->GetWorker();

		log(Debug) << "Worker address " << worker;

		assert(worker != nullptr);

		auto& toProcess = worker->messagesToProcess;
		auto queue = &data->queue;

		toProcess.lock.Wait([=, &toProcess] {
			lock_guard<mutex> lock(data->mutex);
			log(Debug) << "Received " << toProcess.size() << " messages to process";

			while(!toProcess.empty()) {
				auto pair = toProcess.front();

				assert(pair.second > 0);
				data->queueSize += pair.second;
				queue->push(pair);
				toProcess.pop_front();
			}


			if(!data->scheduled && !queue->empty()) {
				log(Debug) << "Scheduling IPC message processing";
				data->scheduled = true;
				uv_async_send(&data->async);
			}

			return false;
		});
	}

	void NoOp(uv_work_t* req, int status) {
		// delete req;
	}
}

void addon::WaitForMessage(JSArg jsCallback) {
	auto request = new uv_work_t();

	request->data = new WaitForMessageData(Handle<Function>::Cast(jsCallback));

	log(Debug) << "WaitForMessage called";

	uv_queue_work(uv_default_loop(), request, &WaitForMessageToProcess, &NoOp);
}
