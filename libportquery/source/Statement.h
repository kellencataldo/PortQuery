#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <tuple>

#include "Lexer.h"
#include "Network.h"
#include "Environment.h"
#include "PortQuery.h"


namespace PortQuery {

    enum class Tristate : int {
        FALSE_STATE = -1,
        UNKNOWN_STATE = 0,
        TRUE_STATE = 1,
    };

    struct NumericTerminal;
    struct PortTerminal;
    struct QueryResultTerminal;
    struct ProtocolTerminal;
    struct IExpression;
    class SelectStatement;

    using SOSQLTerminal = std::variant<NumericTerminal, PortTerminal, QueryResultTerminal, ProtocolTerminal>;
    using SOSQLExpression = std::unique_ptr<IExpression>;
    using SOSQLSelectStatement = std::unique_ptr<SelectStatement>;

    Tristate operator||(const Tristate lhs, const Tristate rhs);
    Tristate operator&&(const Tristate lhs, const Tristate rhs);
    Tristate operator!(const Tristate rhs);

    SOSQLTerminal getTerminalFromToken(const Token t);

    struct NumericTerminal {

        NumericTerminal(const uint16_t value) : m_value(value) { }
        uint16_t getValue(EnvironmentPtr);
        bool compareValue(const ComparisonToken::OpType op, const uint16_t other, EnvironmentPtr);
        bool preNetworkAvailable(void) const;

        uint16_t m_value;
    };

    struct PortTerminal {

        uint16_t getValue(EnvironmentPtr env);
        bool compareValue(const ComparisonToken::OpType op, const uint16_t other, EnvironmentPtr env);
        bool preNetworkAvailable() const;
    };

    struct QueryResultTerminal {

        QueryResultTerminal(const PQ_QUERY_RESULT queryResult) : m_queryResult(queryResult) { }

        PQ_QUERY_RESULT getValue(EnvironmentPtr);
        bool compareValue(const ComparisonToken::OpType op, const PQ_QUERY_RESULT other, EnvironmentPtr);
        bool preNetworkAvailable(void) const;

        PQ_QUERY_RESULT m_queryResult;
    };

    struct ProtocolTerminal {

        ProtocolTerminal(const NetworkProtocol protocol) : m_protocol(protocol) { }

        PQ_QUERY_RESULT getValue(EnvironmentPtr env);
        bool compareValue(const ComparisonToken::OpType op, const PQ_QUERY_RESULT other, EnvironmentPtr env);
        bool preNetworkAvailable(void) const;

        NetworkProtocol m_protocol;
    };


    struct IExpression {

        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) = 0;
        virtual NetworkProtocol collectRequiredProtocols(void) const = 0;

        virtual ~IExpression() = default;
    };


    struct ORExpression : IExpression {

        ORExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;
        SOSQLExpression m_left;
        SOSQLExpression m_right;
    };


    struct ANDExpression : IExpression {

        ANDExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        SOSQLExpression m_left;
        SOSQLExpression m_right;
    };


    struct NOTExpression : IExpression {

        NOTExpression(SOSQLExpression expr) : m_expr(std::move(expr)) { }
        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        SOSQLExpression m_expr;
    };


    struct BETWEENExpression : IExpression {

        BETWEENExpression(const uint16_t lowerBound, const uint16_t upperBound, const Token t) :
            m_lowerBound(NumericTerminal{lowerBound}), 
            m_upperBound(NumericTerminal{upperBound}), 
            m_terminal(getTerminalFromToken(t)) { }

        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        SOSQLTerminal m_lowerBound;
        SOSQLTerminal m_upperBound;

        SOSQLTerminal m_terminal;
    };

    struct ComparisonExpression : IExpression {

        ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) : m_op(op), 
            m_LHSTerminal(getTerminalFromToken(lhs)), m_RHSTerminal(getTerminalFromToken(rhs)) { }
        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        ComparisonToken::OpType m_op;
        SOSQLTerminal m_LHSTerminal;
        SOSQLTerminal m_RHSTerminal;
    };

    struct NULLExpression : IExpression {

        virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;
    };


    class SelectSet {

        public:

            using ColumnVector = std::vector<ColumnToken::Column>;

            SelectSet(const std::initializer_list<ColumnToken::Column> columns) : m_selectedColumns(columns) { }

            void addColumn(const ColumnToken::Column c);

            ColumnVector::const_iterator begin() const;
            ColumnVector::const_iterator end() const;

            bool operator==(const std::vector<ColumnToken::Column> other) const;
            friend bool operator==(const std::vector<ColumnToken::Column>& lhs, const SelectSet rhs);

        private:

            ColumnVector m_selectedColumns;
    };

    class SelectStatement : IExpression {
        public:
            SelectStatement(SelectSet selectedSet, std::string tableReference, SOSQLExpression tableExpression) : 
                m_selectedSet(std::move(selectedSet)), m_tableReference(std::move(tableReference)), 
                m_tableExpression(std::move(tableExpression)) { }


            virtual NetworkProtocol collectRequiredProtocols(void) const override;
        
            virtual Tristate attemptPreNetworkEval(EnvironmentPtr env) override;

            SelectSet getSelectSet() const;

            virtual ~SelectStatement() = default;
            SelectStatement(SelectStatement&&) = default;
            SelectStatement &operator=(SelectStatement&&) = default;


        private:

            SelectSet m_selectedSet;
            std::string m_tableReference;
            SOSQLExpression m_tableExpression;
    };
}
