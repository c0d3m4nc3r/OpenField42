#pragma once

#include <queue>
#include <mutex>
#include <optional>

template<typename T>
class ThreadSafeQueue
{
public:
    
    void push(T item)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(item));
    }

    std::optional<T> pop()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return std::nullopt;
        }
        T item = std::move(_queue.front());
        _queue.pop();
        return item;
    }

private:
    std::queue<T> _queue;
    mutable std::mutex _mutex;
};
