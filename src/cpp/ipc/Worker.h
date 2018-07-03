//
// Created by fathy on 11/06/2018.
//

#ifndef NODE_SHARED_BUFFER_WORKER_H
#define NODE_SHARED_BUFFER_WORKER_H

#include <thread>

#include "../platform/Platform.h"
#include "../allocator/STL.h"
#include "Message.h"
#include "../allocator/Allocator.h"

namespace nsb {
	struct Worker;
	// Should be in the process private address space
	class PollThread {
	public:
		typedef std::function<bool()> Listener;

		explicit PollThread(Worker& worker);

		void Listen(const Listener& listener);
	private:
		std::thread toProcessThread;
		std::thread freeThread;
		std::list<Listener> listeners;
		SpinLock lock;
	};

	struct Worker {
		typedef unsigned int Id;

		struct Less {
			bool operator()(const Worker* leftWorker, const Worker* rightWorker) const {
				auto left = leftWorker->processing;
				auto right = rightWorker->processing;

				if(left == right) {
					return leftWorker < rightWorker;
				}
				else {
					return left < right;
				}
			}
		};

		Worker(platform::ProcessId pid, Id id, Allocator* allocator):
			pid(pid),
			id(id),
			allocator(allocator),
			pollThread(new PollThread(*this))
		{}

		bool operator<(const Worker& other) {
			return processing < other.processing;
		}

		bool IsCurrent() {
			return pid == platform::GetProcessId();
		}

	   	bool master = false;
		pid_t pid;
		Id id;
		Allocator* allocator;
		SharedList<Message::WithSize, platform::Condition> messagesToProcess;
		SharedList<Message*, platform::Condition> messagesProcessed;
		SharedList<void*, platform::Condition> freeQueue;
		PollThread* pollThread;
		int processing = 0;
	};
}

#endif //NODE_SHARED_BUFFER_WORKER_H
