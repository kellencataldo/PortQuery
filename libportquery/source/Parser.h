#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "Lexer.h"
#include "Network.h"


template <typename T> void UNUSED_PARAMETER(T &&) { };

struct Terminal{ };
//
struct BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const = 0;
};

using SOSQLExpression = std::shared_ptr<BaseExpression>;

struct ORExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return left->shouldSubmitForScan(port) || right->shouldSubmitForScan(port);
    }

    SOSQLExpression left;
    SOSQLExpression right;
};

struct ANDExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return left->shouldSubmitForScan(port) && right->shouldSubmitForScan(port);
    }

    SOSQLExpression left;
    SOSQLExpression right;
};

struct BETWEENExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        return (port >= m_lowerBound) && (port <= m_upperBound);
    }

    uint16_t m_lowerBound;
    uint16_t m_upperBound;
};

struct ComparisonExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const;

    Terminal m_lhs;
    Terminal m_rhs; 
};

struct NULLExpression : BaseExpression {

    virtual bool shouldSubmitForScan(const uint16_t port) const {
        UNUSED_PARAMETER(port);
        return true;
    }
};

/*
using SOSQLExpression = std::variant<
    std::shared_ptr<ORExpression>,
    std::shared_ptr<ANDExpression>,
    std::shared_ptr<BETWEENExpression>,
    std::shared_ptr<ComparisonExpression>,
    // special case, no WHERE clause
    std::shared_ptr<NULLExpression>
    >;
*/


struct SelectStatement {

    bool m_selectPort;

    NetworkProtocols m_selectedProtocols;

    std::string m_tableReference;

    SOSQLExpression m_tableExpression;
};

typedef SelectStatement SOSQLSelectStatement;

class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // SELECT Only SQL
        // maybe make this a unique_ptr? investigation required.
        SOSQLSelectStatement parseSOSQLStatement();


    private:

        std::tuple<bool, NetworkProtocols> parseSelectSetQuantifier();
        std::tuple<bool, NetworkProtocols> parseSelectList();

        std::string parseTableReference();

        SOSQLExpression parseTableExpression();
        SOSQLExpression parseORExpression();
        SOSQLExpression parseANDExpression();

        Lexer m_lexer;
};
