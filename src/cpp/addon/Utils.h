//
// Created by fathy on 06/06/2018.
//

#ifndef NODE_SHARED_BUFFER_ADDON_UTILS_H
#define NODE_SHARED_BUFFER_ADDON_UTILS_H

#include "Types.h"

namespace nsb {
	std::pair<char*, size_t> SerializeJSValue(v8::Isolate* isolate, JSArg value);
	v8::Local<v8::Value> DeserializeValue(v8::Isolate* isolate, char* pointer, size_t size);
}

#endif //NODE_SHARED_BUFFER_ADDON_UTILS_H
