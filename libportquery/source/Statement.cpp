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

    std::string getTerminalString(const SOSQLTerminal terminal) {

       return std::visit(overloaded {
               [] (PortTerminal) { return "[PORT TERMINAL]"; },
               [] (NumericTerminal) { return "[NUMERIC TERMINAL]"; },
               [] (ProtocolTerminal) { return "[PROTOCOL Terminal]"; },
               [] (QueryResultTerminal) { return "[QUERY RESULT TERMINAL]"; },
               // throw here?
               [] (auto) { return "[UNKNOWN TERMINAL]"; }, 
            },
        terminal);
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

    template <typename L, typename R> constexpr auto isValidComparison(...) -> std::false_type {

        return std::false_type();
    }

    bool canCompareTerminals(SOSQLTerminal lhs, SOSQLTerminal rhs) {

       return std::visit( [] (auto&& l, auto&& r) -> bool { 
               return isValidComparison<std::remove_reference_t<decltype(l)>, std::remove_reference_t<decltype(r)>>(int());
            },

        lhs, rhs);
    }

    template <class L, class R> auto compare(const ComparisonToken::OpType op, L& lhs, R& rhs, EnvironmentPtr env) -> 
        typename std::enable_if<isValidComparison<L, R>(int()), bool>::type {
        return lhs.compareValue(op, rhs.getValue(env), env);
    }

    template <class L, class R> auto compare(const ComparisonToken::OpType op, L& lhs, R& rhs, EnvironmentPtr env) -> 
        typename std::enable_if<!isValidComparison<L, R>(int()), bool>::type {
        const std::string LHS_string = getTerminalString(lhs);
        const std::string RHS_string = getTerminalString(rhs);
        throw std::invalid_argument("Unable to perform comparison on provided types: " + LHS_string + " " + RHS_string);
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

        return true;
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

    SOSQLTerminal getTerminalFromToken(const Token t) {

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
        }

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

   BETWEENExpression::BETWEENExpression(const uint16_t lowerBound, const uint16_t upperBound, const Token t) :
       m_lowerBound(NumericTerminal{lowerBound}), 
       m_upperBound(NumericTerminal{upperBound}), 
       m_terminal(getTerminalFromToken(t)) { 

       if (!canCompareTerminals(m_terminal, m_lowerBound)) {
           const std::string LHS_string = getTerminalString(m_terminal);
           const std::string RHS_string = getTerminalString(m_lowerBound);
           throw std::invalid_argument("Unable to perform comparison on provided types: " + LHS_string + " " + RHS_string);
       }
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
   ComparisonExpression::ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) : 
       m_op(op), m_LHSTerminal(getTerminalFromToken(lhs)), m_RHSTerminal(getTerminalFromToken(rhs)) { 

       if (!canCompareTerminals(m_LHSTerminal, m_RHSTerminal)) {
           const std::string LHS_string = getTerminalString(m_LHSTerminal);
           const std::string RHS_string = getTerminalString(m_RHSTerminal);
           throw std::invalid_argument("Unable to perform comparison on provided types: " + LHS_string + " " + RHS_string);
       }
   }

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


   // SELECT SET
   SelectSet::SelectSet(const std::initializer_list<ColumnToken> columns) { 

       std::transform(columns.begin(), columns.end(), std::back_inserter(m_selectedColumns), getTerminalFromToken); 
   }

   SelectSet::ColumnVector::const_iterator SelectSet::begin() const {

       return m_selectedColumns.begin();
   }

   SelectSet::ColumnVector::const_iterator SelectSet::end() const {

       return m_selectedColumns.end();
   }

   void SelectSet::addColumn(const ColumnToken c) {

       m_selectedColumns.push_back(getTerminalFromToken(c));
   }
    
   PQ_ROW SelectSet::getSelectedColumns(EnvironmentPtr env) {

       // yep, still in hell
       auto compLambda = [env] (SOSQLTerminal s) -> PQ_QUERY_RESULT {
           return std::visit( [env] (auto&& t) ->PQ_QUERY_RESULT { 
               return PQ_QUERY_RESULT{t.getValue(env)};
            }, s); };

       PQ_ROW row { };
       std::transform(m_selectedColumns.begin(), m_selectedColumns.end(), std::back_inserter(row), compLambda);
       return std::move(row);
   }

    // SELECT STATEMENT

    
    Tristate SelectStatement::attemptPreNetworkEval(EnvironmentPtr env) {

        return m_tableExpression->attemptPreNetworkEval(env);
    }

    NetworkProtocol SelectStatement::collectRequiredProtocols() const {

        NetworkProtocol requestedProtocols = NetworkProtocol::NONE;
        for (auto i : m_selectedSet) {

            requestedProtocols |= getProtocolFromTerminal(i);
        }

        requestedProtocols |= m_tableExpression->collectRequiredProtocols();
        return requestedProtocols;
    }
}
