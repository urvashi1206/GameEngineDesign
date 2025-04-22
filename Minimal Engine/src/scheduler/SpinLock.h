#pragma once

#include <mutex>

class SpinLock
{
private:
	std::atomic_flag locked;

public:
	SpinLock();
	~SpinLock();

	bool TryAcquire();
	void Acquire();

	void Release();
};