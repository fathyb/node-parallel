//
// Created by fathy on 03/06/2018.
//

#ifndef NODE_SHARED_BUFFER_IPC_H
#define NODE_SHARED_BUFFER_IPC_H


#include <map>
#include <list>

#include "../utils/SpinLock.h"
#include "../allocator/STL.h"
#include "../platform/Platform.h"
#include "Message.h"
#include "Worker.h"

namespace nsb {
	class SharedMemory;

	static const long MAX_CALLS = 256;

	class IPC {
	public:
		struct StoreEntry {
			StoreEntry() = default;

			StoreEntry(const StoreEntry&) = delete;

			enum class State {
				Pending, Resolved
			};

			State state = State::Pending;
			char* value = nullptr;
			size_t size = 0;

			platform::Condition condition;
		};

		explicit IPC(SharedMemory* memory);

		void AddMessages(Message *messages, size_t size, bool broadcast);
		void MessagesProcessed(std::queue<Message*>& messages);

		bool AddWorker(Worker::Id id);
		void RemoveWorker(pid_t pid);
		Worker* GetWorker(platform::ProcessId pid = platform::GetProcessId());

		StoreEntry* GetOrSet(
			const std::string& name,
			std::function<void(StoreEntry*)> set
		);
		void Set(StoreEntry* entry, char* value, size_t size);
	private:
		void DispatchMessages();

		SharedList<Worker*> workers;
		SharedQueue<Message::WithSize> distributedQueue;
		size_t distributedQueueSize = 0;

		SharedQueue<Message::WithSize> broadcastQueue;

		SharedMemory* memory;

		SharedMap<std::string, StoreEntry*> store;
	};
}


#endif //NODE_SHARED_BUFFER_IPC_H
