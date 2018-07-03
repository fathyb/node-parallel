//
// Created by fathy on 02/06/2018.
//

#ifndef NODE_SHARED_BUFFER_CONFIGURATION_H
#define NODE_SHARED_BUFFER_CONFIGURATION_H

#include <string>

#include "../ipc/Worker.h"

namespace nsb {
	struct Configuration {
		void* heap;
		bool master;
		Worker* worker = nullptr;
	};

	// Per-process global configuration
	// Make it external so we can load it in the addon, which is dlopen'd by Node.js
	extern Configuration GlobalConfiguration;
}

#endif //NODE_SHARED_BUFFER_CONFIGURATION_H
