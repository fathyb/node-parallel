//
// Created by fathy on 11/06/2018.
//

#ifndef NODE_SHARED_BUFFER_MESSAGE_H
#define NODE_SHARED_BUFFER_MESSAGE_H

#include <cstdlib>
#include <string>
#include <cassert>

#include "../utils/SpinLock.h"

namespace nsb {
	struct Worker;
	struct MessageContainer {
		std::atomic<size_t> references;

		char* Start();
		void Release();
	};
	struct Message {
		using WithSize = std::pair<Message*, size_t>;

		static MessageContainer* Make(size_t size);

		explicit Message(
			const std::string& name,
			char* ptr,
			size_t size,
			Worker* worker,
			MessageContainer* container,
			int groupId
		);

		struct SerializedValue {
			char* pointer = nullptr;
			size_t size = 0;
		};

		char name[32] = "";
		int groupId;
		SerializedValue input;
		SerializedValue output;
		SpinLock lock;

		Worker* callingWorker = nullptr;
		Message* prev = nullptr;
		Message* next = nullptr;
		MessageContainer* container = nullptr;

		inline void InsertAfter_NoLock(Message* other) {
			prev = other;
			next = other->next;
			other->next = this;
		}

		Message* Slice(size_t length);
	};
}

#endif //NODE_SHARED_BUFFER_MESSAGE_H
