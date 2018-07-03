//
// Created by fathy on 02/06/2018.
//

#ifndef NODE_SHARED_BUFFER_POINTERUTILS_H
#define NODE_SHARED_BUFFER_POINTERUTILS_H

#include <cstdint>

namespace nsb {
	namespace pointer {
		template<typename O = char, typename I>
		constexpr O* StaticCast(I* pointer) {
			return static_cast<O*>(
				static_cast<void*>(pointer)
			);
		}

		// Returns a pointer $offset bytes ahead of $pointer
		template<typename T>
		constexpr T* Offset(T* pointer, intptr_t offset) {
			return StaticCast<T>(
				StaticCast(pointer) + offset
			);
		}

		template<typename T>
		constexpr char* After(T* pointer) {
			return StaticCast(
				Offset(pointer, sizeof(T))
			);
		}

		template<typename T, typename U>
		constexpr T* Before(U* pointer) {
			return StaticCast<T>(
				Offset(StaticCast(pointer), -sizeof(T))
			);
		}
	}
}

#endif //NODE_SHARED_BUFFER_POINTERUTILS_H
