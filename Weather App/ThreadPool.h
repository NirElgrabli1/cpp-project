/**
 * @file ThreadPool.h
 * @brief Thread pool implementation for parallel task execution
 */
#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

 /**
  * @class ThreadPool
  * @brief Thread pool for executing tasks in parallel
  */
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    /**
     * @brief Constructor
     * @param numThreads Number of worker threads
     */
    ThreadPool(size_t numThreads);

    /**
     * @brief Destructor
     */
    ~ThreadPool();

    /**
     * @brief Enqueue a task for execution
     * @param f The task to execute
     * @return Future for the task result
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;
};

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop.load()) {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return result;
}