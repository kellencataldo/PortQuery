#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "Lexer.h"
#include "Network.h"


template <typename T> void UNUSED_PARAMETER(T &&) { };

//
struct BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const = 0;
};

using SOSQLExpression = std::shared_ptr<BaseExpression>;

struct ORExpression : BaseExpression {

    ORExpression(const SOSQLExpression left, const SOSQLExpression right) : m_left(left), m_right(right) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return m_left->shouldSubmitForScan(port) || m_right->shouldSubmitForScan(port);
    }

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct ANDExpression : BaseExpression {

    ANDExpression(const SOSQLExpression left, const SOSQLExpression right) : m_left(left), m_right(right) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return m_left->shouldSubmitForScan(port) && m_right->shouldSubmitForScan(port);
    }

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct NOTExpression : BaseExpression {

    NOTExpression(const SOSQLExpression expr) : m_expr(expr) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return m_expr->shouldSubmitForScan(port);
    }

    SOSQLExpression m_expr;
};

struct BETWEENExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return (port >= m_lowerBound) && (port <= m_upperBound);
    }

    uint16_t m_lowerBound;
    uint16_t m_upperBound;
};

struct ComparisonExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const;

    ComparisonToken::OpType op;
    Token m_LHSTerminal;
    Token m_RHSTerminal;
};

struct NULLExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        UNUSED_PARAMETER(port);
        return true;
    }
};


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
        SOSQLExpression parseBooleanFactor();
        SOSQLExpression parseBooleanExpression();

        SOSQLExpression parseComparisonExpression(const Token lhs);
        SOSQLExpression parseISExpression(const Token lhs);
        SOSQLExpression parseBETWEENExpression(const Token lhs);

        Lexer m_lexer;
};
