//
// Created by fathy on 31/05/2018.
//

#include <iostream>
#include <thread>

#include <node.h>

#include "SharedMemory.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"
#include "platform/Platform.h"
#include "utils/Exceptions.h"

using namespace std;
using namespace nsb;

namespace {
	int ForkWorker(Worker::Id id, int argc, char* argv[], SharedMemory* memory, bool* master) {
		// Fork the process, this will create a new process with the memory block mapped in its virtual address space
		platform::ForkProcess(master);

		if(!*master) {
			bool isMaster = memory->GetIPC()->AddWorker(id);

			log(Info) << "Child forked (as " << (isMaster ? "master" : "child") << "), starting Node.js";

			GlobalConfiguration.master = isMaster;
			return node::Start(argc, argv);
		}
		else {
			return 0;
		}
	}
}

int main(int argc, char *argv[]) {
	log(Debug) << "Booting";
	size_t gigabyte = 1024 * 1024 * 1024;
	size_t maxMemory = 1 * gigabyte;
	auto workers = thread::hardware_concurrency();
	int argsBufferSize = 0;
	vector<string> validArgs;

	validArgs.reserve(static_cast<unsigned long>(argc));

	// Ok, let's parse the arguments
	for(int i = 0; i < argc; ++i) {
		string arg(argv[i]);
		string workerPrefix("--workers=");
		string memoryPrefix("--max-memory=");

		log(Debug) << "Arg[" << i << "] = " << arg;

		if(!arg.compare(0, workerPrefix.size(), workerPrefix)) {
			workers = (unsigned int)stoi(arg.substr(workerPrefix.size()));
		}
		else if(!arg.compare(0, memoryPrefix.size(), memoryPrefix)) {
			maxMemory = size_t(stoi(arg.substr(memoryPrefix.size())));
		}
		else {
			// If the argument is unknown pass it to Node.js
			validArgs.push_back(arg);
			argsBufferSize += arg.size() + 1; // size + null terminator
		}
	}

	argc = static_cast<int>(validArgs.size());

	// libuv needs argv to be contiguous
	auto argsBuffer = new char[argsBufferSize];
	int argsBufferOffset = 0;
	argv = new char*[argc];

	for(int i = 0; i < argc; i++) {
		auto arg = validArgs[i];
		auto dest = &argsBuffer[argsBufferOffset];

		// Set the reference to the args buffer
		argv[i] = dest;

		// Copy the string to the buffer
		copy(arg.begin(), arg.end(), dest);

		// Null-terminate the string
		dest[arg.size()] = 0;

		// Add the new content size to the offset
		argsBufferOffset += arg.size() + 1;
	}

	log(Debug) << "Allocating " << maxMemory << " bytes of virtual memory";

	SharedMemory memory(maxMemory, workers);

	GlobalConfiguration.master = true;

	Worker::Id id = 0;
	bool master;

	log(Debug) << "Forking " << workers << " workers";

	do {
		int status = ForkWorker(id + 1, argc, argv, &memory, &master);

		if(!master) {
			return status;
		}
	} while(id++ < workers);

	log(Info) << "Waiting for a child to die";

	auto ipc = memory.GetIPC();

	while(true) {
		int exitCode = -1;
		auto pid = platform::WaitForChild(&exitCode);
		auto worker = ipc->GetWorker(pid);

		if(worker && worker->master) {
			log(Info) << "Master died, quitting";
			memory.Stop();

			return exitCode;
		}

		ipc->RemoveWorker(pid);

		log(Info) << "Child " << pid << " died, restarting, my pid = " << platform::GetProcessId();

		// ForkWorker(argc, argv, &memory, &master);

		abort();
	}
}
