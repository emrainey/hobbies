#pragma once

#include <exception>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>

#include <basal/safe_queue.hpp>

namespace basal {

class ExceptionListener {
public:
    virtual void on_exception(const std::exception& e) = 0;

protected:
    virtual ~ExceptionListener() = default;
};

template <typename WORK_UNIT_TYPE, typename RESULT_UNIT_TYPE>
class Worker {
public:
    using WorkUnitType = WORK_UNIT_TYPE;
    using ResultUnitType = RESULT_UNIT_TYPE;

protected:
    ExceptionListener& exception_listener_;
    std::atomic<bool> running_;
    std::atomic<bool> exitted_;
    std::thread thread_;
    std::mutex mutable interface_mutex_;
    std::condition_variable condition_;
    basal::SafeQueue<WorkUnitType> work_queue_;
    basal::SafeQueue<ResultUnitType> result_queue_;

public:
    Worker(ExceptionListener& exception_listener)
        : exception_listener_{exception_listener}
        , running_{false}
        , exitted_{true}
        , thread_{}
        , interface_mutex_{}
        , condition_{}
        , work_queue_{}
        , result_queue_{} {
    }

    virtual ~Worker() {
        stop();
    }

    void start() {
        std::unique_lock lock(interface_mutex_);
        if (not running_.load()) {
            work_queue_.reset();
            result_queue_.reset();
            running_.store(true);
            exitted_.store(true);
            thread_ = std::thread(&Worker::run, this);
            condition_.wait(lock, [this] { return not exitted_.load(); });
        }
    }

    void stop() {
        std::unique_lock<std::mutex> lock(interface_mutex_);
        running_.store(false);
        work_queue_.shutdown();
        result_queue_.shutdown();
        condition_.notify_all();
        lock.unlock();  // release the lock before joining the thread to avoid deadlock!
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void submit(const WorkUnitType& work_unit) {
        work_queue_.push(work_unit);
    }

    void submit(const std::vector<WorkUnitType>& work_units) {
        for (const auto& work_unit : work_units) {
            submit(work_unit);
        }
    }

    ResultUnitType collect() {
        ResultUnitType result_unit;
        if (not result_queue_.wait_and_pop(result_unit)) {
            throw std::runtime_error("Worker is stopping");
        }
        return result_unit;
    }

    bool active() const {
        std::unique_lock<std::mutex> lock(interface_mutex_);
        return running_.load() or not exitted_.load();
    }

protected:
    virtual ResultUnitType work(const WorkUnitType& work_unit) = 0;

    bool receive(WorkUnitType& work_unit) {
        return work_queue_.wait_and_pop(work_unit);
    }

    void send(const ResultUnitType& result_unit) {
        result_queue_.push(result_unit);
    }

    void run() {
        // signals that this thread is started
        exitted_.store(false);
        // this signals start() to return
        condition_.notify_all();
        while (running_.load()) {
            try {
                WorkUnitType data;
                if (not receive(data)) {
                    break;
                }
                auto result = work(data);
                send(result);
            } catch (std::runtime_error& e) {
                exception_listener_.on_exception(e);
                running_.store(false);  // stop the worker if an exception is thrown
            }
        }
        // this signals that the thread is exiting
        exitted_.store(true);
    }
};

}  // namespace basal