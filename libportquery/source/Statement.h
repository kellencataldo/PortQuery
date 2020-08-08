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

    Tristate operator||(const Tristate lhs, const Tristate rhs);
    Tristate operator&&(const Tristate lhs, const Tristate rhs);
    Tristate operator!(const Tristate rhs);

    template <typename T> struct ITerminal;
    using SOSQLTypeNumeric = std::unique_ptr<ITerminal<uint16_t>>;
    using SOSQLTypeQuery = std::unique_ptr<ITerminal<PQ_QUERY_RESULT>>;
    using SOSQLTerminal = std::variant<SOSQLTypeNumeric, SOSQLTypeQuery>;

    struct IExpression;
    using SOSQLExpression = std::unique_ptr<IExpression>;

    class SelectStatement;
    using SOSQLSelectStatement = std::unique_ptr<SelectStatement>;

   template <typename T> struct ITerminal {

       virtual T getTerminalValue(EnvironmentPtr env) = 0;
       virtual std::pair<bool, T> getPreNetworkValue(const EnvironmentPtr env) = 0;

       virtual NetworkProtocol collectRequiredProtocols(void) const; 

       virtual ~ITerminal() = default;
    };

    struct NumericTerminal : ITerminal<uint16_t> {

        NumericTerminal(const uint16_t value) : m_value(value) { }

        virtual uint16_t getTerminalValue(EnvironmentPtr env) override;
        virtual std::pair<bool, uint16_t> getPreNetworkValue(const EnvironmentPtr env) override;

        uint16_t m_value;
    };

    struct PortTerminal : ITerminal<uint16_t> {

        PortTerminal() = default;

        virtual uint16_t getTerminalValue(EnvironmentPtr env) override;
        virtual std::pair<bool, uint16_t> getPreNetworkValue(const EnvironmentPtr env) override;

    };

    struct QueryResultTerminal : ITerminal<PQ_QUERY_RESULT> {

        QueryResultTerminal(const PQ_QUERY_RESULT queryResult) : m_queryResult(queryResult) { }

        virtual PQ_QUERY_RESULT getTerminalValue(EnvironmentPtr env) override;
        virtual std::pair<bool, PQ_QUERY_RESULT> getPreNetworkValue(const EnvironmentPtr env) override;

        PQ_QUERY_RESULT m_queryResult;
    };

    struct ProtocolTerminal : ITerminal<PQ_QUERY_RESULT> {

        ProtocolTerminal(const NetworkProtocol protocol) : m_protocol(protocol) { }

        virtual PQ_QUERY_RESULT getTerminalValue(EnvironmentPtr env) override;
        virtual std::pair<bool, PQ_QUERY_RESULT> getPreNetworkValue(const EnvironmentPtr env) override;

        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        NetworkProtocol m_protocol;
    };


    SOSQLTerminal getTerminalFromToken(const Token t);

    struct IExpression {

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const = 0;
        virtual NetworkProtocol collectRequiredProtocols(void) const = 0;

        virtual ~IExpression() = default;
    };


    struct ORExpression : IExpression {

        ORExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;
        SOSQLExpression m_left;
        SOSQLExpression m_right;
    };


    struct ANDExpression : IExpression {

        ANDExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        SOSQLExpression m_left;
        SOSQLExpression m_right;
    };


    struct NOTExpression : IExpression {

        NOTExpression(SOSQLExpression expr) : m_expr(std::move(expr)) { }
        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        SOSQLExpression m_expr;
    };


    struct BETWEENExpression : IExpression {

        BETWEENExpression(const uint16_t lowerBound, const uint16_t upperBound, const Token t) :
            m_lowerBound(lowerBound), m_upperBound(upperBound), m_terminal(getTerminalFromToken(t)) { }

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        uint16_t m_lowerBound;
        uint16_t m_upperBound;

        SOSQLTerminal m_terminal;
    };

    struct ComparisonExpression : IExpression {

        ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs) : m_op(op), 
            m_LHSTerminal(getTerminalFromToken(lhs)), m_RHSTerminal(getTerminalFromToken(rhs)) { }
        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocol collectRequiredProtocols(void) const override;

        static bool Evaluate(ComparisonToken::OpType, const uint16_t lhs, const uint16_t rhs);

        ComparisonToken::OpType m_op;
        SOSQLTerminal m_LHSTerminal;
        SOSQLTerminal m_RHSTerminal;
    };

    struct NULLExpression : IExpression {

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
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
        
            virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;

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
