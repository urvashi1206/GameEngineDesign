#include "SpinLock.h"

SpinLock::SpinLock()
{

}
SpinLock::~SpinLock()
{

}

bool SpinLock::TryAcquire()
{
	// use an acquire fence to ensure all subsequent
	// reads by this thread will be valid
	bool isAlreadyLocked = locked.test_and_set(std::memory_order_acquire);
	return !isAlreadyLocked;
}
void SpinLock::Acquire()
{
	// spin until successful acquire
	while (!TryAcquire())
	{
		// reduce power consumption on Intel CPUs
		// (can substitute with std::this_thread::yield()
		// on platforms that don’t support CPU pause, if
		// thread contention is expected to be high)
		std::this_thread::yield();
	}
}

void SpinLock::Release()
{
	// use release semantics to ensure that all prior
	// writes have been fully committed before we unlock
	locked.clear(std::memory_order_release);
}