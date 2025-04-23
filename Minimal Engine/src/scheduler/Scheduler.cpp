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
}

void Scheduler::Startup()
{
	instance = this;

	taskQueues.emplace(TaskPriority::LOW, std::queue<FiberEntryParams>());
	taskQueues.emplace(TaskPriority::MEDIUM, std::queue<FiberEntryParams>());
	taskQueues.emplace(TaskPriority::HIGH, std::queue<FiberEntryParams>());
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
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

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

		// Find a task queue to pull from
		std::queue<FiberEntryParams>* taskQueue = nullptr;
		if(instance->taskQueues[TaskPriority::LOW].size() > 0)
			taskQueue = &instance->taskQueues[TaskPriority::LOW];
		if(instance->taskQueues[TaskPriority::MEDIUM].size() > 0)
			taskQueue = &instance->taskQueues[TaskPriority::MEDIUM];
		if(instance->taskQueues[TaskPriority::HIGH].size() > 0)
			taskQueue = &instance->taskQueues[TaskPriority::HIGH];

		// If there is a new task available
		if(taskQueue)
		{
			instance->lock_taskQueue.Acquire();

			// After lock is released, make sure queue hasn't been emptied by another thread
			if(taskQueue->size() == 0)
			{
				instance->lock_taskQueue.Release();
				continue;
			}

			FiberEntryParams* task = new FiberEntryParams(taskQueue->front());
			taskQueue->pop();

			instance->lock_taskQueue.Release();

			LPVOID taskFiber = CreateFiber(NULL, ExecuteFiber, task);

			assert(taskFiber);
			SwitchToFiber(taskFiber);
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

	instance->lock_taskQueues.Acquire();
	instance->taskQueues[priority].push(entryParams);
	instance->lock_taskQueues.Release();

	//cv_taskAvailable.notify_one(); // Wake up one thread
}

Counter* Scheduler::CreateCounter(int startValue)
{
	Counter* counter = new Counter(startValue);
	instance->counters.push_back(counter);
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

	// Switch back to local fiber
	SwitchToFiber(localFiber);
}

void Scheduler::ExecuteFiber(void* fiberEntryParams)
{
	FiberEntryParams* entryParams = (FiberEntryParams*) fiberEntryParams;

	// Save the param values locally to avoid pointer misdirection during fiber execution

	std::function<void()> func = entryParams->func;
	Counter* taskCounter = entryParams->taskCounter;

	delete entryParams;

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

	std::queue<void*>& fiberWaitQueue = instance->fiberWaitList[counter];

	for(int i = 0; i < fiberWaitQueue.size(); i++)
	{
		instance->restoredFibersQueue.push(fiberWaitQueue.front());
		fiberWaitQueue.pop();
	}
}