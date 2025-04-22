#pragma once

#include <mutex>

class Counter
{
private:
	std::atomic<int> count;

private:
	Counter(int startValue = 0);
	~Counter();

	void Increment();
	void Decrement();

	int GetCount() const;

	friend class Scheduler;
};