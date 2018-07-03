//
// Created by fathy on 24/06/2018.
//

#ifndef NODE_SHARED_BUFFER_LINKEDLIST_H
#define NODE_SHARED_BUFFER_LINKEDLIST_H

namespace nsb {
	template <typename T>
	struct LinkedList: public T {
		using T::T;

		LinkedList<T>* next = nullptr;
	};
}

#endif //NODE_SHARED_BUFFER_LINKEDLIST_H
