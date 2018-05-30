//
// Created by fathy on 26/05/2018.
//

#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <mutex>

#include "SharedBuffer.h"
#include "Exceptions.h"

namespace nsb {
	namespace {
		struct Table {
			bool used;

		};

		Buffer* buffer = nullptr;
		std::mutex bufferMutex;

		Buffer* GetTableBuffer() {
			std::lock_guard<std::mutex> lock(bufferMutex);

			if(buffer == nullptr) {
				buffer = new Buffer("nsb_process_table", 1024 * 1024, true);
			}

			return buffer;
		}

		const std::string GetHandleName(const std::string& name) {
			return "/nsb_handle." + name;
		}
	}

	Buffer::Buffer(const std::string &name, unsigned int size, bool persistent) {
		// Try to open an existing descriptor, create it otherwise
		int fd = shm_open(GetHandleName(name).c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG);

		// If we can't open the file descriptor
		if(fd < 0) {
			throw StdException(errno);
		}
		// If we can't resize the file descriptor
		if (ftruncate(fd, size) < 0) {
			throw StdException(errno);
		}

		// We add a header to save some metadata about the buffer
		unsigned int payloadSize = sizeof(Header) + size;

		// Map the descriptor to the current process virtual address space
		pointer = reinterpret_cast<Header*>(
			mmap(nullptr, payloadSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
		);

		// Initialize the metadata if it just has been created
		if (pointer->magic != Header::MAGIC) {
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
		} else {
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

		munmap(header, sizeof(Header) + header->size);
	}
}
