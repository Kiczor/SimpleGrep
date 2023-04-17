#include "ThreadPool.h"

ThreadPool::ThreadPool(int t)
{
    if( t <= 0 )
        t = 1;
    ending = false;
    for(int i = 0; i < t; i++)
        threads.push_back( std::thread(&ThreadPool::WorkLoop, this) );
}

ThreadPool::~ThreadPool()
{
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

std::map<std::thread::id, std::vector<std::string> > ThreadPool::GetLogs()
{
    for( auto& thread : threads )
    {
        if( filelogs.find(thread.get_id()) == filelogs.end() )
            filelogs[thread.get_id()].push_back("");
    }
    return filelogs;
}

void ThreadPool::AddWork(const std::function<void()>& work, std::string filename)
{
    std::unique_lock<std::mutex> lock(queuemutex);

    workqueue.push( make_pair(work, filename) );

    workqueuecondvar.notify_one();
}

void ThreadPool::WorkLoop()
{
    while( true )
    {
        std::function<void()> work;
        {
            std::unique_lock<std::mutex> lock(queuemutex);
            workqueuecondvar.wait( lock, [&] { return !workqueue.empty() || ending; });

            if( ending ) break;

            work = workqueue.front().first;
            std::string workfilename = workqueue.front().second;
            workqueue.pop();

            filelogs[ std::this_thread::get_id() ].push_back(workfilename);
        }

        work();
    }
}

bool ThreadPool::Busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queuemutex);
        poolbusy = !workqueue.empty();
    }
    return poolbusy;
}
