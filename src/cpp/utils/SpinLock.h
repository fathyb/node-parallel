//
// Created by fathy on 01/06/2018.
//

#ifndef NODE_SHARED_BUFFER_SPINLOCK_H
#define NODE_SHARED_BUFFER_SPINLOCK_H

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include "Logger.h"

namespace nsb {
	// Find an efficient way to make the CPU spin
	inline static void PauseCPU() {
		// Prevent memory order violations (and pipeline flushes) by waiting for the memory pipeline to become empty
		// Only needed for SMT-enabled CPUs
		#if __linux__ || defined(cpu_relax)
			// https://github.com/torvalds/linux/blob/ead751507de86d90fa250431e9990a8b881f713c/arch/x86/um/asm/processor.h#L29
			cpu_relax();
		#elif defined(YieldProcessor)
			// https://msdn.microsoft.com/en-us/library/ms687419(v=vs.85).aspx
			YieldProcessor()
		#elif defined(__x86_64__)
			// https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-305.html
			asm volatile("pause" ::: "memory");
		#endif
	}

	// Used for short-delay (~ns) lock operations, not recursive
	class SpinLock {
	public:
		SpinLock() = default;
		SpinLock(const SpinLock&) = delete;

		void lock() {
			while(locked.test_and_set(std::memory_order_acquire)) {
				PauseCPU();
			}
		}

		void unlock() {
			locked.clear(std::memory_order_release);
		}
	private:
		std::atomic_flag locked = ATOMIC_FLAG_INIT;
	};
}


#endif //NODE_SHARED_BUFFER_SPINLOCK_H
