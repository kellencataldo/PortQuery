#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "Lexer.h"
#include "Network.h"


#define UNUSED_PARAMETER(x) (void) (x)


struct BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const = 0;
    virtual ~BaseExpression() { }
};

using SOSQLExpression = std::unique_ptr<BaseExpression>;

struct ORExpression : BaseExpression {

    ORExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }
    virtual bool shouldSubmitForScan(const uint16_t port) const override {

        return m_left->shouldSubmitForScan(port) || m_right->shouldSubmitForScan(port);
    }

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct ANDExpression : BaseExpression {

    ANDExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const override {

        return m_left->shouldSubmitForScan(port) && m_right->shouldSubmitForScan(port);
    }

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct NOTExpression : BaseExpression {

    NOTExpression(SOSQLExpression expr) : m_expr(std::move(expr)) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const override {

        return m_expr->shouldSubmitForScan(port);
    }

    SOSQLExpression m_expr;
};

struct BETWEENExpression : BaseExpression {

    BETWEENExpression(const Token terminal, const uint16_t lowerBound, const uint16_t upperBound) :
        m_terminal(terminal), m_lowerBound(lowerBound), m_upperBound(upperBound) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const override {

        return (port >= m_lowerBound) && (port <= m_upperBound);
    }

    Token m_terminal;
    uint16_t m_lowerBound;
    uint16_t m_upperBound;
};

struct ComparisonExpression : BaseExpression {

    ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) :
       m_op(op), m_LHSTerminal(lhs), m_RHSTerminal(rhs) { }

    virtual bool shouldSubmitForScan(const uint16_t port) const override { 

        UNUSED_PARAMETER(port);
        return true; 
    }

    ComparisonToken::OpType m_op;
    Token m_LHSTerminal;
    Token m_RHSTerminal;
};

struct NULLExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const override { 

        UNUSED_PARAMETER(port);
        return true;
    }
};

using SelectSet = std::vector<ColumnToken::Column>;


class SelectStatement {
    public:
        SelectStatement(SelectSet selectedSet, std::string tableReference, SOSQLExpression tableExpression) : 
            m_selectedSet(std::move(selectedSet)), m_tableReference(std::move(tableReference)), 
            m_tableExpression(std::move(tableExpression)) { }

        SelectSet getSelectSet() const {

            return m_selectedSet;
        }

    private:
        SelectSet m_selectedSet;
        std::string m_tableReference;
        SOSQLExpression m_tableExpression;
};

typedef std::unique_ptr<SelectStatement> SOSQLSelectStatement;

class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // SELECT Only SQL
        // maybe make this a unique_ptr? investigation required.
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
