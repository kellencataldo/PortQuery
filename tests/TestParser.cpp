#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"
#include "../libportquery/source/Statement.h"
#include "../libportquery/source/Network.h"

#include <iostream>

TEST(ParseSOSQLStatements, ParseColumnList) {

    const auto select_T1 = Parser("SELECT * FROM WWW.GOOGLE.COM").parseSOSQLStatement();
    SelectSet set_T1{ColumnToken::PORT, ColumnToken::TCP, ColumnToken::UDP};
    EXPECT_TRUE( set_T1 == select_T1->getSelectSet() );

    SelectSet set_T2{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T2 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM").parseSOSQLStatement();
    EXPECT_TRUE( set_T2 == select_T2->getSelectSet() );

    SelectSet set_T3{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T3 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500").parseSOSQLStatement();
    EXPECT_TRUE( set_T3 == select_T3->getSelectSet() );

}


TEST(ParseSOSQLStatements, ParseWHEREStatement) {

    const auto select_T1 = Parser("SELECT * FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500").parseSOSQLStatement();
    const IExpression* const AST_T1 = select_T1->getTableExpression();

    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T1->attemptPreNetworkEval(1));
    EXPECT_TRUE(Tristate::TRUE_STATE == AST_T1->attemptPreNetworkEval(101));
    
    std::string sosql_T2 = "SELECT * FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500 OR PORT BETWEEN 600 AND 700";
    const auto select_T2 = Parser(sosql_T2).parseSOSQLStatement();
    const IExpression* const AST_T2 = select_T2->getTableExpression();

    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T2->attemptPreNetworkEval(1));
    EXPECT_TRUE(Tristate::TRUE_STATE == AST_T2->attemptPreNetworkEval(101));

    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T2->attemptPreNetworkEval(550));
    EXPECT_TRUE(Tristate::TRUE_STATE == AST_T2->attemptPreNetworkEval(600));

    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T2->attemptPreNetworkEval(701));

    std::string sosql_T3 = "SELECT * FROM 127.0.0.1 WHERE PORT BETWEEN 100 AND 500 AND UDP = OPEN";
    const auto select_T3 = Parser(sosql_T3).parseSOSQLStatement();
    const IExpression* const AST_T3 = select_T3->getTableExpression();

    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T3->attemptPreNetworkEval(1));
    EXPECT_TRUE(Tristate::UNKNOWN_STATE == AST_T3->attemptPreNetworkEval(101));
    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T3->attemptPreNetworkEval(600));

    const auto select_T4 = Parser("SELECT * FROM 127.0.0.1 WHERE REJECTED = TCP").parseSOSQLStatement();
    const IExpression* const AST_T4 = select_T4->getTableExpression();
    EXPECT_TRUE(Tristate::UNKNOWN_STATE == AST_T4->attemptPreNetworkEval(100));

    const auto select_T5 = Parser("SELECT * FROM GOOGLE.COM WHERE NOT PORT = 100").parseSOSQLStatement();
    const IExpression* const AST_T5 = select_T5->getTableExpression();
    EXPECT_TRUE(Tristate::TRUE_STATE == AST_T5->attemptPreNetworkEval(101));
    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T5->attemptPreNetworkEval(100));
}


TEST(ParseSOSQLStatements, CollectRequiredProtocols) {

    // should be expanded.
    const auto select_T1 = Parser("SELECT * FROM WWW.YAHOO.COM WHERE UDP = CLOSED").parseSOSQLStatement();
    EXPECT_TRUE((NetworkProtocols::UDP | NetworkProtocols::TCP) == select_T1->collectRequiredProtocols());

    const auto select_T2 = Parser("SELECT PORT, UDP FROM WWW.YAHOO.COM").parseSOSQLStatement();
    EXPECT_TRUE(NetworkProtocols::UDP == select_T2->collectRequiredProtocols());

    const auto select_T3 = Parser("SELECT PORT FROM WWW.YAHOO.COM WHERE UDP = CLOSED AND REJECTED = TCP").parseSOSQLStatement();
    EXPECT_TRUE((NetworkProtocols::UDP | NetworkProtocols::TCP) == select_T3->collectRequiredProtocols());

    const auto select_T4 = Parser("SELECT PORT FROM WWW.YAHOO.COM WHERE PORT < 40").parseSOSQLStatement();
    EXPECT_TRUE(NetworkProtocols::NONE == select_T4->collectRequiredProtocols());

    const auto select_T5 = Parser("SELECT TCP FROM WWW.YAHOO.COM WHERE PORT < 40").parseSOSQLStatement();
    EXPECT_TRUE(NetworkProtocols::TCP == select_T5->collectRequiredProtocols());
}



