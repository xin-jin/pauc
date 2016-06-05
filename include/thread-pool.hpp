/**
 * File: thread-pool.h
 * -------------------
 * This class defines the ThreadPool class, which accepts a collection
 * of thunks (which are zero-argument functions that don't return a value)
 * and schedules them in a FIFO manner to be executed by a constant number
 * of child threads that exist solely to invoke previously scheduled thunks.
 */

#pragma once

#include <cstddef>     // for size_t
#include <functional>  // for the function template used in the schedule signature
#include <thread>      // for thread
#include <vector>      // for vector
#include <queue>
#include <stack>
#include <condition_variable>



class ThreadPool {
public:
    struct JobInfo {
        std::function<void(void)> job;
        bool done = true;
    };

    /**
     * Constructs a ThreadPool configured to spawn up to the specified
     * number of threads.
     */
    ThreadPool(size_t numThreads);

    /**
     * Schedules the provided thunk (which is something that can
     * be invoked as a zero-argument function without a return value)
     * to be executed by one of the ThreadPool's threads as soon as
     * all previously scheduled thunks have been handled.
     */
    void schedule(const std::function<void(void)>& thunk, size_t worker_id);

    void wait(size_t id);

    /**
     * Blocks and waits until all previously scheduled thunks
     * have been executed in full.
     */
    void wait_all();

    /** Return the size of the pool */
    int size() {
        return kNumThreads;
    }

    /**
     * Waits for all previously scheduled thunks to execute, then waits
     * for all threads to be be destroyed, and then otherwise brings
     * down all resources associated with the ThreadPool.
     */
    ~ThreadPool();

private:
    const int kNumThreads;
    std::vector<JobInfo> jobs;   // vector of jobs
    bool running;
    std::vector<std::thread> workers;
    std::mutex mt;              // mutex jobs, nAvailable, and running
    std::condition_variable cv_wait;
    std::vector<std::condition_variable> cvv;

    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
    void worker(size_t ID);
};
