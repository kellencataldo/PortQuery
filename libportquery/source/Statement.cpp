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

    template <typename T> NetworkProtocol ITerminal<T>::collectRequiredProtocols(void) const {

        return NetworkProtocol::NONE;
    }

    NetworkProtocol ProtocolTerminal::collectRequiredProtocols(void) const {

        return m_protocol;
    }


    SOSQLTerminal getTerminalFromToken(const Token t) {

        if (MATCH<NumericToken>(t)) {

            return std::move(std::make_unique<NumericTerminal>(std::get<NumericToken>(t).m_value));
        }

        else if (MATCH_COLUMN<ColumnToken::PORT>(t)) {

            return std::move(std::make_unique<PortTerminal>());
        }

        else if (MATCH<QueryResultToken>(t)) {

            return std::move(std::make_unique<QueryResultTerminal>(std::get<QueryResultToken>(t).m_queryResult));
        }

        else if (MATCH_COLUMN<ColumnToken::TCP, ColumnToken::UDP>(t)) {

            NetworkProtocol p = NetworkProtocol::NONE;
            switch (std::get<ColumnToken>(t).m_column) {
                case ColumnToken::TCP:
                    p = NetworkProtocol::TCP;
                case ColumnToken::UDP:
                    p = NetworkProtocol::UDP;
                default:
                    p = NetworkProtocol::NONE; // better error handling here.
            }

            return std::move(std::make_unique<ProtocolTerminal>(p));
        }

        throw std::invalid_argument("Unable to construct terminal from non terminal token");
    }


    // OR EXPRESSION

    Tristate ORExpression::attemptPreNetworkEval(const uint16_t port) const {

        return m_left->attemptPreNetworkEval(port) || m_right->attemptPreNetworkEval(port);
    }

    NetworkProtocol ORExpression::collectRequiredProtocols(void) const {

        return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
    }


    // AND EXPRESSION

    Tristate ANDExpression::attemptPreNetworkEval(const uint16_t port) const {

        return m_left->attemptPreNetworkEval(port) && m_right->attemptPreNetworkEval(port);
    }

    NetworkProtocol ANDExpression::collectRequiredProtocols(void) const {

        return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
    }


    // NOTExpression

   Tristate NOTExpression::attemptPreNetworkEval(const uint16_t port) const {

        return !m_expr->attemptPreNetworkEval(port);
    }

   NetworkProtocol NOTExpression::collectRequiredProtocols(void) const {

        return m_expr->collectRequiredProtocols();
   }


   template <typename T, typename E> bool Evaluate(const T lhs, const E rhs, const ComparisonToken::OpType op, EnvironmentPtr env) {

       throw std::invalid_argument("Unable to compare mismatched terminal types"); // probably better message should be printed here.
   }

   template <typename T> bool Evaluate(const std::unique_ptr<ITerminal<T>> lhs, const std::unique_ptr<ITerminal<T>> rhs, 
           const ComparisonToken::OpType op, EnvironmentPtr env) {


       T LHSValue = lhs->getTerminalValue(env);
       T RHSValue = rhs->getTerminalValue(env);

        switch (op) {
            case ComparisonToken::OP_EQ:
                return LHSValue == RHSValue;
            case ComparisonToken::OP_GT:
                return LHSValue > RHSValue;
            case ComparisonToken::OP_LT:
                return LHSValue < RHSValue;
            case ComparisonToken::OP_GTE:
                return LHSValue >= RHSValue;
            case ComparisonToken::OP_LTE:
                return LHSValue <= RHSValue;
            case ComparisonToken::OP_NE:
                return LHSValue != RHSValue;
            default:
                return false; // need better error handling 
        }
   }



   // Between expression

   Tristate BETWEENExpression::attemptPreNetworkEval(const uint16_t port) const {

       // return std::visit( [] (auto&& terminal) { return terminal->collectRequiredProtocols(); }, m_terminal);
   }


   NetworkProtocol BETWEENExpression::collectRequiredProtocols(void) const {

       return std::visit( [] (auto&& terminal) { return terminal->collectRequiredProtocols(); }, m_terminal);
   }

   // Comparison expression

   Tristate ComparisonExpression::attemptPreNetworkEval(const uint16_t port) const {


   }

   NetworkProtocol ComparisonExpression::collectRequiredProtocols(void) const {

       return std::visit([](auto&& lhs, auto&& rhs) { 
               return lhs->collectRequiredProtocols() | rhs->collectRequiredProtocols(); 
            }, m_LHSTerminal, m_RHSTerminal);
   }


   // NULL EXPRESSION

   Tristate NULLExpression::attemptPreNetworkEval(const uint16_t port) const {

       return Tristate::TRUE_STATE;
   }

   NetworkProtocol NULLExpression::collectRequiredProtocols(void) const {

       return NetworkProtocol::NONE;
   }


   void SelectSet::addColumn(const ColumnToken::Column c) {

       m_selectedColumns.push_back(c);
   }

   SelectSet::ColumnVector::const_iterator SelectSet::begin() const {

       return m_selectedColumns.begin();
   }

   SelectSet::ColumnVector::const_iterator SelectSet::end() const {

       return m_selectedColumns.end();
   }

   bool SelectSet::operator==(const std::vector<ColumnToken::Column> other) const {

       return m_selectedColumns == other;
   }
   
   bool operator==(const std::vector<ColumnToken::Column>& lhs, const SelectSet rhs) {

       return rhs == lhs;
   }



    // SELECT STATEMENT

    SelectSet SelectStatement::getSelectSet() const {

        return m_selectedSet;
    }

    
    Tristate SelectStatement::attemptPreNetworkEval(const uint16_t port) const {

        return m_tableExpression->attemptPreNetworkEval(port);
    }


    NetworkProtocol SelectStatement::collectRequiredProtocols() const {

        NetworkProtocol requestedProtocols = NetworkProtocol::NONE;
        for (auto i : m_selectedSet) {

            switch (i) {

                case ColumnToken::TCP:
                   requestedProtocols |= NetworkProtocol::TCP;
                   break;
                case ColumnToken::UDP:
                   requestedProtocols |= NetworkProtocol::UDP;
                   break;
                default:

                   // just making the ALE warning go away.
                   break;
            }
        }

        requestedProtocols |= m_tableExpression->collectRequiredProtocols();
        return requestedProtocols;
    }
    

/*

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

    */

}
