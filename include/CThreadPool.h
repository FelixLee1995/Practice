//
// Created by felix on 6/26/19.
//
#pragma once

#ifndef PRACTICE_CTHREADPOOL_H
#define PRACTICE_CTHREADPOOL_H

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

namespace comm {

#define MAX_THREAD_NUM 256

    //线程池，可以提交变参函数或者lambda表达式的匿名函数执行，可以获取执行返回值
    //不支持成员变量函数，支持类静态成员函数和全局函数， operate()函数等

    class CThreadPool {
        using Task = std::function<void()>;

        //线程池
        std::vector<std::thread> pool;

        //任务队列
        std::queue<Task> tasks;

        //同步
        std::mutex m_lock;

        //条件阻塞
        std::condition_variable cv_task;


        //是否关闭提交
        std::atomic<bool> is_stopped;

        //空闲线程数量
        std::atomic<int> idlThrNum;


    public:
        inline CThreadPool(unsigned short size = 4) : is_stopped(false) {
            idlThrNum = size < 1 ? 1 : size;
            for (size = 0; size < idlThrNum; ++size) {
                pool.emplace_back(
                        [this] {
                            //工作线程函数
                            while (!this->is_stopped) {
                                std::function<void()> task;
                                {
                                    //获取一个待执行的task
                                    std::unique_lock<std::mutex> lock(this->m_lock);
                                    this->cv_task.wait(
                                            lock, [this] {
                                                return this->is_stopped.load() || !this->tasks.empty();
                                            }
                                    );
                                    // wait 直到有 task
                                    if (this->is_stopped && this->tasks.empty())
                                        return;

                                    task = std::move(this->tasks.front()); // 取一个 task
                                    tasks.pop();
                                }
                                idlThrNum--;
                                task();
                                idlThrNum++;
                            }

                        }
                );
            }

        }


        inline ~CThreadPool(){
            is_stopped = true;
            cv_task.notify_all(); // 唤醒所有线程执行
            for (std::thread &t : pool) {
                if (t.joinable()) {
                    t.join(); //等待任务结束， 前提： 线程可执行完
                }
            }


        }


    public:
        //提交一个任务
        //调用.get()获取返回值会等待任务执行完，获取返回值
        //一种是使用 bind:  .commit(std::bind(&dog::sayHello, &dog));
        //另一种是使用 mem_fn: .commit(std::mem_fn(&dog::sayHello), &dog)

        template<class F, class... Args>
        auto commit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
            if (this->is_stopped) {
                throw std::runtime_error("commit on ThreadPool is stopped");
            }
            using RetType  = decltype(f(args...));
            auto task = std::make_shared<std::packaged_task<RetType()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );


            std::future<RetType> future = task->get_future();
            {
                //将任务添加到任务队列
                std::lock_guard<std::mutex> guard{this->m_lock};
                tasks.emplace(
                        [task]() {
                            (*task)();
                        }
                );
            }
            cv_task.notify_one();

            return future;

        }


        //空闲线程数量
        int idlCount() { return idlThrNum; }

    };
}


#endif //PRACTICE_CTHREADPOOL_H
