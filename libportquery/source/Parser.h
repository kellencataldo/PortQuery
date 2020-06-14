#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "Lexer.h"
#include "Network.h"


struct ORExpression;
struct ANDExpression;
struct BETWEENExpression;
struct ComparisonExpression;

struct NULLExpression;


using SOSQLExpression = std::variant<
    std::shared_ptr<ORExpression>,
    std::shared_ptr<ANDExpression>,
    std::shared_ptr<BETWEENExpression>,
    std::shared_ptr<ComparisonExpression>,
    // special case, no WHERE clause
    std::shared_ptr<NULLExpression>
    >;


// what should this be?
struct NULLExpression { };


struct ORExpression {
    SOSQLExpression left;
    SOSQLExpression right;
    
};

struct ANDExpression {
    SOSQLExpression left;
    SOSQLExpression right;
};

/*
struct BETWEENExpression {
    uint16_t m_lowerBound;
    uint16_t m_upperBound;
    ColumnToken m_comparisonColumn;
    // column token here.
};

*/




struct SelectStatement {

    bool m_selectPort;

    NetworkProtocols m_selectedProtocols;

    std::string m_tableReference;

    SOSQLExpression m_tableExpression;
};

typedef SelectStatement SOSQLSelectStatement;

class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // SELECT Only SQL
        // maybe make this a unique_ptr? investigation required.
        SOSQLSelectStatement parseSOSQLStatement();


    private:

        std::tuple<bool, NetworkProtocols> parseSelectSetQuantifier();
        std::tuple<bool, NetworkProtocols> parseSelectList();

        std::string parseTableReference();

        SOSQLExpression parseTableExpression();
        SOSQLExpression parseORExpression();
        SOSQLExpression parseANDExpression();

        Lexer m_lexer;
};
