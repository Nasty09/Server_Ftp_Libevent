#include "Server.hpp"

void ThreadPool::Init(int num) {
    this->threadCount = num;
    this->lastThread = -1;
    for (int i = 0; i < num; i++)
    {
        auto t = new Thread();
        std::cout << "Create thread " << i + 1 << "\n";
        t->id = i + 1;
        t->Start();
        threads.emplace_back(t);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void ThreadPool::Dispatch(Task *task)
{
    if (!task)
        return;
    int tid = (lastThread + 1) % threadCount;
    lastThread = tid;
    Thread *t = threads[tid];
    t->AddTask(task);
    t->Activate();
}
