//
// Created by fathy on 03/06/2018.
//

#include "IPC.h"

#include "../SharedMemory.h"
#include "Worker.h"
#include "../utils/Configuration.h"

using namespace nsb;
using namespace std;

IPC::IPC(SharedMemory* memory): memory(memory) {}

void IPC::AddMessages(Message *front, size_t size, bool broadcast) {
	if(broadcast) {
		lock_guard<SpinLock> lock(broadcastQueue.lock);

		broadcastQueue.push(make_pair(front, size));

		log(Debug) << "Adding " << size << " messages to the broadcast queue";
	}
	else {
		lock_guard<SpinLock> lock(distributedQueue.lock);

		distributedQueue.push(make_pair(front, size));
		distributedQueueSize += size;

		log(Debug) << "Adding " << size << " messages to the distributed queue";
	}

	DispatchMessages();
}

void IPC::DispatchMessages() {
	lock_guard<SpinLock> workersLock(workers.lock);

	log(Debug) << "Dispatching messages";

	{
		lock_guard<SpinLock> broadcastLock(broadcastQueue.lock);

		while(!broadcastQueue.empty()) {
			auto& front = broadcastQueue.front();

			for(const auto& worker: workers) {
				auto& toProcess = worker->messagesToProcess;
				lock_guard<platform::Condition> lock(toProcess.lock);

				worker->processing++;
				toProcess.push_back(front);
				toProcess.lock.Broadcast();
			}

			broadcastQueue.pop();
		}
	}

	lock_guard<SpinLock> queueLock(distributedQueue.lock);

	if(distributedQueue.empty()) {
		return;
	}

	std::map<Worker*, size_t, Worker::Less> toNotify;

	for(const auto& worker: workers) {
		if(!worker->master) {
			toNotify.insert(make_pair(worker, worker->processing));
		}
	}

	auto leftToQueue = distributedQueueSize;
	bool queued = true;
	int inQueue = 0;

	while(queued && leftToQueue > 0) {
		queued = false;

		for(auto& pair: toNotify) {
			if(pair.second < MAX_CALLS) {
				pair.second++;
				inQueue++;

				if(--leftToQueue == 0) {
					break;
				}

				queued = true;
			}
		}
	}

	log(Debug) << leftToQueue << " left to queue, " << inQueue << " queued";

	for(const auto& pair: toNotify) {
		auto worker = pair.first;
		auto messages = pair.second - worker->processing;

		if(messages == 0) {
			break;
		}

		auto toProcess = &worker->messagesToProcess;

		lock_guard<platform::Condition> lock(toProcess->lock);

		assert(messages <= distributedQueueSize);
		distributedQueueSize -= messages;

		while(messages > 0) {
			auto& front = distributedQueue.front();
			auto start = front.first;
			auto sliced = min(front.second, messages);
			auto cursor = start->Slice(sliced);

			toProcess->push_back(make_pair(start, sliced));

			if(cursor != nullptr && start != cursor) {
				front.first = cursor;
			}
			else {
				distributedQueue.pop();
			}

			log(Debug) << "Giving " << sliced << " messages to worker " << worker->pid;
			worker->processing += sliced;
			messages -= sliced;
		}

		toProcess->lock.Broadcast();
	}
}

void IPC::MessagesProcessed(queue<Message*>& messages) {
	map<Worker*, queue<Message*>> workersQueue;
	auto currentWorker = GetWorker();

	while(!messages.empty()) {
		auto message = messages.front();
		auto worker = message->callingWorker;
		auto it = workersQueue.find(worker);

		if(it == workersQueue.end()) {
			it = workersQueue.insert(make_pair(worker, queue<Message*>())).first;
		}

		it->second.push(message);
		messages.pop();

		{
			lock_guard<SpinLock> lock(workers.lock);
			currentWorker->processing--;
		}
	}

	for(auto& pair: workersQueue) {
		auto worker = pair.first;
		auto& workerQueue = pair.second;
		auto& processed = worker->messagesProcessed;
		lock_guard<platform::Condition> lock(processed.lock);

		while(!workerQueue.empty()) {
			processed.push_back(workerQueue.front());
			workerQueue.pop();
		}

		log(Debug) << "Notifying worker " << worker->pid;

		processed.lock.Broadcast();
	}

	DispatchMessages();
}

void IPC::Set(IPC::StoreEntry* entry, char* value, size_t size) {
	lock_guard<platform::Condition> entryLock(entry->condition);

	entry->value = value;
	entry->size = size;
	entry->state = StoreEntry::State::Resolved;

	entry->condition.Broadcast();
}

IPC::StoreEntry* IPC::GetOrSet(const string& name, function<void(StoreEntry*)> set) {
	StoreEntry* entry = nullptr;
	bool shouldSet = false;

	{
		lock_guard<SpinLock> lock(store.lock);
		auto it = store.find(name);

		if (it == store.end()) {
			entry = memory->New<StoreEntry>();
			store.insert(make_pair(name, entry));

			shouldSet = true;
		}
		else {
			entry = it->second;
		}
	}

	if(shouldSet) {
		set(entry);
	}

	return entry;
}

bool IPC::AddWorker(Worker::Id id) {
	bool master;
	auto pid = platform::GetProcessId();
	auto allocator = memory->GetAllocator(id);
	auto address = allocator->Allocate(sizeof(Worker));

	auto worker = new (address) Worker(pid, id, allocator);

	GlobalConfiguration.worker = worker;

	{
		lock_guard<SpinLock> lock(workers.lock);

		worker->master = master = workers.empty();
		workers.push_back(worker);
	}

	DispatchMessages();

	return master;
}

void IPC::RemoveWorker(pid_t pid) {
	lock_guard<SpinLock> lock(workers.lock);

	log(Debug) << "Removing worker " << pid;

	for(auto it = workers.begin(); it != workers.end(); it++) {
		if((*it)->pid == pid) {
			workers.erase(it);

			return;
		}
	}
}

Worker* IPC::GetWorker(platform::ProcessId pid) {
	log(Debug) << "Locking workers";
	lock_guard<SpinLock> lock(workers.lock);

	log(Debug) << "GetWorker(), this = " << this;

	for(const auto& worker: workers) {
		if(worker->pid == pid) {
			return worker;
		}
	}

	return nullptr;
}
