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


TEST(CompareTerminalsPreNetwork, BETWEENTerminals) {

    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr env = EnvironmentFactory::createEnvironment(0);

    BETWEENExpression between_T1 = BETWEENExpression(0, 10, NumericToken{ 5 });
    ASSERT_TRUE(Tristate::TRUE_STATE == between_T1.attemptPreNetworkEval(env));

    BETWEENExpression between_T2 = BETWEENExpression(0, 10, NumericToken{ 11 });
    ASSERT_TRUE(Tristate::FALSE_STATE == between_T2.attemptPreNetworkEval(env));

    BETWEENExpression between_T3 = BETWEENExpression(0, 10, ColumnToken{ ColumnToken::PORT });
    env->setPort(5);
    ASSERT_TRUE(Tristate::TRUE_STATE == between_T3.attemptPreNetworkEval(env));
    env->setPort(11);
    ASSERT_TRUE(Tristate::FALSE_STATE == between_T3.attemptPreNetworkEval(env));

    EXPECT_THROW(BETWEENExpression(0, 1, ColumnToken{ ColumnToken::UDP }), std::invalid_argument);
    EXPECT_THROW(BETWEENExpression(0, 1, ColumnToken{ ColumnToken::TCP }), std::invalid_argument);
    EXPECT_THROW(BETWEENExpression(0, 1, QueryResultToken{ PQ_QUERY_RESULT::OPEN }), std::invalid_argument);
    EXPECT_THROW(BETWEENExpression(0, 1, QueryResultToken{ PQ_QUERY_RESULT::CLOSED }), std::invalid_argument);
    EXPECT_THROW(BETWEENExpression(0, 1, QueryResultToken{ PQ_QUERY_RESULT::REJECTED }), std::invalid_argument);
}

TEST(CompareTerminalsPreNetwork, ComparisonExpression) {

    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr env = EnvironmentFactory::createEnvironment(0);
    env->setPort(0);

    ComparisonExpression comparison_T1 = ComparisonExpression(ComparisonToken::OP_EQ, NumericToken{1}, NumericToken{1});
    ASSERT_TRUE(Tristate::TRUE_STATE == comparison_T1.attemptPreNetworkEval(env));

    ComparisonExpression comparison_T2 = ComparisonExpression(ComparisonToken::OP_GT, NumericToken{1}, NumericToken{2});
    ASSERT_TRUE(Tristate::FALSE_STATE == comparison_T2.attemptPreNetworkEval(env));

    ComparisonExpression comparison_T3 = ComparisonExpression(ComparisonToken::OP_LT, ColumnToken{ColumnToken::PORT}, NumericToken{2});
    ASSERT_TRUE(Tristate::TRUE_STATE == comparison_T3.attemptPreNetworkEval(env));

    env->setPort(100);
    ComparisonExpression comparison_T4 = ComparisonExpression(ComparisonToken::OP_LT, NumericToken{2}, ColumnToken{ColumnToken::PORT});
    ASSERT_TRUE(Tristate::TRUE_STATE == comparison_T4.attemptPreNetworkEval(env));

    ComparisonExpression comparison_T5 = ComparisonExpression(ComparisonToken::OP_EQ, QueryResultToken{PQ_QUERY_RESULT::CLOSED}, 
            ColumnToken{ColumnToken::UDP});
    ASSERT_TRUE(Tristate::UNKNOWN_STATE == comparison_T5.attemptPreNetworkEval(env));

    ComparisonExpression comparison_T6 = ComparisonExpression(ComparisonToken::OP_NE, ColumnToken{ColumnToken::UDP},
            QueryResultToken{PQ_QUERY_RESULT::OPEN});
    ASSERT_TRUE(Tristate::UNKNOWN_STATE == comparison_T6.attemptPreNetworkEval(env));

    ComparisonExpression comparison_T7 = ComparisonExpression(ComparisonToken::OP_EQ, QueryResultToken{PQ_QUERY_RESULT::OPEN},
            QueryResultToken{PQ_QUERY_RESULT::OPEN});
    ASSERT_TRUE(Tristate::TRUE_STATE == comparison_T7.attemptPreNetworkEval(env));

    ComparisonExpression comparison_T8 = ComparisonExpression(ComparisonToken::OP_EQ, QueryResultToken{PQ_QUERY_RESULT::CLOSED},
            QueryResultToken{PQ_QUERY_RESULT::REJECTED});
    ASSERT_TRUE(Tristate::FALSE_STATE == comparison_T8.attemptPreNetworkEval(env));

    EXPECT_THROW(ComparisonExpression(ComparisonToken::OP_LT, NumericToken{2}, QueryResultToken{PQ_QUERY_RESULT::CLOSED}), std::invalid_argument);
    EXPECT_THROW(ComparisonExpression(ComparisonToken::OP_LT, QueryResultToken{PQ_QUERY_RESULT::OPEN}, NumericToken{1}), std::invalid_argument);
    EXPECT_THROW(ComparisonExpression(ComparisonToken::OP_LT, ColumnToken{ColumnToken::PORT}, QueryResultToken{PQ_QUERY_RESULT::REJECTED}), std::invalid_argument);
    EXPECT_THROW(ComparisonExpression(ComparisonToken::OP_LT, ColumnToken{ColumnToken::UDP}, NumericToken{0}), std::invalid_argument);
    EXPECT_THROW(ComparisonExpression(ComparisonToken::OP_LT, ColumnToken{ColumnToken::TCP}, ColumnToken{ColumnToken::PORT}), std::invalid_argument);
}
