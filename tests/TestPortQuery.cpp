#include <string>

#include "gmock/gmock.h"
#include "../libportquery/source/Environment.h"
#include "../libportquery/include/PortQuery.h"

using ::testing::AtLeast;
using ::testing::_;
using ::testing::Lt;
using ::testing::Ge;

using namespace PortQuery;


class MockEnvironment : public IEnvironment {

    public:

        MOCK_METHOD(bool, scanPort, (), (override));
};



struct MockGenerator {

    public:

        static void setGeneratorFunction(void) {

            if (!s_generatorSet) {
                EnvironmentFactory::setGenerator(generatorFunction);
                s_generatorSet = true;
            }
        }

    private:

        static EnvironmentPtr generatorFunction(const int threadCount) {
            return std::make_shared<MockEnvironment>();
        }

        static bool s_generatorSet;
};

bool MockGenerator::s_generatorSet = false;


/*
TEST(RunScan, ExpectedNoPortsSubmitted) {

    MockGenerator::setGeneratorFunction();
    EnvironmentPtr baseEnv = MockGenerator::getEnv();
    MockEnvironment* mockEnv = dynamic_cast<MockEnvironment*>(baseEnv.get());
    EXPECT_CALL(*mockEnv, scanPort).Times(0);
    PQConn pq = PQConn();
//    PQConn pq;
//    pq.execute("SELECT * FROM WWW.GOOGLE.COM WHERE PORT = 1000 AND PORT = 1");
}


TEST(RunScan, ExpectedOnePortSubmitted) {

    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr baseEnv = mockGenerator(0);
    std::shared_ptr<MockEnvironment> mockEnv = std::dynamic_pointer_cast<MockEnvironment>(baseEnv);
    EXPECT_CALL(*mockEnv, submitPortForScan(_, _)).Times(0);
    EXPECT_CALL(*mockEnv, submitPortForScan(1000, _)).Times(1);
    PortQuery pq;
    pq.execute("SELECT * FROM WWW.GOOGLE.COM WHERE PORT = 1000");
}

TEST(RunScan, ExpectedTwoPortsSubmitted) {

    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr baseEnv = mockGenerator(0);
    std::shared_ptr<MockEnvironment> mockEnv = std::dynamic_pointer_cast<MockEnvironment>(baseEnv);
    EXPECT_CALL(*mockEnv, submitPortForScan(_, _)).Times(0);
    EXPECT_CALL(*mockEnv, submitPortForScan(5000, _)).Times(1);
    EXPECT_CALL(*mockEnv, submitPortForScan(1000, _)).Times(1);
    PortQuery pq;
    pq.execute("SELECT * FROM WWW.GOOGLE.COM WHERE PORT = 1000 OR PORT = 5000");
}


TEST(RunScan, ExpectedMultiplePortsSubmitted) {

    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr baseEnv = mockGenerator(0);
    std::shared_ptr<MockEnvironment> mockEnv = std::dynamic_pointer_cast<MockEnvironment>(baseEnv);
    EXPECT_CALL(*mockEnv, submitPortForScan(Ge(100), _)).Times(0);
    EXPECT_CALL(*mockEnv, submitPortForScan(Lt(100), _)).Times(100);
    PortQuery pq;
    pq.execute("SELECT * FROM WWW.GOOGLE.COM WHERE PORT < 100");
}

TEST(RunScan, ExpectedAllPortsSubmitted) {

    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr baseEnv = mockGenerator(0);
    std::shared_ptr<MockEnvironment> mockEnv = std::dynamic_pointer_cast<MockEnvironment>(baseEnv);
    EXPECT_CALL(*mockEnv, submitPortForScan(_, _)).Times(static_cast<uint16_t>(-1) + 1);
    PortQuery pq;
    pq.execute("SELECT * FROM WWW.GOOGLE.COM WHERE UDP = CLOSED");
}
*/
