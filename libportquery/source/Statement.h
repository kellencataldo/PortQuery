#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <tuple>

#include "Lexer.h"
#include "Network.h"


#define UNUSED_PARAMETER(x) (void) (x)

enum class Tristate : int {
    FALSE_STATE = -1,
    UNKNOWN_STATE = 0,
    TRUE_STATE = 1,
};

Tristate operator||(const Tristate lhs, const Tristate rhs);
Tristate operator&&(const Tristate lhs, const Tristate rhs);
Tristate operator!(const Tristate rhs);

struct IExpression {

    virtual Tristate attemptPreNetworkEval(const uint16_t port) const = 0;
    static std::tuple<bool, uint16_t> getPreNetworkValue(const Token terminal, const uint16_t port);
    virtual ~IExpression() { }
};

using SOSQLExpression = std::unique_ptr<IExpression>;

struct ORExpression : IExpression {

    ORExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

    virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

        return m_left->attemptPreNetworkEval(port) || m_right->attemptPreNetworkEval(port);
    }

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct ANDExpression : IExpression {

    ANDExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

    virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

        return m_left->attemptPreNetworkEval(port) && m_right->attemptPreNetworkEval(port);
    }

    SOSQLExpression m_left;
    SOSQLExpression m_right;
};

struct NOTExpression : IExpression {

    NOTExpression(SOSQLExpression expr) : m_expr(std::move(expr)) { }
    virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

        return !m_expr->attemptPreNetworkEval(port);
    }

    SOSQLExpression m_expr;
};

struct BETWEENExpression : IExpression {

    BETWEENExpression(const Token terminal, const uint16_t lowerBound, const uint16_t upperBound) :
        m_terminal(terminal), m_lowerBound(lowerBound), m_upperBound(upperBound) { }

    virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;

    static bool Evaluate(const uint16_t value, const uint16_t lowerBound, const uint16_t upperBound) {

        return lowerBound <= value && value <= upperBound;
    }

    Token m_terminal;
    uint16_t m_lowerBound;
    uint16_t m_upperBound;
};

struct ComparisonExpression : IExpression {

    ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) :
       m_op(op), m_LHSTerminal(lhs), m_RHSTerminal(rhs) { }

    virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;

    static bool Evaluate(ComparisonToken::OpType, const uint16_t lhs, const uint16_t rhs);

    ComparisonToken::OpType m_op;
    Token m_LHSTerminal;
    Token m_RHSTerminal;
};

struct NULLExpression : IExpression {

    virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

        return Tristate::TRUE_STATE;
    }
};

using SelectSet = std::vector<ColumnToken::Column>;


class SelectStatement {
    public:
        SelectStatement(SelectSet selectedSet, std::string tableReference, SOSQLExpression tableExpression) : 
            m_selectedSet(std::move(selectedSet)), m_tableReference(std::move(tableReference)), 
            m_tableExpression(std::move(tableExpression)) { }

        const IExpression * const getTableExpression() const {

            return m_tableExpression.get();
        }

        SelectSet getSelectSet() const {

            return m_selectedSet;
        }

    private:

        SelectSet m_selectedSet;
        std::string m_tableReference;
        SOSQLExpression m_tableExpression;
};

typedef std::unique_ptr<SelectStatement> SOSQLSelectStatement;


