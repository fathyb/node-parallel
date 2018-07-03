//
// Created by fathy on 06/06/2018.
//

#ifndef NODE_SHARED_BUFFER_STORE_H
#define NODE_SHARED_BUFFER_STORE_H


#include "Types.h"

namespace nsb::addon {
	void GetOrSet(v8::Isolate* isolate, JSArg name, JSArg getCallback, JSArg setCallback);
}


#endif //NODE_SHARED_BUFFER_STORE_H
