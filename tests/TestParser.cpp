#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"
#include "../libportquery/source/Statement.h"


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

    const auto select_T1 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500").parseSOSQLStatement();
    const IExpression* const AST_T1 = select_T1->getTableExpression();

    EXPECT_TRUE(Tristate::FALSE_STATE == AST_T1->attemptPreNetworkEval(1));
    EXPECT_TRUE(Tristate::TRUE_STATE == AST_T1->attemptPreNetworkEval(101));

}



