#pragma once

#include <string>
#include <memory>

#include "Lexer.h"
#include "Network.h"


struct ORExpression;
struct ANDExpression;


using SOSQLExpression = std::variant<
    std::shared_ptr<ORExpression>,
    std::shared_ptr<ANDExpression>
    >;


struct ORExpression {

    SOSQLExpression ANDLeft;
    SOSQLExpression ANDRight;
};

struct ANDExpression {};


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

        Lexer m_lexer;
};
