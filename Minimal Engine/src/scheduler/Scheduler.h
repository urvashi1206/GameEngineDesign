#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <map>

#include "Counter.h"
#include "SpinLock.h"

#define QUEUE_TASK(functionBody) \
    QueueTask([&]()                \
    {                            \
        functionBody             \
    });                          \

enum class TaskPriority
{
    LOW,
    MEDIUM,
    HIGH
};

struct FiberEntryParams
{
    std::function<void()> func;
    Counter* taskCounter;
};

class Scheduler
{
private:
    std::vector<std::thread> threads;
    void* primaryFiber;

    std::map<TaskPriority, std::queue<FiberEntryParams>> taskQueues;
    SpinLock lock_taskQueues;
    std::queue<FiberEntryParams> taskQueue;
    SpinLock lock_taskQueue;
    std::condition_variable cv_taskAvailable;

	// All counters created and used by any task
    std::vector<Counter*> counters;

    std::queue<void*> restoredFibersQueue;
    SpinLock lock_restoredFibersQueue;

    std::map<Counter*, std::queue<void*>> fiberWaitList;

	bool shouldTerminate = false;

public:
    Scheduler();
    ~Scheduler();

    void Startup();
    static void Shutdown();

    void Run();
    static void ExecuteWorkerThread();

    static void QueueTask(std::function<void()> task, TaskPriority priority = TaskPriority::LOW, Counter* taskCounter = nullptr);

	static Counter* CreateCounter(int startValue = 0);
    static void WaitForCounter(Counter* counter);

private:

    static void ExecuteFiber(void* fiberEntryParams);

    static void RestoreFibersFromWaitList(Counter* counter);

    // Counter increment and decrement access RestoreFibersFromWaitList() to notify when counter has reached desired value
    friend void Counter::Increment();
    friend void Counter::Decrement();
};