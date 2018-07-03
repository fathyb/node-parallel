//
// Created by fathy on 05/06/2018.
//

#ifndef NODE_SHARED_BUFFER_TYPES_H
#define NODE_SHARED_BUFFER_TYPES_H

#include <v8.h>

namespace nsb {
	typedef v8::Local<v8::Value> JSValue;
	typedef const JSValue& JSArg;
}

#endif //NODE_SHARED_BUFFER_TYPES_H
