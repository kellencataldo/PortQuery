#include "gtest/gtest.h"
#include "../libportquery/source/ThreadPool.h"


using namespace PortQuery;


TEST(ThreadPool, ConstructThreadPool) {

    ThreadPool tp = ThreadPool(4);
}
