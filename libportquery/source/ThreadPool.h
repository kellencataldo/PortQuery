#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>


namespace PortQuery {

    class ThreadSafeWorkQueue {

        public:
            using WorkType = std::function<void(void)>;
            static_assert(std::is_move_constructible<WorkType>::value, "");
            static_assert(std::is_move_assignable<WorkType>::value, "");

            ThreadSafeWorkQueue() : m_done(false) { }

            void blockingPush(const WorkType&& work);
            bool nonBlockingPush(const WorkType&& work);

            bool blockingPop(WorkType& work);
            bool nonBlockingPop(WorkType& work);

            void setDone();
            bool empty() const;

    private:
        bool m_done;
        std::queue<WorkType> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_ready;
    };


    class ThreadPool
    {
        public:
            ThreadPool(const int threadCount);
            ~ThreadPool();

            template<typename Function, typename... Args> 
            std::future<std::invoke_result_t<Function, Args...>> submitWork(Function&& f, Args&&... args);

        private:

            void workerLoop(const unsigned int startQueue);

            static constexpr unsigned int MAX_LOOPS = 1;
            std::vector<ThreadSafeWorkQueue> m_queues;
            std::vector<std::thread> m_threads;
            int m_threadCount;
            std::atomic_uint m_nextQueue;
     
    };

}
