//
// Created by fathy on 26/05/2018.
//

#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>

#include "SharedBuffer.h"
#include "Exceptions.h"

namespace nsb {
	Buffer::Buffer(const std::string &path, unsigned int size) {
		using namespace std;

		// Convert the path to as System V IPC key
		key_t key = ftok(path.c_str(), Header::MAGIC);

		if(key == -1) {
			throw StdException(errno);
		}

		// We add a header to save some metadata about the buffer
		unsigned int payloadSize = sizeof(Header) + size;

		// Get a memory object for the key, create it if needed
		int memoryObject = shmget(key, payloadSize, IPC_CREAT | 0660);

		if(memoryObject == -1) {
			throw StdException(errno);
		}

		// Get a pointer to the shared memory
		void* address = shmat(memoryObject, nullptr, 0);

		if(address == (void*)-1) {
			throw StdException(errno);
		}

		cout << "Hash = " << key << ", Memory object = " << memoryObject << endl;
		pointer = reinterpret_cast<Header*>(address);

		// Initialize the metadata if it just has been created
		if(pointer->magic != Header::MAGIC) {
			// Mark the memory object to be destroyed when all processes remove it
			shmctl(memoryObject, IPC_RMID, nullptr);

			std::cout << "Create buffer" << std::endl;
			pointer->size = size;
			pointer->references = 1;
			pointer->magic = Header::MAGIC;

			// Create a mutex attributes object and set the mutex as shared
			pthread_mutexattr_t attributes;
			pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);

			// Initialize the mutex
			pthread_mutex_init(&pointer->mutex, &attributes);
			pthread_mutexattr_destroy(&attributes);
		}
		else {
			Lock lock(pointer);

			pointer->references++;

			std::cout << "Reuse buffer (usage = " << pointer->references << ")" << std::endl;
		}
	}

	size_t Buffer::size() {
		// Lock in case the buffer is being resized
		Lock lock(pointer);

		return pointer->size;
	}

	void Buffer::free(char *data, void*) {
		auto header = reinterpret_cast<Header*>(data - sizeof(Header));

		{
			Lock lock(header);

			if(header->magic != Header::MAGIC) {
				std::cout << "Memory corrupted reuse" << std::endl;
			}
			auto count = --header->references;

			// If nobody is using the buffer anymore release the descriptor
			if (count <= 0) {
				std::cout << "Should remove buffer (refs = " << count << ")" << std::endl;
			} else {
				std::cout << "Refs = " << count << std::endl;
			}
		}

		// munmap(header, sizeof(Header) + header->size);
	}
}
