#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "Lexer.h"
#include "Network.h"
#include "Expression.h"


#define UNUSED_PARAMETER(x) (void) (x)


class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        SOSQLSelectStatement parseSOSQLStatement();


    private:

        SelectSet parseSelectSetQuantifier();
        SelectSet parseSelectList();

        std::string parseTableReference();

        SOSQLExpression parseTableExpression();
        SOSQLExpression parseORExpression();
        SOSQLExpression parseANDExpression();
        SOSQLExpression parseBooleanFactor();
        SOSQLExpression parseBooleanExpression();

        SOSQLExpression parseComparisonExpression(const Token lhs);
        SOSQLExpression parseISExpression(const Token lhs);
        SOSQLExpression parseBETWEENExpression(const Token lhs);

        Lexer m_lexer;
};
