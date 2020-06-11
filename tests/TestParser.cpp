#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"


TEST(ParseSOSQLStatements, ParseColumnList) {

    Parser query_T1("SELECT * FROM WWW.GOOGLE.COM");
    const SelectSet selectSet_T1 = query_T1.parseSOSQLStatement().m_selectSet;

    EXPECT_TRUE(selectSet_T1.m_selectPort);
    EXPECT_TRUE( (NetworkProtocols::TCP | NetworkProtocols::UDP) == selectSet_T1.m_selectedProtocols);

}




