//
// Created by fathy on 05/06/2018.
//

#ifndef NODE_SHARED_BUFFER_BUFFER_H
#define NODE_SHARED_BUFFER_BUFFER_H

#include <v8.h>

#include "Types.h"

namespace nsb {
	namespace addon {
		JSValue ImportBuffer(JSArg name);
		JSValue ExportBuffer(v8::Isolate* isolate, JSArg buffer);
		JSValue AllocBuffer(JSArg size);
	}
}

#endif //NODE_SHARED_BUFFER_BUFFER_H
