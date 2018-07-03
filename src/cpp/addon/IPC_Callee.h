//
// Created by fathy on 11/06/2018.
//

#ifndef NODE_SHARED_BUFFER_IPC_CALLEE_H
#define NODE_SHARED_BUFFER_IPC_CALLEE_H

#include <v8.h>
#include <nan.h>

#include "Types.h"

namespace nsb {
	namespace addon {
		void WaitForMessage(JSArg callback);
	}
}


#endif //NODE_SHARED_BUFFER_IPC_CALLEE_H
