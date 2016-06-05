/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include <iostream>
#include "thread-pool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : kNumThreads(numThreads), jobs(numThreads),
                                            running(true), cvv(numThreads) {
    for (size_t i = 0; i != numThreads; ++i)
        workers.emplace_back(&ThreadPool::worker, this, i);
}

void ThreadPool::schedule(const std::function<void(void)>& thunk, size_t worker_id) {
    std::lock_guard<std::mutex> lck(mt);
    jobs[worker_id].job = thunk;
    jobs[worker_id].done = false;
    cvv[worker_id].notify_one();
}

void ThreadPool::worker(size_t id) {
    std::unique_lock<std::mutex> lck(mt);

    while (true) {
        while (running && jobs[id].done)
            cvv[id].wait(lck);

        if (!running) break;
        lck.unlock();
        jobs[id].job();

        lck.lock();
        jobs[id].done = true;

        // notify wait()
        cv_wait.notify_all();
    }
}

void ThreadPool::wait(size_t id) {
    std::unique_lock<std::mutex> lck(mt);
    while (!jobs[id].done) {
        cv_wait.wait(lck);
    }
}

void ThreadPool::wait_all() {
    for (size_t id = 0; id != kNumThreads; ++id) {
        wait(id);
    }
}


ThreadPool::~ThreadPool() {
    std::unique_lock<std::mutex> lck(mt);
    running = false;
    lck.unlock();

    wait_all();

    for (auto& cv : cvv)
        cv.notify_all();

    for (auto& w : workers) {
        w.join();
    }
}
