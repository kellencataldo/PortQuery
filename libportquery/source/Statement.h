#pragma once

#include <cstdint>
#include <memory>

#include "Lexer.h"
#include "Network.h"


#define UNUSED_PARAMETER(x) (void) (x)


struct IExpression {

    enum PreNetworkEvaluation {
        EvaluatedTruePreNet,
        EvaluatedFalsePreNet,
        UnableToEvaluatePreNet
    };

    virtual PreNetworkEvaluation attemptPreNetworkEval(const uint16_t port) const = 0;
    virtual ~IExpression() { }
};

using SOSQLExpression = std::unique_ptr<IExpression>;

struct ORExpression : IExpression {

    ORExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

    virtual PreNetworkEvaluation attemptPreNetworkEval(const uint16_t port) const override;

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct ANDExpression : IExpression {

    ANDExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }
    virtual PreNetworkEvaluation attemptPreNetworkEval(const uint16_t port) const override;


    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct NOTExpression : IExpression {

    NOTExpression(SOSQLExpression expr) : m_expr(std::move(expr)) { }

    virtual bool evaluate(const uint16_t port, NetworkEnvPtr netenv) const override {

        return !m_expr->evaluate(port, netenv);
    }


    virtual bool shouldSubmitForScan(const uint16_t port, NetworkEnvPtr netenv) const override {

        return m_expr->shouldSubmitForScan(port, netenv);
    }

    SOSQLExpression m_expr;
};

struct BETWEENExpression : IExpression {

    BETWEENExpression(const Token terminal, const uint16_t lowerBound, const uint16_t upperBound) :
        m_terminal(terminal), m_lowerBound(lowerBound), m_upperBound(upperBound) { }

    virtual bool shouldSubmitForScan(uint16_t port, NetworkEnvPtr netenv) const override;

    Token m_terminal;
    uint16_t m_lowerBound;
    uint16_t m_upperBound;
};

struct ComparisonExpression : IExpression {

    ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) :
       m_op(op), m_LHSTerminal(lhs), m_RHSTerminal(rhs) { }

    virtual bool shouldSubmitForScan(const uint16_t port, NetworkEnvPtr netenv) const override;

    ComparisonToken::OpType m_op;
    Token m_LHSTerminal;
    Token m_RHSTerminal;
};

struct NULLExpression : IExpression {

    virtual bool shouldSubmitForScan(const uint16_t port, NetworkEnvPtr netenv) const override { 

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


