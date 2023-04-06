#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

class ThreadPool
{
private:
    bool ending; //no more work to do
    std::mutex queuemutex;
    std::condition_variable workqueuecondvar;
    std::vector<std::thread> threads;
    std::queue<std::function<void()> > workqueue;

    void WorkLoop();
public:
    ThreadPool(int t);
    ~ThreadPool();
    void AddWork(const std::function<void()>& work);
    bool busy();
};