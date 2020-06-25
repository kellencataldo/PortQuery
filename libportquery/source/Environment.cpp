#include "Environment.h"


bool Environment::availablePreSubmission(const Token t) const {

    return std::visit(overloaded {
            [=] (ColumnToken c)     { return ColumnToken::PORT == c.m_column; },
            [=] (NumericToken)      { return true; },
            [=] (QueryResultToken)  { return true; },
            [=] (auto) -> bool      { throw std::invalid_argument("Invalid token stored as expression primary"); } 
        }, t);
}


uint16_t Environment::retrieveTokenValue(const Token t) {

    return std::visit(overloaded {
            [=] (ColumnToken c)       { return getColumnResult(c.m_column); },
            [=] (NumericToken n)      { return n.m_value; },
            [=] (QueryResultToken q)  { return q.m_queryResult; },
            [=] (auto) -> uint16_t    { throw std::invalid_argument("Invalid token stored as expression primary"); } 
        }, t);
}


uint16_t Environment::getColumnResult(ColumnToken::Column c) {

    return 0;
}

