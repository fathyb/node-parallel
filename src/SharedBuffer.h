//
// Created by fathy on 26/05/2018.
//

#ifndef NODE_SHARED_BUFFER_VIRTUALMEMORY_H
#define NODE_SHARED_BUFFER_VIRTUALMEMORY_H

#include <string>
#include <functional>
#include <cstdint>

#include <mutex>

namespace nsb {
	class Buffer {
	public:
		Buffer(const std::string &name, unsigned int size, bool persistent = false);

		char* address() {
			return reinterpret_cast<char*>(pointer) + sizeof(Header);
		}

		size_t size();

		static void free(char* data, void* hint);
	private:
		struct Header {
			static const int MAGIC = 0xBADADBAD;

			int magic;

			// Save the size for further release
			size_t size;

			// A mutex to handle atomic operations
			pthread_mutex_t mutex;

			// A reference counter, once it reaches 0 the descriptor is released
			unsigned int references;

			// A list of process ids using the buffer, using for safe reference counting
			int processTable[1024];
		};

		// A RAII lock utility for atomic access on the buffer
		class Lock {
		public:
			explicit Lock(Header* header): header(header) {
				pthread_mutex_lock(&header->mutex);
			}

			~Lock() {
				pthread_mutex_unlock(&header->mutex);
			}
		private:
			Header* header;
		};

		Header* pointer;
	};
}


#endif //NODE_SHARED_BUFFER_VIRTUALMEMORY_H
