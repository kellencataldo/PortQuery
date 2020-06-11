#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"


TEST(ParseSOSQLStatements, ParseColumnList) {

    const SelectSet selectSet_T1 = Parser("SELECT * FROM WWW.GOOGLE.COM").parseSOSQLStatement().m_selectSet;
    EXPECT_TRUE(selectSet_T1.m_selectPort);
    EXPECT_TRUE( (NetworkProtocols::TCP | NetworkProtocols::UDP) == selectSet_T1.m_selectedProtocols);

    const SelectSet selectSet_T2 = Parser("SELECT TCP, UDP FROM WWW.GOOGLE.COM").parseSOSQLStatement().m_selectSet;
    EXPECT_FALSE(selectSet_T2.m_selectPort);
    EXPECT_TRUE( (NetworkProtocols::TCP | NetworkProtocols::UDP) == selectSet_T2.m_selectedProtocols);





}




