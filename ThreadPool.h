#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <map>
#include <condition_variable>
#include <functional>

class ThreadPool
{
private:
    bool ending; //no more work to do
    std::mutex queuemutex;
    std::condition_variable workqueuecondvar;
    std::vector<std::thread> threads;
    std::queue<std::pair<std::function<void()>, std::string> > workqueue;
    std::map<std::thread::id, std::vector<std::string> > filelogs;

    void WorkLoop();
public:
    ThreadPool(int t);
    ~ThreadPool();
    void AddWork(const std::function<void()>& work, std::string filename);
    bool Busy();
    std::map<std::thread::id, std::vector<std::string> > GetLogs();
};