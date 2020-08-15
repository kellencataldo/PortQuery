#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "Statement.h"


namespace PortQuery {

    std::string getExtendedTokenInfo(const NumericToken n);
    std::string getExtendedTokenInfo(const UserToken u);
    std::string getExtendedTokenInfo(const ComparisonToken c);
    std::string getExtendedTokenInfo(const KeywordToken k);
    std::string getExtendedTokenInfo(const ColumnToken c);
    std::string getExtendedTokenInfo(const QueryResultToken q);
    std::string getTokenString(const Token t);

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
}
