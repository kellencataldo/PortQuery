#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>


template<typename TaskType> class ThreadSafeTaskQueue {

    public:

        static_assert(std::is_move_constructible<TaskType>::value, "");
        static_assert(std::is_move_assignable<TaskType>::value, "");

        ThreadSafeTaskQueue() : m_done(false) { }

        void blockingPush(const TaskType&& task);
        bool nonBlockingPush(const TaskType&& task);

        bool blockingPop(TaskType& task);
        bool nonBlockingPop(TaskType& task);

        void setDone();
        bool empty() const;

private:
	std::queue<TaskType> m_queue;
	mutable std::mutex m_mutex;
	std::condition_variable m_ready;
	bool m_done;
};


class ThreadPool
{
    public:
        ThreadPool(const int threadCount) : m_nextQueue(0) {

            m_threadCount = threadCount != 0 ? threadCount : std::thread::hardware_concurrency();
		    for (unsigned int startQueue = 0; startQueue < m_threadCount; startQueue++) {
                m_threads.emplace_back([&, startQueue] { workerLoop(startQueue); });
            }
        }

        ~ThreadPool() {

            for(auto& q : m_queues) {
                q.setDone();
            }

            for(auto& t : m_threads) {
                t.join();
            }
        }

        template<typename Function, typename... Args> 
        std::future<std::invoke_result_t<Function, Args...>> submitWork(Function&& f, Args&&... args);

    private:

        void workerLoop(const unsigned int startQueue);

        static constexpr unsigned int MAX_LOOPS = 1;
        std::vector<ThreadSafeTaskQueue<std::function<void(void)>>> m_queues;
        std::vector<std::thread> m_threads;
        int m_threadCount;
        std::atomic_uint m_nextQueue;
};
