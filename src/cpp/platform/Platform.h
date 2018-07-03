
//
// Created by fathy on 08/06/2018.
//

#ifndef NODE_SHARED_BUFFER_GLOBAL_PLATFORM_H
#define NODE_SHARED_BUFFER_GLOBAL_PLATFORM_H

#if __linux__ || defined(__unix__) || defined(__APPLE__)
	#include "Unix.h"
#else
	#error "Platform not supported"
#endif

#endif //NODE_SHARED_BUFFER_GLOBAL_PLATFORM_H
