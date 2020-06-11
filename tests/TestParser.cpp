#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Parser.h"


TEST(ParseSOSQLStatements, ParseColumnList) {



    Parser query_T1("SELECT * FROM WWW.GOOGLE.COM");

    SOSQLSelectStatement stmt_T1 = query_T1.parseSOSQLStatement();

    
    


}




