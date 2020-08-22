#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Statement.h"


using namespace PortQuery;

class MockEnvironment : public IEnvironment {

    public:

        virtual bool scanPort(void) override {

            return true;
        }
};


TEST(CompareTerminals, BETWEENTerminals) {


    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr env = EnvironmentFactory::createEnvironment(0);

    BETWEENExpression between_T1 = BETWEENExpression(0, 10, NumericToken{ 5 });
    ASSERT_TRUE(Tristate::TRUE_STATE == between_T1.attemptPreNetworkEval(env));

    BETWEENExpression between_T2 = BETWEENExpression(0, 10, ColumnToken{ ColumnToken::PORT });
    env->setPort(5);
    ASSERT_TRUE(Tristate::TRUE_STATE == between_T2.attemptPreNetworkEval(env));
    env->setPort(11);
    ASSERT_TRUE(Tristate::FALSE_STATE == between_T2.attemptPreNetworkEval(env));


}


