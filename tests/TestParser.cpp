#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"


TEST(ParseSOSQLStatements, ParseColumnList) {

    const auto select_T1 = Parser("SELECT * FROM WWW.GOOGLE.COM").parseSOSQLStatement();
    SelectSet set_T1{ColumnToken::PORT, ColumnToken::TCP, ColumnToken::UDP};
    EXPECT_TRUE( set_T1 == select_T1.m_selectSet);

    SelectSet set_T2{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T2 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM").parseSOSQLStatement();
    EXPECT_TRUE( set_T2 == select_T2.m_selectSet);

}


TEST(ParseSOSQLStatements, ParseWHEREStatement) {

    const auto select_T1 = Parser("SELECT * FROM WWW.GOOGLE.COM WHERE UDP = OPEN").parseSOSQLStatement();
    SelectSet set_T1{ColumnToken::PORT, ColumnToken::TCP, ColumnToken::UDP};

    SelectSet set_T2{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T2 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM WHERE 1 = TCP").parseSOSQLStatement();

    SelectSet set_T3{ColumnToken::TCP, ColumnToken::UDP};
    const auto select_T3 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM WHERE PORT BETWEEN 100 AND 500").parseSOSQLStatement();
}
