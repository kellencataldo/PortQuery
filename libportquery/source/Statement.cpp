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

    template <typename T> std::pair<bool, T> ITerminal<T>::getPreNetworkValue(EnvironmentPtr env) {

        return { true, getValue(env) };
    }

    // Terminals
    uint16_t NumericTerminal::getValue(EnvironmentPtr env) {

        return m_value;
    }

   uint16_t PortTerminal::getValue(EnvironmentPtr env) {

       return env->getPort();
   }

   PQ_QUERY_RESULT QueryResultTerminal::getValue(EnvironmentPtr env) {

       return m_queryResult;
   }

   PQ_QUERY_RESULT ProtocolTerminal::getValue(EnvironmentPtr env) {

       // this will need to be fixed please;
       return PQ_QUERY_RESULT::CLOSED;

   }
    
   std::pair<bool, PQ_QUERY_RESULT> ProtocolTerminal::getPreNetworkValue(EnvironmentPtr env) {

       return { false, PQ_QUERY_RESULT::CLOSED };
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

    Tristate ORExpression::attemptPreNetworkEval(EnvironmentPtr env) const {

        return m_left->attemptPreNetworkEval(env) || m_right->attemptPreNetworkEval(env);
    }

    NetworkProtocol ORExpression::collectRequiredProtocols(void) const {

        return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
    }


    // AND EXPRESSION

    Tristate ANDExpression::attemptPreNetworkEval(EnvironmentPtr env) const {

        return m_left->attemptPreNetworkEval(env) && m_right->attemptPreNetworkEval(env);
    }

    NetworkProtocol ANDExpression::collectRequiredProtocols(void) const {

        return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
    }


    // NOTExpression

   Tristate NOTExpression::attemptPreNetworkEval(EnvironmentPtr env) const {

        return !m_expr->attemptPreNetworkEval(env);
    }

   NetworkProtocol NOTExpression::collectRequiredProtocols(void) const {

        return m_expr->collectRequiredProtocols();
   }


   template <typename T, typename E> bool Evaluate(const ComparisonToken::OpType op, const T lhs, const E rhs) { 

       throw std::invalid_argument("Unable to compare mismatched terminal types"); // probably better message should be printed here.
   }

   template <typename T> bool Evaluate(const ComparisonToken::OpType op, const T lhs, const T rhs) {

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

   // Between expression

   Tristate BETWEENExpression::attemptPreNetworkEval(EnvironmentPtr env) const {

       return std::visit( [env] (auto&& lowerBound, auto&& upperBound, auto&& terminal) -> Tristate { 
                const auto [valAvailable, val] = terminal->getPreNetworkValue(env);
                if (valAvailable) {
                    const auto lowerVal = lowerBound->getValue(env);
                    const auto upperVal = upperBound->getValue(env);
                    return Evaluate(ComparisonToken::OP_GTE, val, lowerVal) && 
                        Evaluate(ComparisonToken::OP_LTE, val, upperVal) ? Tristate::TRUE_STATE : Tristate::FALSE_STATE;
                }

                return Tristate::UNKNOWN_STATE;
            },
        m_lowerBound, m_upperBound, m_terminal);
   }


   NetworkProtocol BETWEENExpression::collectRequiredProtocols(void) const {

       return std::visit( [] (auto&& terminal) { return terminal->collectRequiredProtocols(); }, m_terminal);
   }

   // Comparison expression

   Tristate ComparisonExpression::attemptPreNetworkEval(EnvironmentPtr env) const {

       return std::visit( [&] (auto&& lhs, auto&& rhs) -> Tristate { 
                const auto [lhsAvailable, lhsVal] = lhs->getPreNetworkValue(env);
                const auto [rhsAvailable, rhsVal] = rhs->getPreNetworkValue(env);
                if (lhsAvailable && rhsAvailable) {
                    return Evaluate(m_op, lhsVal, rhsVal) ? Tristate::TRUE_STATE : Tristate::FALSE_STATE;
                }

                return Tristate::UNKNOWN_STATE;
            },

        m_LHSTerminal, m_RHSTerminal);
 
   }

   NetworkProtocol ComparisonExpression::collectRequiredProtocols(void) const {

       return std::visit([](auto&& lhs, auto&& rhs) { 
               return lhs->collectRequiredProtocols() | rhs->collectRequiredProtocols(); 
            }, m_LHSTerminal, m_RHSTerminal);
   }


   // NULL EXPRESSION

   Tristate NULLExpression::attemptPreNetworkEval(EnvironmentPtr env) const {

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

    
    Tristate SelectStatement::attemptPreNetworkEval(EnvironmentPtr env) const {

        return m_tableExpression->attemptPreNetworkEval(env);
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
}
