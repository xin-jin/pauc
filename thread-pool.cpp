/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include <iostream>
#include "thread-pool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : kNumThreads(numThreads), nAvailable(numThreads), running(true) {
    while (numThreads--) {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

void ThreadPool::schedule(const JobType& thunk) {
    std::lock_guard<std::mutex> lck(mt);
    jq.push(thunk);
    cv.notify_one();
}

void ThreadPool::worker() {
    std::unique_lock<std::mutex> lck(mt);

    while (true) {
        while (running && jq.empty())
            cv.wait(lck);

        if (!running) break;

        // acquire the job and execute it
        auto job = jq.front();
        jq.pop();
        --nAvailable;
        lck.unlock();
        job();

        lck.lock();
        ++nAvailable;
        lck.unlock();

        // notify wait() that nAvailable has changed
        cv_wait.notify_one();

        lck.lock();
    }
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lck(mt);
    while (!jq.empty() || nAvailable != kNumThreads) {
        cv_wait.wait(lck);
    }
}

ThreadPool::~ThreadPool() {
    std::unique_lock<std::mutex> lck(mt);
    running = false;
    lck.unlock();

    wait();

    cv.notify_all();
    for (auto& w : workers) {
        w.join();
    }
}
