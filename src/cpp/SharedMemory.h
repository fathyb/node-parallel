//
// Created by fathy on 01/06/2018.
//

#ifndef NODE_SHARED_BUFFER_POSIXSHAREDMEMORY_H
#define NODE_SHARED_BUFFER_POSIXSHAREDMEMORY_H

#include <map>

#include "allocator/STL.h"
#include "ipc/IPC.h"
#include "utils/Exceptions.h"
#include "utils/SpinLock.h"
#include "allocator/Allocator.h"
#include "utils/LinkedList.h"

namespace nsb {
	// Allocates the initial virtual memory region and creates a jemalloc arena on top of it
	class SharedMemory {
	public:
		SharedMemory(size_t size, size_t workers);

		static SharedMemory* Shared();

		template<typename T, typename ...A>
		inline T* New(A ...args) {
			return new (Allocate(sizeof(T))) T(args...);
		}


		void* Allocate(size_t size);
		void Free(void* pointer);
		void Stop();
		bool IsStopped();

		int GetId();

		IPC* GetIPC();
		Allocator* GetAllocator(Worker::Id id);
	private:
		struct MemoryHeader {
			MemoryHeader(SharedMemory* memory, size_t workers):
				ipc(memory),
				counter(0),
				workers(workers)
			{}

			IPC ipc;
			int counter;
			size_t workers;
			bool stop = false;

			struct Region {
				typedef LinkedList<Region> List;

				Region(char* space, size_t size, Worker::Id id):
					allocator(space, size),
					id(id)
				{}

				Allocator allocator;
				Worker::Id id;
			};

			LinkedList<Region>* regions = nullptr;
		};

		SharedMemory();

		static thread_local SharedMemory* sharedInstance;
		MemoryHeader* header;
	};

}


#endif //NODE_SHARED_BUFFER_POSIXSHAREDMEMORY_H
