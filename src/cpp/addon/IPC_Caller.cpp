//
// Created by fathy on 11/06/2018.
//

#include "IPC_Caller.h"

#include <functional>

#include "../SharedMemory.h"
#include "../ipc/IPC.h"
#include "../utils/Logger.h"

#include "Utils.h"
#include <nan.h>

using namespace nsb;
using namespace std;
using namespace Nan;

using v8::Local;
using v8::Handle;
using v8::Value;
using v8::External;
using v8::Array;
using v8::Function;
using v8::Isolate;


namespace {
	void QueuedMessageProcessed(uv_async_t* async);

	struct QueueData {
		QueueData(
			const Local<Function>& callback,
			Message* message,
			size_t messages,
			bool broadcast,
			int groupId
		):
			callback(callback),
			message(message),
			messages(messages),
			deletionCounter(messages),
			broadcast(broadcast),
			groupId(groupId),
			asyncResource("nsb::MessageToProcess"),
			async()
		{
			async.data = this;
			uv_async_init(uv_default_loop(), &async, &QueuedMessageProcessed);
		}

		std::mutex mutex;
		std::condition_variable condition;
		Nan::Callback callback;
		Message* message;
		std::queue<Message*> queue;
		size_t messages;
		size_t deletionCounter;
		bool scheduled = false;
		bool broadcast;
		int groupId;

		uv_async_t async;
		Nan::AsyncResource asyncResource;
	};

	void QueuedMessageProcessed(uv_async_t* async) {
		auto data = static_cast<QueueData*>(async->data);
		lock_guard<mutex> lock(data->mutex);
		Nan::HandleScope scope;
		auto& callback = data->callback;
		auto& queue = data->queue;

		auto isolate = Isolate::GetCurrent();
		auto ctx = isolate->GetCurrentContext();
		auto array = Array::New(isolate, int(queue.size()));
		auto memory = SharedMemory::Shared();

		for(uint32_t i = 0; !queue.empty(); i++, queue.pop()) {
			auto message = queue.front();
			auto& payload = message->output;
			auto pointer = payload.pointer;
			auto value = DeserializeValue(isolate, pointer, payload.size);

			array->Set(i, value);

			message->container->Release();
			// memory->Free(pointer);
		}

		Local<Value> args[1] = {array};
		data->asyncResource.runInAsyncScope(ctx->Global(), callback.GetFunction(), 1, args);

		if(data->deletionCounter == 0) {
			// delete data;
		}
		else {
			assert(data->queue.size() == 0);
			data->scheduled = false;
		}

		// delete async;
	}

	void QueueMessages(uv_work_t *req) {
		auto data = static_cast<QueueData*>(req->data);
		auto ipc = SharedMemory::Shared()->GetIPC();
		auto worker = ipc->GetWorker();

		log(Debug) << "Sending " << data->messages << " IPC messages";

		ipc->AddMessages(data->message, data->messages, data->broadcast);
		worker->pollThread->Listen([=] {
			lock_guard<mutex> lock(data->mutex);
			auto& processed = worker->messagesProcessed;

			while(!processed.empty()) {
				auto message = processed.front();

				if(message->groupId == data->groupId) {
					data->deletionCounter--;

					log(Debug) << "MessageId = " << message->groupId << ", DataId = " << data->groupId << ", counter = " << data->deletionCounter;
				}

				data->queue.push(message);
				processed.pop_front();
			}

			if(!data->scheduled && !data->queue.empty()) {
				data->scheduled = true;
				uv_async_send(&data->async);
			}

			assert(data->deletionCounter >= 0);
			return data->deletionCounter == 0;
		});

		return;
		{
			unique_lock<std::mutex> lock(data->mutex);

			data->condition.wait(lock, [data] {
				return data->deletionCounter == 0 && data->queue.empty();
			});

			log(Debug) << "Finished!";
		}
	}

	void NoOp(uv_work_t* req, int status) {
		// delete req;
	}
}

void addon::QueueIPCMessage(Isolate* isolate, JSArg messages, JSArg jsCallback, JSArg broadcast) {
	auto callback = Handle<Function>::Cast(jsCallback);
	auto array = Handle<Array>::Cast(messages);
	auto length = array->Length();
	auto memory = SharedMemory::Shared();
	auto worker = memory->GetIPC()->GetWorker();
	Message* prevMessage = nullptr;
	Message* firstMessage = nullptr;
	auto container = Message::Make(length);
	auto start = container->Start();
	auto groupId = memory->GetId();

	for(uint32_t i = 0; i < length; i++) {
		auto message = Handle<Array>::Cast(array->Get(i));
		auto name = message->Get(0);
		auto args = message->Get(1);
		auto buffer = SerializeJSValue(isolate, args);
		auto address = start + i * sizeof(Message);
		Nan::Utf8String nameString(name);
		auto ipcMessage = new (address) Message(
			*nameString,
			buffer.first, buffer.second,
			worker,
			container,
			groupId
		);

		if(!firstMessage) {
			firstMessage = ipcMessage;
		}
		else {
			ipcMessage->InsertAfter_NoLock(prevMessage);
		}

		prevMessage = ipcMessage;
	}

	auto request = new uv_work_t();

	request->data = new QueueData(callback, firstMessage, length, broadcast->BooleanValue(), groupId);

	uv_queue_work(uv_default_loop(), request, &QueueMessages, &NoOp);
}
