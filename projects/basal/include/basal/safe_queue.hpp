#pragma once

/// @file
/// A simple thread-safe queue implementation. This is not intended to be a general-purpose thread-safe queue, but
/// rather a simple implementation that can be used by the Worker class.
/// @note This is a header-only implementation, so it can be included in any translation unit without needing to link
/// against a separate library.

#include <condition_variable>
#include <queue>
#include <mutex>

namespace basal {

/// A typical thread safe queue implementation. This is not intended to be a general-purpose thread-safe queue, but
/// rather a simple implementation that can be used by the Worker class.
template <typename T>
class SafeQueue {
public:
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_) {
            return;
        }
        queue_.push(std::move(value));
        condition_.notify_one();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;
    }

    bool wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return shutdown_ or not queue_.empty(); });
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        condition_.notify_all();
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = false;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool shutdown_{false};
};

}  // namespace basal