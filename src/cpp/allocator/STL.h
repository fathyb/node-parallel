//
// Created by fathy on 06/06/2018.
//

#ifndef NODE_SHARED_BUFFER_STL_H
#define NODE_SHARED_BUFFER_STL_H

#include <cstddef>
#include <list>
#include <map>
#include <queue>
#include "../utils/SpinLock.h"

namespace nsb {
	namespace memory {
		void* AllocateShared(size_t size);
		void FreeShared(void *ptr);
	}

	template<typename T>
	struct STLAllocator {
		using value_type = T;

		STLAllocator() = default;

		template<typename U>
		explicit STLAllocator(const STLAllocator<U>& allocator) {}

		T* allocate(size_t size) {
			return reinterpret_cast<T*>(memory::AllocateShared(size * sizeof(T)));
		}

		void deallocate(T* ptr, size_t size = 0) {
			memory::FreeShared(reinterpret_cast<void *>(ptr));
		}
	};

	namespace STLTypes {
		template<typename T>
		using List = std::list<T, STLAllocator<T>>;


		template<typename K, typename V>
		using MapElement = std::pair<const K, V>;

		template<typename K, typename V>
		using Map = std::map<K, V, std::less<K>, STLAllocator<MapElement<K, V>>>;
	}


	template<typename S, typename T, typename L>
	class SharedContainer: public S {
	public:
		SharedContainer():
			S(allocator)
		{}

		SharedContainer(const SharedContainer&) = delete;
		SharedContainer& operator=(const SharedContainer&) = delete;

		L lock;
	private:
		STLAllocator<T> allocator;
	};

	template <typename T, typename L = SpinLock>
	using SharedList = SharedContainer<std::list<T, STLAllocator<T>>, T, L>;

	template <typename K, typename V, typename L = SpinLock>
	using SharedMap = SharedContainer<STLTypes::Map<K, V>, STLTypes::MapElement<K, V>, L>;

	template <typename T, typename L = SpinLock>
	using SharedDeque = std::deque<T, STLAllocator<T>>;


	template<typename T, typename L = SpinLock>
	class SharedQueue: public std::queue<T, SharedDeque<T, L>> {
	public:
		SharedQueue():
			std::queue<T, SharedDeque<T, L>>(allocator)
		{}

		SharedQueue(const SharedQueue&) = delete;
		SharedQueue& operator=(const SharedQueue&) = delete;

		L lock;
	private:
		STLAllocator<T> allocator;
	};

}

#endif //NODE_SHARED_BUFFER_STL_H
