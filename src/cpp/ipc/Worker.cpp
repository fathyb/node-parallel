//
// Created by fathy on 17/06/2018.
//

#include <jemalloc/jemalloc.h>

#include "Worker.h"
#include "../SharedMemory.h"

using namespace nsb;
using namespace std;

PollThread::PollThread(Worker& worker) {
	toProcessThread = std::thread([&]() -> void {
		worker.messagesProcessed.lock.Wait([&]() -> bool {
			lock_guard<SpinLock> guard(lock);

			for(auto it = listeners.begin(); it != listeners.end(); it++) {
				if((*it)()) {
					it = listeners.erase(it);
				}
			}

			return false;
		});
	});

	freeThread = std::thread([&]() -> void {
		auto& queue = worker.freeQueue;
		auto allocator = worker.allocator;

		queue.lock.Wait([&]() -> bool {
			while(!queue.empty()) {
				log(Error) << "Freeing ptr " << queue.front();
				allocator->Free(queue.front());

				queue.pop_front();
			}

			return false;
		});
	});

	toProcessThread.detach();
	freeThread.detach();
}

void PollThread::Listen(const Listener &listener) {
	lock_guard<SpinLock> guard(lock);

	listeners.push_back(listener);
}
