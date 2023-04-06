#include "ThreadPool.h"

//temp
#include <iostream>

ThreadPool::ThreadPool(int t)
{
    std::cout << "calling constructor t:" << t << "\n";
    ending = false;
    for(int i = 0; i < t; i++)
        threads.push_back( std::thread(&ThreadPool::WorkLoop, this) );
    std::cout << threads.size() << "\n";
}

ThreadPool::~ThreadPool()
{
    std::cout << "calling destructor\n";

    {
        std::unique_lock<std::mutex> lock(queuemutex);
        ending = true;
    }

    //wake up all threads
    workqueuecondvar.notify_all();
    for( auto& thread : threads )
    {
        if( thread.joinable() )
            thread.join();
    }
}

void ThreadPool::AddWork(const std::function<void()>& work)
{
    std::unique_lock<std::mutex> lock(queuemutex);

    std::cout << "adding work\n";

    workqueue.push(work);

    workqueuecondvar.notify_one();
}

void ThreadPool::WorkLoop()
{
    while( true )
    {
        std::cout << "working in work loop\n";
        std::function<void()> work;
        {
            std::unique_lock<std::mutex> lock(queuemutex);
            workqueuecondvar.wait( lock, [&] {
                return !workqueue.empty() || ending;
            });

            std::cout << "ending: " << ending << ", queue size: " << workqueue.size() << "\n";

            if( ending ) break;

            std::cout << "got work to do\n";

            work = workqueue.front();
            workqueue.pop();
        }

        work();
    }
}

bool ThreadPool::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queuemutex);
        poolbusy = !workqueue.empty();
    }
    return poolbusy;
}
