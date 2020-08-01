#include "Statement.h"


namespace PortQuery {

    Tristate operator||(const Tristate lhs, const Tristate rhs) {
        typedef typename std::underlying_type<Tristate>::type underlying;
        return static_cast<Tristate>(std::max(static_cast<underlying>(lhs), static_cast<underlying>(rhs)));
    }

    Tristate operator&&(const Tristate lhs, const Tristate rhs) {
        typedef typename std::underlying_type<Tristate>::type underlying;
        return static_cast<Tristate>(std::min(static_cast<underlying>(lhs), static_cast<underlying>(rhs)));
    }

    Tristate operator!(const Tristate rhs) {
        typedef typename std::underlying_type<Tristate>::type underlying;
        return static_cast<Tristate>(-static_cast<underlying>(rhs)); 
    }


    SOSQLExpression createTerminalFromToken(const Token t) {


    }


    BETWEENExpression::BETWEENExpression(const uint16_t lowerBound, const uint16_t upperBound, const Token terminal) :
        m_lowerBound(lowerBound), m_upperBound(upperBound) {

            m_


    }

    ComparisonExpression::ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) : m_op(op) {

    }
  
  

/*
    std::tuple<bool, uint16_t> getPreNetworkValue(const Terminal terminal, const uint16_t port) {

        auto t = std::visit(overloaded {
                   // [=] (QueryResultToken q) { return std::make_tuple(true, q.m_queryResult); },
                   // [=] (NumericToken n)     { return std::make_tuple(true, n.m_value); },
                    [=] (ColumnToken c)      { return (ColumnToken::PORT == c.m_column) ? std::make_tuple(true, port) :
                        std::make_tuple<bool, uint16_t>(false, 0);
                    } },
                terminal);

        return t;
    }


    NetworkProtocols getProtocolFromColumn(const ColumnToken::Column c) {

        switch (c) {
            case ColumnToken::TCP:
                return NetworkProtocols::TCP;
            case ColumnToken::UDP:
                return NetworkProtocols::UDP;
            default:
                return NetworkProtocols::NONE;
        }
    }


    NetworkProtocols getNetworkProtocolFromToken(const Terminal terminal) {

        auto t = std::visit(overloaded {
                    [=] (QueryResultToken q) { return NetworkProtocols::NONE; },
                    [=] (NumericToken n)     { return NetworkProtocols::NONE; },
                    [=] (ColumnToken c)      { return getProtocolFromColumn(c.m_column);
                    } },
                terminal);

        return t;
    }


    NetworkProtocols BETWEENExpression::collectRequiredProtocols() const {

        return getNetworkProtocolFromToken(m_terminal);
    }


    Tristate BETWEENExpression::attemptPreNetworkEval(const uint16_t port) const {

        const auto [valAvailable, value] = getPreNetworkValue(m_terminal, port);
        if (valAvailable) {

            return BETWEENExpression::Evaluate(value, m_lowerBound, m_upperBound) ? Tristate::TRUE_STATE : Tristate::FALSE_STATE;
        }

        return Tristate::UNKNOWN_STATE;
    }


    NetworkProtocols ComparisonExpression::collectRequiredProtocols() const {

        return getNetworkProtocolFromToken(m_LHSTerminal) | getNetworkProtocolFromToken(m_RHSTerminal);
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
    }


    Tristate ComparisonExpression::attemptPreNetworkEval(const uint16_t port) const {

        const auto [LHSAvailable, LHSValue] = getPreNetworkValue(m_LHSTerminal, port);
        if (LHSAvailable) {

            const auto [RHSAvailable, RHSvalue] = getPreNetworkValue(m_RHSTerminal, port);
            if (RHSAvailable) {

                return ComparisonExpression::Evaluate(m_op, LHSValue, RHSvalue) ? Tristate::TRUE_STATE : Tristate::FALSE_STATE;
            }
        }

        return Tristate::UNKNOWN_STATE;
    }


    NetworkProtocols SelectStatement::collectRequiredProtocols() const {

        NetworkProtocols requestedProtocols = NetworkProtocols::NONE;
        for (auto i : m_selectedSet) {
            requestedProtocols |= getProtocolFromColumn(i);
        }

        requestedProtocols |= m_tableExpression->collectRequiredProtocols();
        return requestedProtocols;
    }
    */

}
