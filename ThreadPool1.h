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

class ThreadPool1 {
public:
    ThreadPool1(size_t = std::thread::hardware_concurrency());

    template <class F>
    void submit(F&& task);

    ~ThreadPool1();

    ThreadPool1(ThreadPool1&&) = default;
    ThreadPool1(ThreadPool1&) = delete;

private:
    // Initialize thread
    void waitForTask();

    // used to keep the working threads
    std::vector<std::thread> workers;

    // tasks queue
    std::queue<std::function<void()>> tasks;

    // mutex for tasks queue
    std::mutex queueMutex;

    // synchronization
    std::condition_variable condition;

    // the state of ThreadPool
    bool isShutdown;
};

ThreadPool1::ThreadPool1(size_t threads) : isShutdown(false)
{
    for(int i = 0; i < threads; i++) {
        std::cout << "From Thread--" << std::this_thread::get_id() << " saying: I am constructing" << std::endl;
        std::thread workThread(std::bind(&ThreadPool1::waitForTask, this));
        workers.push_back(std::move(workThread));
    }
}

ThreadPool1::~ThreadPool1() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        isShutdown = true;
    }
    this->condition.notify_all();
    std::cout << "Waiting for all tasks to finish" << std::endl;
    for(std::thread &worker : this->workers) {
        worker.join();
    }
    std::cout << "I am destructed" << std::endl;
}

void ThreadPool1::waitForTask() {
    std::unique_lock<std::mutex> uniqueLock(this->queueMutex);
    // keep it waiting for task
    while(true) {
        if(!this->tasks.empty()) {
            auto task = std::move(this->tasks.front());
            this->tasks.pop();
            uniqueLock.unlock();
            task();
            uniqueLock.lock();
        } else if(this->isShutdown){
            break;
        } else {
            this->condition.wait(uniqueLock);
        }
    }
}

template <class F>
void ThreadPool1::submit(F&& task) {
    {
        std::lock_guard<std::mutex> lockGuard(this->queueMutex);
        this->tasks.emplace(std::forward<F>(task));
    }
    this->condition.notify_one();
}

#endif //TP_THREADPOOL1_H
