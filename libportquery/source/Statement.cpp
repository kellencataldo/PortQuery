#include "Statement.h"
#include "Parser.h"


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

    template <typename T> bool performCompare(const ComparisonToken::OpType op, const T lhs, const T rhs) {

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

    uint16_t NumericTerminal::getValue(EnvironmentPtr env) { 

        return m_value;
    }

    bool NumericTerminal::compareValue(const ComparisonToken::OpType op, const uint16_t other, EnvironmentPtr) {

        return performCompare(op, m_value, other);
    }

    bool NumericTerminal::preNetworkAvailable(void) const { 

        return true;
    }

    uint16_t PortTerminal::getValue(EnvironmentPtr env) { 

        return env->getPort();
    }

    bool PortTerminal::compareValue(const ComparisonToken::OpType op, const uint16_t other, EnvironmentPtr env) { 

        return performCompare(op, env->getPort(), other);
    }

    bool PortTerminal::preNetworkAvailable(void) const { 

        return true;
    }

    PQ_QUERY_RESULT QueryResultTerminal::getValue(EnvironmentPtr) { 

        return m_queryResult;
    }

    bool QueryResultTerminal::compareValue(const ComparisonToken::OpType op, const PQ_QUERY_RESULT other, EnvironmentPtr) {

        return performCompare(op, m_queryResult, other);
    }

    bool QueryResultTerminal::preNetworkAvailable(void) const { 

        return false;
    }


    PQ_QUERY_RESULT ProtocolTerminal::getValue(EnvironmentPtr env) { 

        // fix interface here.
        return PQ_QUERY_RESULT::CLOSED;
    }

    bool ProtocolTerminal::compareValue(const ComparisonToken::OpType op, const PQ_QUERY_RESULT other, EnvironmentPtr env) {

        // fix interface here.
        return false;
    }

    bool ProtocolTerminal::preNetworkAvailable(void) const { 

        return false;
    }

    template<typename C, typename R, typename... Args> struct MethodTraits {
        using ClassType = C;
        using ReturnType = R;
        using ArgumentTypes = std::tuple<Args...>;
    };

    template<typename C, typename R, typename... Args> MethodTraits<C, R, Args...> getMethodTraits(R(C::*)(Args...)) { 

        return { }; 
    }

    template <typename L, typename R> constexpr auto matchComparisonTraits() -> bool {

        using ArgumentList = typename decltype(getMethodTraits(&L::compareValue))::ArgumentTypes;
        static_assert(std::tuple_size<ArgumentList>::value == 3, "Argument required to compare");
        using RequiredArgument = typename std::tuple_element<1, ArgumentList>::type;
        using ProvidedArgument = typename decltype(getMethodTraits(&R::getValue))::ReturnType;
        return std::is_same_v<RequiredArgument, ProvidedArgument>;
    }

    template <typename L, typename R> constexpr auto isValidComparison(int) -> decltype(std::declval<L>().compareValue(
                std::declval<ComparisonToken::OpType>(),
                std::declval<R>().getValue(std::declval<EnvironmentPtr>()), 
                std::declval<EnvironmentPtr>()),
            matchComparisonTraits<L,R>()) {

        return matchComparisonTraits<L,R>();
    }

    template <typename L, typename R> constexpr auto isValidComparison(...) {

        return std::false_type();
    }

    template <class L, class R> auto compare(const ComparisonToken::OpType op, L& lhs, R& rhs, EnvironmentPtr env) -> 
        typename std::enable_if<isValidComparison<L, R>(int()), bool>::type {
        return lhs.compareValue(op, rhs.getValue(env), env);
    }

    template <class L, class R> auto compare(const ComparisonToken::OpType op, L& lhs, R& rhs, EnvironmentPtr env) -> 
        typename std::enable_if<!isValidComparison<L, R>(int()), bool>::type {
        // do more stuff here.
        throw std::invalid_argument("Unable to perform comparison on provided types");
    }

    SOSQLTerminal getTerminalFromToken(const Token t) {

//        return NumericTerminal{4 };

        return std::visit(overloaded {
                    [] (const NumericToken n) -> SOSQLTerminal { return NumericTerminal{n.m_value}; },
                    [] (const QueryResultToken q) -> SOSQLTerminal { return QueryResultTerminal{q.m_queryResult}; },
                    [] (const ColumnToken c) -> SOSQLTerminal {
                        switch (c.m_column) {
                            case ColumnToken::PORT:
                                return PortTerminal{};
                            case ColumnToken::TCP:
                                return ProtocolTerminal{NetworkProtocol::TCP};
                            case ColumnToken::UDP:
                                return ProtocolTerminal{NetworkProtocol::UDP};
                            default:
                                throw std::invalid_argument("Unable to convert unknown column token to terminal" + getExtendedTokenInfo(c));
                        }
                    }, 
                    [] (const auto t) -> SOSQLTerminal { 
                        throw std::invalid_argument("Unable to convert unknown token to terminal" + getTokenString(t));
                    }, },
                t);
 //               */
        }

    // OR EXPRESSION

    Tristate ORExpression::attemptPreNetworkEval(EnvironmentPtr env) {

        return m_left->attemptPreNetworkEval(env) || m_right->attemptPreNetworkEval(env);
    }

    NetworkProtocol ORExpression::collectRequiredProtocols(void) const {

        return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
    }


    // AND EXPRESSION

    Tristate ANDExpression::attemptPreNetworkEval(EnvironmentPtr env) {

        return m_left->attemptPreNetworkEval(env) && m_right->attemptPreNetworkEval(env);
    }

    NetworkProtocol ANDExpression::collectRequiredProtocols(void) const {

        return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
    }


    // NOTExpression

   Tristate NOTExpression::attemptPreNetworkEval(EnvironmentPtr env) {

        return !m_expr->attemptPreNetworkEval(env);
    }

   NetworkProtocol NOTExpression::collectRequiredProtocols(void) const {

        return m_expr->collectRequiredProtocols();
   }


    NetworkProtocol getProtocolFromTerminal(const SOSQLTerminal t) {

        return std::visit(overloaded {
                [] (const ProtocolTerminal p) { return p.m_protocol; },
                [] (auto) { return NetworkProtocol::NONE; }, 
            }, t);
    }

   Tristate BETWEENExpression::attemptPreNetworkEval(EnvironmentPtr env) {

       return std::visit( [env] (auto&& lowerBound, auto&& upperBound, auto&& terminal) -> Tristate { 

               if (terminal.preNetworkAvailable()) {
                    return compare(ComparisonToken::OP_GTE, terminal, lowerBound, env) && 
                        compare(ComparisonToken::OP_LTE, terminal, upperBound, env) ? 
                        Tristate::TRUE_STATE : Tristate::FALSE_STATE;
                }

                return Tristate::UNKNOWN_STATE;
            },

        m_lowerBound, m_upperBound, m_terminal);
   }


   NetworkProtocol BETWEENExpression::collectRequiredProtocols(void) const {

       return getProtocolFromTerminal(m_terminal);
   }

   // Comparison expression

   Tristate ComparisonExpression::attemptPreNetworkEval(EnvironmentPtr env) {

       return std::visit( [&] (auto&& lhs, auto&& rhs) -> Tristate { 
                if (lhs.preNetworkAvailable() && rhs.preNetworkAvailable()) {
                    return compare(m_op, lhs, rhs, env) ? Tristate::TRUE_STATE : Tristate::FALSE_STATE;
                }

                return Tristate::UNKNOWN_STATE;
            },

        m_LHSTerminal, m_RHSTerminal);
 
   }

   NetworkProtocol ComparisonExpression::collectRequiredProtocols(void) const {

       return getProtocolFromTerminal(m_LHSTerminal) | getProtocolFromTerminal(m_RHSTerminal);
   }

   // NULL EXPRESSION

   Tristate NULLExpression::attemptPreNetworkEval(EnvironmentPtr env) {

       return Tristate::TRUE_STATE;
   }

   NetworkProtocol NULLExpression::collectRequiredProtocols(void) const {

       return NetworkProtocol::NONE;
   }


   void SelectSet::addColumn(const ColumnToken::Column c) {

       m_selectedColumns.push_back(c);
   }

   SelectSet::ColumnVector SelectSet::getSelectedColumns(void) const {
       
       return m_selectedColumns;
   }

   SelectSet::ColumnVector::const_iterator SelectSet::begin() const {

       return m_selectedColumns.begin();
   }

   SelectSet::ColumnVector::const_iterator SelectSet::end() const {

       return m_selectedColumns.end();
   }

   bool SelectSet::operator==(const std::vector<ColumnToken::Column>& other) const {

       return m_selectedColumns == other;
   }

   bool SelectSet::operator==(const SelectSet& other) const {

       return m_selectedColumns == other.getSelectedColumns();
   }
   
   bool operator==(const std::vector<ColumnToken::Column>& lhs, const SelectSet rhs) {

       return rhs == lhs;
   }



    // SELECT STATEMENT

    SelectSet SelectStatement::getSelectSet() const {

        return m_selectedSet;
    }

    
    Tristate SelectStatement::attemptPreNetworkEval(EnvironmentPtr env) {

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
