#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"


TEST(ParseSOSQLStatements, ParseColumnList) {

    const auto select_T1 = Parser("SELECT * FROM WWW.GOOGLE.COM").parseSOSQLStatement();
    EXPECT_TRUE(select_T1.m_selectPort);
    EXPECT_TRUE( (NetworkProtocols::TCP | NetworkProtocols::UDP) == select_T1.m_selectedProtocols);

    const auto select_T2 = Parser("SELECT TCP, UDP FROM WWW.YAHOO.COM").parseSOSQLStatement();
    EXPECT_FALSE(select_T2.m_selectPort);
    EXPECT_TRUE( (NetworkProtocols::TCP | NetworkProtocols::UDP) == select_T2.m_selectedProtocols);

}




