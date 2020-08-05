#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <tuple>

#include "Lexer.h"
#include "Network.h"
#include "Environment.h"


namespace PortQuery {

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
        virtual NetworkProtocols collectRequiredProtocols(void) const = 0;
        virtual ~IExpression() = default;
    };

    using SOSQLExpression = std::unique_ptr<IExpression>;

    template <typename T> struct ITerminal {

        virtual T getTerminalValue(EnvironmentPtr env) = 0;
        virtual std::pair<bool, T> getPreNetworkValue(const EnvironmentPtr env) = 0;
        virtual ~ITerminal() = default;
    };

    struct NumeralTerminal : ITerminal<uint16_t> {

    };

    struct PortTerminal : ITerminal<uint16_t> {

    };

/*
    struct QUERYRESULTTerminal : IExpression< {

    };

    struct NUMERALTerminal : IExpression {


    };

    struct PORTTerminal : IExpression {

    };

    struct PROTOCOLTerminal : IExpression {

    };

    */

    struct ORExpression : IExpression {

        ORExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

            return m_left->attemptPreNetworkEval(port) || m_right->attemptPreNetworkEval(port);
        }

        virtual NetworkProtocols collectRequiredProtocols(void) const override {

            return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
        }

        SOSQLExpression m_left;
        SOSQLExpression m_right;
    };

    struct ANDExpression : IExpression {

        ANDExpression(SOSQLExpression left, SOSQLExpression right) : m_left(std::move(left)), m_right(std::move(right)) { }

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

            return m_left->attemptPreNetworkEval(port) && m_right->attemptPreNetworkEval(port);
        }

        virtual NetworkProtocols collectRequiredProtocols(void) const override {

            return m_left->collectRequiredProtocols() | m_right->collectRequiredProtocols();
        }


        SOSQLExpression m_left;
        SOSQLExpression m_right;
    };

    struct NOTExpression : IExpression {

        NOTExpression(SOSQLExpression expr) : m_expr(std::move(expr)) { }
        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

            return !m_expr->attemptPreNetworkEval(port);
        }

        virtual NetworkProtocols collectRequiredProtocols(void) const override {

            return m_expr->collectRequiredProtocols();
        }


        SOSQLExpression m_expr;
    };


    struct BETWEENExpression : IExpression {

        BETWEENExpression(const uint16_t lowerBound, const uint16_t upperBound, const Token terminal);
        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocols collectRequiredProtocols(void) const override;

        static bool Evaluate(const uint16_t value, const uint16_t lowerBound, const uint16_t upperBound) {

            return lowerBound <= value && value <= upperBound;
        }

        uint16_t m_lowerBound;
        uint16_t m_upperBound;

        SOSQLExpression m_terminal;
    };

    struct ComparisonExpression : IExpression {

        ComparisonExpression(const ComparisonToken::OpType op, const Token lhs, const Token rhs);
        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override;
        virtual NetworkProtocols collectRequiredProtocols(void) const override;

        static bool Evaluate(ComparisonToken::OpType, const uint16_t lhs, const uint16_t rhs);

        ComparisonToken::OpType m_op;
        SOSQLExpression m_LHSTerminal;
        SOSQLExpression m_RHSTerminal;
    };

    struct NULLExpression : IExpression {

        virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

            return Tristate::TRUE_STATE;
        }


        virtual NetworkProtocols collectRequiredProtocols(void) const override {

             return NetworkProtocols::NONE;
        }

    };


    class SelectSet {

        public:

            SelectSet(const std::initializer_list<ColumnToken::Column> columns) : m_selectedColumns(columns) { }

            bool operator==(const std::vector<ColumnToken::Column> other) const {

                return m_selectedColumns == other;
            }

            friend bool operator==(const std::vector<ColumnToken::Column>& lhs, const SelectSet rhs) {

                return rhs == lhs;
            }

            void addColumn(const ColumnToken::Column c) {

                m_selectedColumns.push_back(c);
            }

        private:

            std::vector<ColumnToken::Column> m_selectedColumns;
    };

    class SelectStatement : IExpression {
        public:
            SelectStatement(SelectSet selectedSet, std::string tableReference, SOSQLExpression tableExpression) : 
                m_selectedSet(std::move(selectedSet)), m_tableReference(std::move(tableReference)), 
                m_tableExpression(std::move(tableExpression)) { }


            virtual NetworkProtocols collectRequiredProtocols(void) const override {

                return NetworkProtocols::NONE;

            }
        
            virtual Tristate attemptPreNetworkEval(const uint16_t port) const override {

                return m_tableExpression->attemptPreNetworkEval(port);
            }

            SelectSet getSelectSet() const {

                return m_selectedSet;
            }

            virtual ~SelectStatement() = default;
            SelectStatement(SelectStatement&&) = default;
            SelectStatement &operator=(SelectStatement&&) = default;


        private:

            SelectSet m_selectedSet;
            std::string m_tableReference;
            SOSQLExpression m_tableExpression;
    };

    using SOSQLSelectStatement = std::unique_ptr<SelectStatement>;
}
