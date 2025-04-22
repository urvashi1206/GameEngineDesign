#include "Counter.h"

#include "Scheduler.h"

Counter::Counter(int startValue) : 
	count(startValue)
{

}
Counter::~Counter()
{
	
}

void Counter::Increment()
{
	int oldValue = count.fetch_add(1, std::memory_order_relaxed);
	if(oldValue + 1 == 0)
		Scheduler::RestoreFibersFromWaitList(this);
}
void Counter::Decrement()
{
	int oldValue = count.fetch_sub(1, std::memory_order_relaxed);
	if(oldValue - 1 == 0)
		Scheduler::RestoreFibersFromWaitList(this);
}

int Counter::GetCount() const
{
	return count.load();
}