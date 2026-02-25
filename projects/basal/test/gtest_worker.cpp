#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <basal/basal.hpp>
#include <basal/worker.hpp>
#include <basal/gtest_helper.hpp>

using namespace basal::literals;

struct Work {
    int a;
    int b;
};

struct Result {
    int value;
};

class DummyWorker : public basal::Worker<Work, Result> {
public:
    DummyWorker(basal::ExceptionListener& exception_listener) : basal::Worker<Work, Result>{exception_listener} {
    }

protected:
    Result work(const Work& work_unit) override {
        // std::cout << "Working on: " << work_unit.a << " + " << work_unit.b << std::endl;
        if (work_unit.a == 0) {
            throw std::runtime_error("a cannot be zero");
        }
        return Result{work_unit.a + work_unit.b};
    }
};

class BasalWorkerTest
    : public ::testing::Test
    , public basal::ExceptionListener {
public:
    BasalWorkerTest() = default;
    ~BasalWorkerTest() = default;
    BasalWorkerTest(const BasalWorkerTest&) = delete;
    BasalWorkerTest& operator=(const BasalWorkerTest&) = delete;

    void SetUp() override {
        exception_thrown.store(false);
    }

    void on_exception(const std::exception& e) override {
        std::cout << "Exception thrown in worker thread: " << e.what() << std::endl;
        exception_thrown.store(true);
    }
    std::atomic<bool> exception_thrown;
};

TEST_F(BasalWorkerTest, QueueBasics) {
    basal::SafeQueue<int> queue;
    int value;
    ASSERT_TRUE(queue.empty());
    ASSERT_FALSE(queue.try_pop(value));
    value = 42;
    queue.push(value);
    ASSERT_FALSE(queue.empty());
    value = 0;
    ASSERT_TRUE(queue.try_pop(value));
    ASSERT_EQ(value, 42);
}

TEST_F(BasalWorkerTest, WorkerException) {
    DummyWorker worker{*this};
    ASSERT_FALSE(worker.active());
    worker.start();
    ASSERT_TRUE(worker.active());
    worker.submit(Work{0, 0});
    worker.stop();
    ASSERT_TRUE(exception_thrown.load());
    ASSERT_FALSE(worker.active());
}

TEST_F(BasalWorkerTest, WorkerSingleThread) {
    DummyWorker worker{*this};
    ASSERT_FALSE(worker.active());
    worker.start();
    ASSERT_TRUE(worker.active());
    worker.submit(Work{1, 2});
    Result result = worker.collect();
    ASSERT_EQ(result.value, 3);
    worker.stop();
    ASSERT_FALSE(worker.active());
}

TEST_F(BasalWorkerTest, WorkerStopsWithoutWork) {
    DummyWorker worker{*this};
    ASSERT_FALSE(worker.active());
    worker.start();
    ASSERT_TRUE(worker.active());
    worker.stop();
    ASSERT_FALSE(worker.active());
}

TEST_F(BasalWorkerTest, WorkerStartStopStress) {
    DummyWorker worker{*this};
    constexpr int iterations = 500;
    for (int index = 0; index < iterations; ++index) {
        worker.start();
        ASSERT_TRUE(worker.active());
        worker.stop();
        ASSERT_FALSE(worker.active());
    }
}

TEST_F(BasalWorkerTest, WorkerConcurrentSubmitStress) {
    DummyWorker worker{*this};
    worker.start();

    constexpr int producer_count = 4;
    constexpr int items_per_producer = 300;
    constexpr int total_items = producer_count * items_per_producer;
    std::atomic<long long> expected_sum{0};

    std::vector<std::thread> producers;
    producers.reserve(producer_count);
    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&worker, &expected_sum, producer] {
            for (int item = 0; item < items_per_producer; ++item) {
                int a = producer + 1;
                int b = item;
                expected_sum.fetch_add(static_cast<long long>(a + b));
                worker.submit(Work{a, b});
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    long long actual_sum = 0;
    for (int index = 0; index < total_items; ++index) {
        auto result = worker.collect();
        actual_sum += result.value;
    }

    worker.stop();
    ASSERT_FALSE(worker.active());
    ASSERT_EQ(actual_sum, expected_sum.load());
    ASSERT_FALSE(exception_thrown.load());
}

TEST_F(BasalWorkerTest, WorkerRestartBatchesStress) {
    DummyWorker worker{*this};

    constexpr int rounds = 60;
    constexpr int items_per_round = 40;
    for (int round = 0; round < rounds; ++round) {
        worker.start();
        ASSERT_TRUE(worker.active());

        int expected_sum = 0;
        for (int value = 1; value <= items_per_round; ++value) {
            worker.submit(Work{value, round});
            expected_sum += value + round;
        }

        int actual_sum = 0;
        for (int index = 0; index < items_per_round; ++index) {
            actual_sum += worker.collect().value;
        }

        worker.stop();
        ASSERT_FALSE(worker.active());
        ASSERT_EQ(actual_sum, expected_sum);
    }

    ASSERT_FALSE(exception_thrown.load());
}

TEST_F(BasalWorkerTest, WorkerMassiveParallelStress) {
    constexpr int worker_count = 100;
    constexpr int items_per_worker = 300;

    std::vector<std::unique_ptr<DummyWorker>> workers;
    workers.reserve(worker_count);

    // Create all workers
    for (int i = 0; i < worker_count; ++i) {
        workers.emplace_back(std::make_unique<DummyWorker>(*this));
    }

    std::atomic<int> completed{0};
    std::atomic<int> errors{0};

    // Start all workers and submit items in parallel
    std::vector<std::thread> worker_threads;
    worker_threads.reserve(worker_count);

    for (int worker_idx = 0; worker_idx < worker_count; ++worker_idx) {
        worker_threads.emplace_back([this, &workers, &completed, &errors, worker_idx] {
            try {
                auto& worker = *workers[worker_idx];
                worker.start();

                long long expected_sum = 0;
                for (int item = 0; item < items_per_worker; ++item) {
                    int value = worker_idx + item;
                    expected_sum += value + value + 2;
                    worker.submit(Work{value + 1, value + 1});
                }

                long long actual_sum = 0;
                for (int item = 0; item < items_per_worker; ++item) {
                    actual_sum += worker.collect().value;
                }

                worker.stop();

                if (actual_sum != expected_sum) {
                    errors.fetch_add(1);
                } else {
                    completed.fetch_add(1);
                }
            } catch (const std::exception& e) {
                errors.fetch_add(1);
            }
        });
    }

    // Wait for all workers to complete
    for (auto& thread : worker_threads) {
        thread.join();
    }

    ASSERT_EQ(completed.load(), worker_count) << "Not all workers completed successfully";
    ASSERT_EQ(errors.load(), 0) << "Some worker threads had errors";
    ASSERT_FALSE(exception_thrown.load());
}
