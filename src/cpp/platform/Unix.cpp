//
// Created by fathy on 08/06/2018.
//

#include <sys/mman.h>
#include <cerrno>
#include <unistd.h>
#include <execinfo.h>
#include <thread>

#include "Unix.h"
#include "../utils/Exceptions.h"
#include "../utils/Logger.h"
#include "../SharedMemory.h"

using namespace nsb;

namespace {
	void Oof(int signal) {
		void* trace[64];
		int size = backtrace(trace, 64);
		auto symbols = backtrace_symbols(trace, size);
		std::string buffer;

		for(int i = 0; i < size; i++) {
			buffer += "\n";
			buffer += symbols[i];
		}

		log(Error) << "Error: " << strsignal(signal) << ")";
		log(Error) << buffer;

		exit(1);
	}
}
void* platform::CreateMemoryRegion(size_t size) {
	// Let everyone read and write the memory
	int protection = PROT_READ | PROT_WRITE;
	// Let the memory be shared and only allocate physical memory when needed
	int flags = MAP_SHARED | MAP_NORESERVE | MAP_ANONYMOUS;
	// Make it private and free the memory when all processes die
	int privateDescriptor = -1;
	int offset = 0;

	// Map the shared memory object in this process address space
	auto address = mmap(nullptr, size, protection, flags, privateDescriptor, offset);

	if(address == (void*)-1) {
		throw StdException(errno);
	}

	return address;
}

namespace  {
	thread_local pid_t currentPid = -1;

	// For non-Linux Unixes
	// Kills children when they become orphan
	// Check should be called each entry and exit of the event-loop
	class ZoombieKiller {
	public:
		explicit ZoombieKiller(pid_t parent) {
			thread = std::thread([this, parent] {
				log(Debug) << "Parent PID " << parent;
				std::unique_lock<std::mutex> lock(mutex);

				condition.wait(lock, [parent] {
					return getppid() != parent || nsb::SharedMemory::Shared()->IsStopped();
				});

				log(Debug) << "Parent " << parent << " vs " << getppid() << " died :( Killing orphan";

				std::abort();
			});
			thread.detach();
		}

		void Check() {
			condition.notify_one();
		}
	private:
		std::condition_variable condition;
		std::mutex mutex;
		std::thread thread;
	};

	ZoombieKiller* killer = nullptr;
}

void platform::ForkProcess(bool* isMasterPtr) {
	pid_t parent = platform::GetProcessId();
	pid_t pid = fork();
	bool isChild = pid == 0;

	currentPid = -1;
	*isMasterPtr = !isChild;
	signal(SIGSEGV, Oof);

	if(isChild) {
		killer = new ZoombieKiller(parent);
	}
}

platform::ProcessId platform::GetProcessId() {
	if(currentPid == -1) {
		pid_t pid;

		// https://yarchive.net/comp/linux/getpid_caching.html
		std::thread([&pid] {
			pid = getpid();
		}).join();

		currentPid = pid;
	}

	return currentPid;
}

platform::ProcessId platform::WaitForChild(int *exitCode) {
	int status;
	auto pid = waitpid(-1, &status, WUNTRACED);

	if(pid == -1) {
		throw StdException(errno);
	}

	*exitCode = WEXITSTATUS(status);

	return pid;
}

void platform::SuicideIfZoombie() {
	if(killer != nullptr) {
		killer->Check();
	}
}

void platform::MapFileInMemory() {

}
