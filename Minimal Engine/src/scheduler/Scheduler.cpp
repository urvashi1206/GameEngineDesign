#include "Scheduler.h"

#include <thread>
#include <mutex>
#include <iostream>

#include <cassert>

#include "Windows.h"

// The main worker fiber for the local thread
thread_local void* localFiber;

static Scheduler* instance;

Scheduler::Scheduler() : 
	primaryFiber(nullptr)
{
	
}
Scheduler::~Scheduler()
{
	for(Counter* c : counters)
	{
		delete c;
		c = nullptr;
	}
	counters.clear();

	DestroyFiberPool();
}

void Scheduler::Startup()
{
	instance = this;

	taskQueues.emplace(TaskPriority::LOW, std::queue<FiberEntryParams>());
	taskQueues.emplace(TaskPriority::MEDIUM, std::queue<FiberEntryParams>());
	taskQueues.emplace(TaskPriority::HIGH, std::queue<FiberEntryParams>());

	InitializeFiberPool();
}
void Scheduler::Shutdown()
{
	instance->shouldTerminate = true;
}

void Scheduler::Run()
{
	static const int THREAD_COUNT = 2;

	primaryFiber = ConvertThreadToFiber(0);
	localFiber = primaryFiber;

	// Launch all worker threads
	for(int i = 0; i < THREAD_COUNT; i++)
		threads.emplace_back([]()
		{
			localFiber = ConvertThreadToFiber(0);
			
			ExecuteWorkerThread();
		});

	ExecuteWorkerThread();

	/* Shutdown */

	// Before shutdown, wait for all threads to complete
	for(std::thread& t : threads)
		t.join();
}
void Scheduler::ExecuteWorkerThread()
{
	while(!instance->shouldTerminate)
	{
		//instance->cv_taskAvailable.wait(,);
		while(instance->taskQueues[TaskPriority::LOW].size() == 0 
			&& instance->taskQueues[TaskPriority::MEDIUM].size() == 0 
			&& instance->taskQueues[TaskPriority::HIGH].size() == 0 
			&& instance->restoredFibersQueue.size() == 0
			&& !instance->shouldTerminate)
			std::this_thread::yield();

		// If there is a restored fiber available
		if(instance->restoredFibersQueue.size() > 0)
		{
			instance->lock_restoredFibersQueue.Acquire();

			if(instance->restoredFibersQueue.size() == 0)
			{
				instance->lock_restoredFibersQueue.Release();
				continue;
			}

			void* restoredFiber = instance->restoredFibersQueue.front();
			instance->restoredFibersQueue.pop();
					
			instance->lock_restoredFibersQueue.Release();

			assert(restoredFiber);
			SwitchToFiber(restoredFiber);
			continue;
		}

		instance->lock_taskQueues.Acquire();

		// Find a task queue to pull from
		std::queue<FiberEntryParams>* taskQueue = nullptr;
		if(instance->taskQueues[TaskPriority::LOW].size() > 0)
			taskQueue = &instance->taskQueues[TaskPriority::LOW];
		if(instance->taskQueues[TaskPriority::MEDIUM].size() > 0)
			taskQueue = &instance->taskQueues[TaskPriority::MEDIUM];
		if(instance->taskQueues[TaskPriority::HIGH].size() > 0)
			taskQueue = &instance->taskQueues[TaskPriority::HIGH];

		assert(instance->taskQueues[TaskPriority::LOW].empty() || instance->taskQueues[TaskPriority::LOW].front().func);

		instance->lock_taskQueues.Release();

		// If there is a new task available
		if(taskQueue)
		{
			instance->lock_taskQueues.Acquire();

			// After lock is released, make sure queue hasn't been emptied by another thread
			if(taskQueue->size() == 0)
			{
				instance->lock_taskQueues.Release();
				continue;
			}

			assert(taskQueue->front().func);
			FiberEntryParams e = taskQueue->front();
			FiberEntryParams* task = new FiberEntryParams(e);
			taskQueue->pop();

			assert(task->func);

			LPVOID taskFiber = CreateFiber(NULL, ExecuteFiber, task);
			instance->lock_taskQueues.Release();

			assert(taskFiber);
			SwitchToFiber(taskFiber);
			//RunTask(task);
			continue;
		}
	}
}

void Scheduler::QueueTask(std::function<void()> task, TaskPriority priority, Counter* taskCounter)
{
	assert(instance);

	// Increment the associated counter if applicable
	if(taskCounter)
		taskCounter->Increment();

	FiberEntryParams entryParams = {};
	entryParams.func = task;
	entryParams.taskCounter = taskCounter;

	assert(entryParams.func);

	// Add the task to the appropriate queue
	instance->lock_taskQueues.Acquire();
	instance->taskQueues[priority].push(entryParams);
	instance->lock_taskQueues.Release();
}

Counter* Scheduler::CreateCounter(int startValue)
{
	assert(instance);

	Counter* counter = new Counter(startValue);
	instance->lock_counters.Acquire();
	instance->counters.push_back(counter);
	instance->lock_counters.Release();
	return counter;
}
void Scheduler::WaitForCounter(Counter* counter)
{
	// If counter is already zero, don't wait
	if(counter->GetCount() == 0)
		return;

	assert(instance);

	// Put current fiber on the wait list
	void* taskFiber = GetCurrentFiber();
	instance->lock_fiberWaitList.Acquire();
	if(instance->fiberWaitList.find(counter) != instance->fiberWaitList.end())
	{
		instance->fiberWaitList[counter].push(taskFiber);
	}
	else
	{
		std::queue<void*> fiberWaitQueue;
		fiberWaitQueue.push(taskFiber);

		instance->fiberWaitList.emplace(counter, fiberWaitQueue);
	}
	instance->lock_fiberWaitList.Release();

	// Switch back to local fiber
	SwitchToFiber(localFiber);
}

void Scheduler::InitializeFiberPool()
{
	lock_fiberPool.Acquire();
	fiberPool = new TaskFiber[FIBER_POOL_SIZE];
	for(size_t i = 0; i < FIBER_POOL_SIZE; i++)
	{
		LPVOID newFiber = CreateFiber(NULL, FiberPoolEntryPoint, new size_t(i));

		TaskFiber taskFiber{};
		taskFiber.fiber = newFiber;
		taskFiber.entryParams = nullptr;
		taskFiber.isBusy = false;
		fiberPool[i] = taskFiber;
	}
	lock_fiberPool.Release();
}
void Scheduler::DestroyFiberPool()
{
	instance->lock_fiberPool.Acquire();
	for(size_t i = 0; i < FIBER_POOL_SIZE; i++)
	{
		DeleteFiber(instance->fiberPool[i].fiber);
		//delete &instance->fiberPool[i];
	}
	instance->fiberPool = nullptr;
	instance->lock_fiberPool.Release();
}

void Scheduler::RunTask(FiberEntryParams* entryParams)
{
	TaskFiber* taskFiber = GetFirstAvailableFiberInPool();
	if(taskFiber)
	{
		taskFiber->entryParams = entryParams;
		SwitchToFiber(taskFiber->fiber);

		instance->lock_fiberPool.Acquire();
		taskFiber->isBusy = false;
		instance->lock_fiberPool.Release();
	}
	else
	{
		// Fallback: create and destroy a temporary fiber if pool is exhausted
		LPVOID tempFiber = CreateFiber(NULL, ExecuteFiber, entryParams);
		SwitchToFiber(tempFiber);
		DeleteFiber(tempFiber);
	}
}

void Scheduler::FiberPoolEntryPoint(void* poolIndex)
{
	assert(instance);

	// These values should be constant throughout the fiberpool's lifetime
	const size_t index = *((size_t*) poolIndex);
	TaskFiber* poolFiber = &instance->fiberPool[index];

	// Don't need the index pointer anymore
	delete poolIndex;

	// Per-task logic that can be reused for any tasks run on this pool fiber
	while(true)
	{
		poolFiber->isBusy = true;

		// Param values should be set before switching to this fiber so we can read the new values here

		std::function<void()> func = poolFiber->entryParams->func;
		Counter* taskCounter = poolFiber->entryParams->taskCounter;

		// The entry params are now old
		delete poolFiber->entryParams;
		poolFiber->entryParams = nullptr;

		assert(func);
		func();

		// After execution of the task completes, decrement the associated task counter if applicable
		if(taskCounter)
			taskCounter->Decrement();

		// Task is complete - switch back to worker fiber
		// Task fiber is marked as reusable after switching back
		SwitchToFiber(localFiber);
	}
}
void Scheduler::ExecuteFiber(void* fiberEntryParams)
{
	FiberEntryParams* entryParams = (FiberEntryParams*) fiberEntryParams;

	// Save the param values locally to avoid pointer misdirection during fiber execution

	std::function<void()> func = entryParams->func;
	Counter* taskCounter = entryParams->taskCounter;

	delete entryParams;

	assert(func);
	func();

	// After execution of the task completes, decrement the associated task counter if applicable
	if(taskCounter)
		taskCounter->Decrement();

	// Task is complete - switch back to worker fiber
	SwitchToFiber(localFiber);
}

void Scheduler::RestoreFibersFromWaitList(Counter* counter)
{
	assert(instance);

	instance->lock_restoredFibersQueue.Acquire();

	instance->lock_fiberWaitList.Acquire();
	std::queue<void*>& fiberWaitQueue = instance->fiberWaitList[counter];
	instance->lock_fiberWaitList.Release();

	for(int i = 0; i < fiberWaitQueue.size(); i++)
	{
		auto waitingFiber = fiberWaitQueue.front();
		assert(waitingFiber);
		instance->restoredFibersQueue.push(waitingFiber);
		fiberWaitQueue.pop();
	}
	instance->lock_restoredFibersQueue.Release();
}

TaskFiber* Scheduler::GetFirstAvailableFiberInPool()
{
	instance->lock_fiberPool.Acquire();
    for(size_t i = 0; i < FIBER_POOL_SIZE; i++)
    {
		TaskFiber* fiber = &instance->fiberPool[i];
        if(!fiber->isBusy)
        {
			instance->lock_fiberPool.Release();
            
			return fiber;
        }
    }
	instance->lock_fiberPool.Release();

    // Expand the pool if needed?
    return nullptr;
}