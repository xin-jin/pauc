/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include <iostream>
#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : kNumThreads(numThreads), wts(numThreads),
                                            worker_infos(numThreads), running(true) {
    for (size_t i = numThreads - 1; i > 0; --i) {
        available_workers.push(i);
    }
    available_workers.push(0);
    d_info.th = thread(&ThreadPool::dispatcher, this);
}

void ThreadPool::dispatcher() {
    unique_lock<mutex> lck(mt);

    while (true) {
        while (jq.empty() && running)
            d_info.cv_new_job.wait(lck);
        if (!running) break;

        while (!jq.empty()) {
            if (available_workers.empty())
                d_info.cv_available_worker.wait(lck);
            size_t workerID = available_workers.top();
            available_workers.pop();
            WorkerInfo &w = worker_infos[workerID];

            if (!w.alive) {
                w.th = thread([this](size_t workerID) { worker(workerID); }, workerID);
                w.alive = true;
            }
            w.job = jq.front();
            w.working = true;
            jq.pop();
            // wake up the worker
            w.cv.notify_one();
        }

        cv_destructor.notify_one();
    }
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    lock_guard<mutex> lck(mt);
    jq.push(thunk);
    d_info.cv_new_job.notify_one();
}

void ThreadPool::wait() {
    unique_lock<mutex> lck{mt};
    while (!jq.empty()) {
        cv_destructor.wait(lck);
    }
    running = false;
    lck.unlock();

    d_info.cv_new_job.notify_one();
    d_info.th.join();

    for (auto& w : worker_infos) {
        if (w.alive) {
            w.cv.notify_one();
            w.th.join();
        }
    }
}

void ThreadPool::worker(size_t workerID) {
    WorkerInfo &w = worker_infos[workerID];
    unique_lock<mutex> lck(mt);

    while (true) {
        while (!w.working && running)
            w.cv.wait(lck);

        if (!w.working) break;

        // execute job
        lck.unlock();
        w.job();

        lck.lock();
        w.working = false;
        available_workers.push(workerID);
        d_info.cv_available_worker.notify_one();
    }
}

ThreadPool::~ThreadPool() {}
