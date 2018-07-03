//
// Created by fathy on 08/06/2018.
//

#ifndef NODE_SHARED_BUFFER_UNIX_PLATFORM_H
#define NODE_SHARED_BUFFER_UNIX_PLATFORM_H

#include <pthread.h>
#include <cassert>
#include <functional>
#include <mutex>
#include "../utils/Logger.h"

namespace nsb {
	namespace platform {
		using ProcessId = pid_t;

		void* CreateMemoryRegion(size_t size);
		void ForkProcess(bool* isMaster);
		ProcessId GetProcessId();
		ProcessId WaitForChild(int* exitCode);

		void MapFileInMemory();

		void SuicideIfZoombie();

		template <int A>
		class POSIXMutex {
		public:
			typedef std::lock_guard<POSIXMutex> Guard;

			POSIXMutex() {
				pthread_mutexattr_t attributes;

				pthread_mutexattr_init(&attributes);
				pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);
				pthread_mutexattr_settype(&attributes, A);

				pthread_mutex_init(&mutex, &attributes);
				pthread_mutexattr_destroy(&attributes);
			}

			void lock() {
				assert(pthread_mutex_lock(&mutex) == 0);
			}

			void unlock() {
				assert(pthread_mutex_unlock(&mutex) == 0);
			}
		protected:
			pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
		};

		using Lock = POSIXMutex<PTHREAD_MUTEX_ERRORCHECK>;
		// using RecursiveLock = POSIXMutex<PTHREAD_MUTEX_RECURSIVE>;

		class Condition: public Lock {
		public:
			Condition(const Condition&) = delete;

			Condition(): Lock() {
				pthread_condattr_t conditionAttributes;

				pthread_condattr_init(&conditionAttributes);
				pthread_condattr_setpshared(&conditionAttributes, PTHREAD_PROCESS_SHARED);

				pthread_cond_init(&condition, &conditionAttributes);
				pthread_condattr_destroy(&conditionAttributes);
			}

			void Wait(const std::function<bool()>& check) {
				Guard lock(*this);

				while(true) {
					if(check()) {
						return;
					}

					Wait();
				}
			}

			void Wait() {
				pthread_cond_wait(&condition, &mutex);
			}

			void Broadcast() {
				pthread_cond_broadcast(&condition);
			}
		private:
			pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
		};
	}

}


#endif //NODE_SHARED_BUFFER_UNIX_PLATFORM_H
