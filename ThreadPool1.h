//
// Created by Mengxuan Jia on 2019/1/13.
//

#ifndef TP_THREADPOOL1_H
#define TP_THREADPOOL1_H

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <vector>
#include <iostream>
#include <type_traits>

class ThreadPool1 {
public:
    ThreadPool1(size_t = std::thread::hardware_concurrency());

    // Submit task into threadpool
    template<class Function, class... Args>
    std::future<std::invoke_result_t<Function, Args...>> submit(Function&&, Args&&...);

    // Waiting for all tasks to finish
    void waitAll();

    // Shutdown all threads now and kick off all tasks from task queue.
    void shutDownNow();

    // Initiates an shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
    void shutDown();

    ~ThreadPool1();

    ThreadPool1(ThreadPool1&&) = default;
    ThreadPool1(ThreadPool1&) = delete;

private:

    // Initialize thread
    void initThread();

    // used to keep the working threads
    std::vector<std::thread> workers;

    // tasks queue
    std::queue<std::function<void()>> tasks;

    // mutex for tasks queue
    std::mutex queueMutex;

    // synchronization
    std::condition_variable condition;

    // atomic number use to count the tasks that are not finished yet
    std::atomic_size_t tasksNum;

    // the state of ThreadPool
    bool isShutdown;
};

ThreadPool1::ThreadPool1(size_t threads) : tasksNum(0), isShutdown(false)
{
    for(int i = 0; i < threads; i++) {
        std::cout << "From Thread--" << std::this_thread::get_id() << " saying: I am constructing" << std::endl;
        std::thread workThread(std::bind(&ThreadPool1::initThread, this));
        workers.push_back(std::move(workThread));
    }
}

ThreadPool1::~ThreadPool1() {
    shutDown();
    std::cout << "I am destructed" << std::endl;
}

void ThreadPool1::shutDownNow() {
    {
        std::lock_guard<std::mutex> lockGuard(queueMutex);
        isShutdown = true;

        tasks = {};// Same with: std::queue<std::function<void()>>().swap(tasks);
    }
    this->condition.notify_all();
    for(std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool1::shutDown() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        isShutdown = true;
    }
    this->condition.notify_all();
    for(std::thread& worker : this->workers) {
        worker.join();
    }
}


void ThreadPool1::initThread() {
    std::unique_lock<std::mutex> uniqueLock(this->queueMutex);
    // keep it waiting for task
    while(true) {
        if(!this->tasks.empty()) {
            auto task = std::move(this->tasks.front());
            this->tasks.pop();
            uniqueLock.unlock();
            task();
            tasksNum.fetch_sub(1);
            uniqueLock.lock();
        } else if(this->isShutdown){
            break;
        } else {
            this->condition.wait(uniqueLock);
        }
    }
}

template<class Function, class... Args>
std::future<std::invoke_result_t<Function, Args...>> ThreadPool1::submit(Function&& f, Args&&... args) {
    tasksNum.fetch_add(1);
    using return_type = std::invoke_result_t<Function, Args...>;
    // Build a packaged_task
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Function>(f), std::forward<Args>(args)...)
    );

    // get the future
    std::future<return_type> res = task->get_future();
    {
        // Lock the task queue，unlock the queue when get out of this code chunk
        std::lock_guard<std::mutex> lock(queueMutex);

        // Don't allow enqueueing after stopping the pool
        if(isShutdown){
            throw std::runtime_error("try to submit task to stopped ThreadPool");
        }
        // Push the task into the task queue
        this->tasks.emplace([task](){ (*task)(); });
    }

    // Notify one thread that is waiting for this condition.
    this->condition.notify_one();
    return std::move(res);
}


void ThreadPool1::waitAll() {
    std::cout<< "Waiting all tasks ot finish" << std::endl;
    while(1) {
        if(tasksNum.load() == 0)
            break;
    }
    std::cout<< "All tasks are finished" << std::endl;
}

#endif //TP_THREADPOOL1_H
