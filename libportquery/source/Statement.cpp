#include "Statement.h"


std::tuple<bool, uint16_t> IExpression::getPreNetworkValue(const Token terminal, const uint16_t port) {

    return std::visit(overloaded {
            [=] (QueryResultToken q) { return std::make_tuple(true, q.m_queryResult); },
            [=] (NumericToken n)     { return std::make_tuple(true, n.m_value); },
            [=] (auto)               { return std::make_tuple(false, 0); },  // better error handling needed
            [=] (ColumnToken c) -> std::tuple<bool, uint16_t> { 
                if (ColumnToken::PORT == c.m_column) {
                    return std::make_tuple(true, port);
                }
                return std::make_tuple(false, 0);
            } },
        terminal);
}


Tristate BETWEENExpression::attemptPreNetworkEval(const uint16_t port) const {

    const auto [valAvailable, value] = IExpression::getPreNetworkValue(m_terminal, port);
    if (valAvailable) {
        
        if (BETWEENExpression::Evaluate(value, m_lowerBound, m_upperBound)) {

            return Tristate::TRUE_STATE;
        }

        return Tristate::FALSE_STATE;
    }

    return Tristate::UNKNOWN_STATE;
}


bool ComparisonExpression::Evaluate(ComparisonToken::OpType op, const uint16_t lhs, const uint16_t rhs) {

    switch (op) {
        case ComparisonToken::OP_EQ:
            return lhs == rhs;
        case ComparisonToken::OP_GT:
            return lhs > rhs;
        case ComparisonToken::OP_LT:
            return lhs < rhs;
        case ComparisonToken::OP_GTE:
            return lhs >= rhs;
        case ComparisonToken::OP_LTE:
            return lhs <= rhs;
        case ComparisonToken::OP_NE:
            return lhs != rhs;
        default:
            return false; // need better error handling 
}


Tristate ComparisonExpression::attemptPreNetworkEval(const uint16_t port) const {

    const auto [LHSAvailable, LHSValue] = IExpression::getPreNetworkValue(m_LHSTerminal, port);
    if (LHSAvailable) {

        const auto [RHSAvailable, RHSvalue] = IExpression::getPreNetworkValue(m_RHSTerminal, port);
        if (RHSAvailable) {

            return ComparisonExpression::Evaluate(m_op, LHSValue, RHSvalue) ? Tristate:TRUE_STATE : Tristate::FALSE_STATE;
        }
    }

    return Tristate::UNKNOWN_STATE;
}


