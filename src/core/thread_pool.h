#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool
{
public:
    ThreadPool(unsigned int thread_count = std::thread::hardware_concurrency())
    {
        if (thread_count == 0) thread_count = 1;

        for (unsigned int i = 0; i < thread_count; ++i)
        {
            _workers.emplace_back([this] { workerLoop(); });
        }
    }

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _stop = true;
        }

        _cv.notify_all();

        for (auto& t : _workers)
            t.join();
    }

    template<typename F>
    auto enqueue(F&& f) -> std::future<std::invoke_result_t<F>>
    {
        using ReturnType = std::invoke_result_t<F>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> guard(_mutex);
            _tasks.push([task](){ (*task)(); });
        }

        _cv.notify_one();

        return result;
    }

private:

    void workerLoop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock, [this]() { return _stop || !_tasks.empty(); });
                if (_stop && _tasks.empty()) return;

                task = std::move(_tasks.front());
                _tasks.pop();
            }

            task();
        }
    }

    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _tasks;
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _stop = false;
};