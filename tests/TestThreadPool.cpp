#include "gtest/gtest.h"
#include "../libportquery/source/ThreadPool.h"


using namespace PortQuery;


TEST(ThreadPool, ConstructThreadPool) {

    ThreadPool tp = ThreadPool(4);
}


TEST(ThreadPool, SubmitWorkNoArgumentsNoReturn) {

    auto tp_T1 = ThreadPool(0);
    auto lambda_T1 = [] () { int loop = 1; for (; loop < 10; loop++); };
    for (int i = 0; i < 10; i++) {
        tp_T1.submitWork(lambda_T1);
    }

    auto tp_T2 = ThreadPool(1);
    for (int i = 0; i < 10; i++) {
        tp_T2.submitWork(lambda_T1);
    }
}


TEST(ThreadPool, SubmitWorkWithArgumentsNoReturn) {

    auto tp_T1 = ThreadPool(0);
    auto lambda_T1 = [] (int x) { for(int i = 0; i < x; i++); };
    for (int i = 0; i < 10; i++) {
        tp_T1.submitWork(lambda_T1, i);
    }

    auto tp_T2 = ThreadPool(1);
    for (int i = 0; i < 10; i++) {
        tp_T2.submitWork(lambda_T1, i);
    }
}


TEST(ThreadPool, SubmitWorkNoArgumentsAndReturn) {

    auto tp_T1 = ThreadPool(1);
    auto lambda_T1 =  [] () { return std::this_thread::get_id(); };
    auto sleepyLambda_T1 =  [] () { std::this_thread::sleep_for(std::chrono::seconds(1)); return std::this_thread::get_id(); };

    auto future_T1 = tp_T1.submitWork(sleepyLambda_T1);
    auto sleepyFuture_T1 = tp_T1.submitWork(lambda_T1);

    auto quick_T1 = future_T1.get();
    auto slow_T1 = sleepyFuture_T1.get();

    ASSERT_TRUE(quick_T1 == slow_T1);

    auto tp_T2 = ThreadPool(2);

    auto future_T2 = tp_T2.submitWork(sleepyLambda_T1);
    auto sleepyFuture_T2 = tp_T2.submitWork(lambda_T1);

    auto quick_T2 = future_T2.get();
    auto slow_T2 = sleepyFuture_T2.get();

    ASSERT_TRUE(quick_T2 == slow_T2);
}
