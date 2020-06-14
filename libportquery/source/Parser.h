#pragma once

#include <string>
#include <memory>

#include "Lexer.h"
#include "Network.h"


struct ORExpression;
struct ANDExpression;
struct NULLExpression;
struct ComparisonExpression;


using SOSQLExpression = std::variant<
    std::shared_ptr<ORExpression>,
    std::shared_ptr<ANDExpression>,
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





struct SelectSet {
    bool m_selectPort;
    NetworkProtocols m_selectedProtocols;
};

// and expression
// or expression
//

struct SelectStatement {
    SelectSet m_selectSet;
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
        SOSQLSelectStatement parseSimpleSelect();
//        SOSQLSelectStatement parseCountSelect();

        SelectSet parseSelectSetQuantifier();
        SelectSet parseSelectList();

        std::string parseTableReference();

        SOSQLExpression parseTableExpression();
        SOSQLExpression parseORExpression();
        SOSQLExpression parseANDExpression();

        Lexer m_lexer;
};
