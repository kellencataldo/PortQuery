#include <string>

#include "gmock/gmock.h"
#include "../libportquery/source/Environment.h"
#include "../libportquery/include/PortQuery.h"

using ::testing::AtLeast;
using ::testing::_;



class MockEnvironment : public IEnvironment {

    public:

        MOCK_METHOD(bool, submitPortForScan, (const uint16_t, const NetworkProtocols), (override));
};


TEST(RunScan, ExpectedNumberOfTokensSubmitted) {


    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr baseEnv = mockGenerator(0);
    std::shared_ptr<MockEnvironment> mockEnv = std::dynamic_pointer_cast<MockEnvironment>(baseEnv);
    EXPECT_CALL(*mockEnv, submitPortForScan(_, _)).Times(1);
    PortQuery pq;
    pq.execute("SELECT * FROM WWW.GOOGLE.COM WHERE PORT = 1000");
}
