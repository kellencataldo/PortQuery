#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"
#include "../libportquery/source/Statement.h"
#include "../libportquery/source/Network.h"


using namespace PortQuery;

class MockEnvironment : public IEnvironment {

    public:

        virtual bool scanPort(void) override {

            return true;
        }
};


TEST(ParseSOSQLStatements, ParseColumnList) {

    /*
    const auto select_T1 = Parser("SELECT * FROM WWW.GOOGLE.COM").parseSOSQLStatement();
    SelectSet set_T1{ColumnToken::PORT, ColumnToken::TCP, ColumnToken::UDP};
    EXPECT_TRUE( set_T1 == select_T1->getSelectSet() );

    SelectSet set_T2{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T2 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM").parseSOSQLStatement();
    EXPECT_TRUE( set_T2 == select_T2->getSelectSet() );

    SelectSet set_T3{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T3 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500").parseSOSQLStatement();
    EXPECT_TRUE( set_T3 == select_T3->getSelectSet() );
    */

}


TEST(ParseSOSQLStatements, ParseWHEREStatement) {


    auto* mockGenerator = +[] (const int _) -> EnvironmentPtr { 
        static EnvironmentPtr mockEnv = std::make_shared<MockEnvironment>();
        return mockEnv;
    };

    EnvironmentFactory::setGenerator(mockGenerator);
    EnvironmentPtr env = EnvironmentFactory::createEnvironment(0);

    const auto select_T1 = Parser("SELECT * FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500").parseSOSQLStatement();

    env->setPort(1);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T1->attemptPreNetworkEval(env));

    env->setPort(101);
    EXPECT_TRUE(Tristate::TRUE_STATE == select_T1->attemptPreNetworkEval(env));
    
    std::string sosql_T2 = "SELECT * FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500 OR PORT BETWEEN 600 AND 700";
    const auto select_T2 = Parser(sosql_T2).parseSOSQLStatement();

    env->setPort(1);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T2->attemptPreNetworkEval(env));

    env->setPort(101);
    EXPECT_TRUE(Tristate::TRUE_STATE == select_T2->attemptPreNetworkEval(env));

    env->setPort(550);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T2->attemptPreNetworkEval(env));

    env->setPort(600);
    EXPECT_TRUE(Tristate::TRUE_STATE == select_T2->attemptPreNetworkEval(env));

    env->setPort(701);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T2->attemptPreNetworkEval(env));

    std::string sosql_T3 = "SELECT * FROM 127.0.0.1 WHERE PORT BETWEEN 100 AND 500 AND UDP = OPEN";
    const auto select_T3 = Parser(sosql_T3).parseSOSQLStatement();


    env->setPort(1);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T3->attemptPreNetworkEval(env));

    env->setPort(101);
    EXPECT_TRUE(Tristate::UNKNOWN_STATE == select_T3->attemptPreNetworkEval(env));

    env->setPort(600);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T3->attemptPreNetworkEval(env));

    const auto select_T4 = Parser("SELECT * FROM 127.0.0.1 WHERE REJECTED = TCP").parseSOSQLStatement();

    env->setPort(100);
    EXPECT_TRUE(Tristate::UNKNOWN_STATE == select_T4->attemptPreNetworkEval(env));

    const auto select_T5 = Parser("SELECT * FROM GOOGLE.COM WHERE NOT PORT = 100").parseSOSQLStatement();

    env->setPort(101);
    EXPECT_TRUE(Tristate::TRUE_STATE == select_T5->attemptPreNetworkEval(env));

    env->setPort(100);
    EXPECT_TRUE(Tristate::FALSE_STATE == select_T5->attemptPreNetworkEval(env));


    // just an assortment of nonsensical SOSQL statements, feel free to add
    EXPECT_THROW(Parser("SELECT * FROM GOOGLE.COM WHERE").parseSOSQLStatement(), std::invalid_argument);
    EXPECT_THROW(Parser("SELECT * FROM GOOGLE.COM WHERE NOT PORT = CLOSED").parseSOSQLStatement(), std::invalid_argument);
    EXPECT_THROW(Parser("SELECT * FROM GOOGLE.COM WHERE UDP AND CLOSED").parseSOSQLStatement(), std::invalid_argument);
    EXPECT_THROW(Parser("SELECT * FROM GOOGLE.COM WHERE REJECTED").parseSOSQLStatement(), std::invalid_argument);
    EXPECT_THROW(Parser("SELECT * FROM GOOGLE.COM WHERE 1 = OPEN").parseSOSQLStatement(), std::invalid_argument);
    EXPECT_THROW(Parser("SELECT * FROM GOOGLE.COM WHERE TCP = 5").parseSOSQLStatement(), std::invalid_argument);
}


TEST(ParseSOSQLStatements, CollectRequiredProtocols) {

    // should be expanded.
    const auto select_T1 = Parser("SELECT * FROM WWW.YAHOO.COM WHERE UDP = CLOSED").parseSOSQLStatement();
    EXPECT_TRUE((NetworkProtocol::UDP | NetworkProtocol::TCP) == select_T1->collectRequiredProtocols());

    const auto select_T2 = Parser("SELECT PORT, UDP FROM WWW.YAHOO.COM").parseSOSQLStatement();
    EXPECT_TRUE(NetworkProtocol::UDP == select_T2->collectRequiredProtocols());

    const auto select_T3 = Parser("SELECT PORT FROM WWW.YAHOO.COM WHERE UDP = CLOSED AND REJECTED = TCP").parseSOSQLStatement();
    EXPECT_TRUE((NetworkProtocol::UDP | NetworkProtocol::TCP) == select_T3->collectRequiredProtocols());

    const auto select_T4 = Parser("SELECT PORT FROM WWW.YAHOO.COM WHERE PORT < 40").parseSOSQLStatement();
    EXPECT_TRUE(NetworkProtocol::NONE == select_T4->collectRequiredProtocols());

    const auto select_T5 = Parser("SELECT TCP FROM WWW.YAHOO.COM WHERE PORT < 40").parseSOSQLStatement();
    EXPECT_TRUE(NetworkProtocol::TCP == select_T5->collectRequiredProtocols());
}
