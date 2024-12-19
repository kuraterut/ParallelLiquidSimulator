#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <set>

using namespace std;

struct Task{
    int task_id;
    function<void()> task_func;
};

class ThreadPool {
    private:
        queue<Task>         tasks_queue;
        vector<thread>      threads;
        set<int>            done_task_ids_set;
        
        mutex               done_task_ids_mtx;
        mutex               tasks_queue_mtx;

        atomic<int>         next_task_id;
        atomic<bool>        quit_flag;
        atomic<size_t>      free_threads;

        condition_variable  task_done;
        condition_variable  task_added;

    public:
        ThreadPool(int threads_num = 1);
        ~ThreadPool(){clean_threadPool();}

        void clean_threadPool();

        template<typename F, typename... Args>
        int add_task(const F& f, Args&&... args);

        void quit();
        void wait(int id);
        void wait_all();

        void execute(int thread_num);
};

template<typename F, typename... Args>
int ThreadPool::add_task(const F& f, Args&&... args) {
    lock_guard<mutex> tasks_queue_lock(tasks_queue_mtx);
    Task task;
    
    task.task_id    = next_task_id;
    task.task_func  = bind(f, args...);
    next_task_id++;
    
    tasks_queue.push(task);
    task_added.notify_one();
    return next_task_id-1;
}


void ThreadPool::execute(int thread_num) {
    while (!quit_flag) {
        unique_lock<mutex> tasks_queue_lock(tasks_queue_mtx);
        task_added.wait(tasks_queue_lock, [this]{
            return !tasks_queue.empty() || quit_flag;
        });

        free_threads--;

        if (quit_flag) break;

        auto task = move(tasks_queue.front());
        tasks_queue.pop();
        tasks_queue_lock.unlock();

        task.task_func();

        lock_guard<mutex> done_task_ids_lock(done_task_ids_mtx);
        done_task_ids_set.insert(task.task_id);

        free_threads++;

        task_done.notify_one();
    }
}


ThreadPool::ThreadPool(int threads_num) {
    quit_flag = false;
    next_task_id = 0;
    threads.reserve(threads_num);
    for (int i = 0; i < threads_num; ++i) threads.emplace_back(&ThreadPool::execute, this, i);
    
    free_threads = threads_num;
}



void ThreadPool::clean_threadPool(){
    quit();
    for (auto& thread : threads) {
        thread.join();
    }
}

void ThreadPool::quit() {
    quit_flag = true;
    task_added.notify_all();
}

void ThreadPool::wait(int id) {
    unique_lock<mutex> done_task_ids_lock(done_task_ids_mtx);
    if (done_task_ids_set.contains(id)) {
        return;
    }
    task_done.wait(done_task_ids_lock, [this, id]{
        return done_task_ids_set.contains(id);
    });
}

void ThreadPool::wait_all() {
    unique_lock<mutex> done_task_ids_lock(done_task_ids_mtx);
    
    unique_lock<mutex> tasks_queue_lock(tasks_queue_mtx);
    if (done_task_ids_set.size() == next_task_id) {
        return;
    }
    tasks_queue_lock.unlock();

    task_done.wait(done_task_ids_lock, [this]{
        lock_guard<mutex> tasks_queue_lock(tasks_queue_mtx);
        return done_task_ids_set.size() == next_task_id;
    });
}

