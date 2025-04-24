#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <map>

#include "Counter.h"
#include "SpinLock.h"

#include "Windows.h"

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
struct TaskFiber
{
    LPVOID fiber;
    FiberEntryParams* entryParams;
    bool isBusy;
};

class Scheduler
{
private:
    static const size_t FIBER_POOL_SIZE = 100;

private:
    std::vector<std::thread> threads;
    void* primaryFiber;

    TaskFiber* fiberPool;
	SpinLock lock_fiberPool;

    std::map<TaskPriority, std::queue<FiberEntryParams>> taskQueues;
    SpinLock lock_taskQueues;

	// All counters created and used by any task
    std::vector<Counter*> counters;
	SpinLock lock_counters;

    std::queue<void*> restoredFibersQueue;
    SpinLock lock_restoredFibersQueue;

    std::map<Counter*, std::queue<void*>> fiberWaitList;
    SpinLock lock_fiberWaitList;

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

    void InitializeFiberPool();
    static void DestroyFiberPool();

    static void RunTask(FiberEntryParams* entryParams);

    static void FiberPoolEntryPoint(void* poolIndex);
    static void ExecuteFiber(void* fiberEntryParams);

    static void RestoreFibersFromWaitList(Counter* counter);

    static TaskFiber* GetFirstAvailableFiberInPool();

    // Counter increment and decrement access RestoreFibersFromWaitList() to notify when counter has reached desired value
    friend void Counter::Increment();
    friend void Counter::Decrement();
};