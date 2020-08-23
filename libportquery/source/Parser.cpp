#include <algorithm>

#include "Parser.h"


namespace PortQuery {

    std::string getExtendedTokenInfo(const NumericToken n) {

        return "[NUMERIC TOKEN: " + std::to_string(n.m_value) + "]";
    }


    std::string getExtendedTokenInfo(const UserToken u) {
        
        return "[USER TOKEN: " + u.m_UserToken + "]";
    }


    std::string getExtendedTokenInfo(const ComparisonToken c) {
        std::string prefix{"[COMPARISON TOKEN: "};
        switch (c.m_opType) {
            case ComparisonToken::OP_EQ:
                prefix += "EQ]";
            case ComparisonToken::OP_GT:
                prefix += "GT]";
            case ComparisonToken::OP_LT:
                prefix += "LT]";
            case ComparisonToken::OP_GTE:
                prefix += "GTE]";
            case ComparisonToken::OP_LTE:
                prefix += "LTE]";
            case ComparisonToken::OP_NE:
                prefix += "NE]";
            default:
                prefix += "UNKNOWN COMPARISON TOKEN]";
        }

        return prefix + "]";
    }

    std::string getExtendedTokenInfo(const KeywordToken k) {

        std::string prefix{"[KEYWORD TOKEN: "};
        switch (k.m_keyword) {
            case KeywordToken::ALL:
                prefix += "ALL";
            case KeywordToken::AND:
                prefix += "AND";
            case KeywordToken::BETWEEN: 
                prefix += "BETWEEN";
            case KeywordToken::FROM:
                prefix += "FROM";
            case KeywordToken::IS:
                prefix += "IS";
            case KeywordToken::NOT:
                prefix += "NOT";
            case KeywordToken::OR:
                prefix += "OR";
            case KeywordToken::SELECT:
                prefix += "SELECT";
            case KeywordToken::WHERE:
                prefix += "WHERE";
            default:
                prefix += "UNKNOWN KEYWORD TOKEN";
        }

        return prefix + "]";
    }

    std::string getExtendedTokenInfo(const ColumnToken c) {

        std::string prefix{"[COLUMN TOKEN: "};
        switch (c.m_column) {
            case ColumnToken::PORT:
                prefix += "PORT";
            case ColumnToken::TCP:
                prefix += "TCP";
            case ColumnToken::UDP:
                prefix += "UDP";
            default:
                prefix += "UNKNOWN COLUMN TOKEN";
            }

        return prefix + "]";
    }


    std::string getExtendedTokenInfo(const QueryResultToken q) {
        
        std::string prefix{"PROTOCOL TOKEN: "};
        switch (q.m_queryResult) {
            case PQ_QUERY_RESULT::OPEN:
                prefix += "OPEN";
            case PQ_QUERY_RESULT::CLOSED:
                prefix += "CLOSED";
            case PQ_QUERY_RESULT::REJECTED:
                prefix += "REJECTED";
            default:
                prefix += "UNKOWN QUERY RESULT TOKEN";
        }

        return prefix + "]";
    }


    std::string getTokenString(const Token t) {

        return std::visit(overloaded {
                    [=] (const KeywordToken k)     { return getExtendedTokenInfo(k); },
                    [=] (const ColumnToken c)      { return getExtendedTokenInfo(c); },
                    [=] (const QueryResultToken q) { return getExtendedTokenInfo(q); },
                    [=] (const NumericToken n)     { return getExtendedTokenInfo(n); },
                    [=] (const UserToken u)        { return getExtendedTokenInfo(u); },
                    [=] (const ComparisonToken c)  { return getExtendedTokenInfo(c); },
                    [=] (PunctuationToken<'*'>) { return std::string("[ * ]"); },
                    [=] (PunctuationToken<'('>) { return std::string("[ ( ]"); }, 
                    [=] (PunctuationToken<')'>) { return std::string("[ ) ]"); },
                    [=] (PunctuationToken<';'>) { return std::string("[ ; ]"); }, 
                    [=] (PunctuationToken<','>) { return std::string("[ , ]"); },
                    [=] (EOFToken)        { return std::string("[END OF INPUT]"); },
                    [=] (auto) -> std::string { return std::string("[UNKNOWN TOKEN]"); } }, 
                t);
        }


    SOSQLSelectStatement Parser::parseSOSQLStatement() {

        // SOSQL statements can obviously only begin with the "SELECT" keyword
        if (!MATCH_KEYWORD<KeywordToken::SELECT>(m_lexer.nextToken())) {

            throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
        }


        SelectSet selectedSet = parseSelectSetQuantifier();
        std::string tableReference = parseTableReference();
        SOSQLExpression tableExpression = parseTableExpression();

        // parse end here, check for EOF and semicolon
        // parse where statement here.

        if (MATCH<PunctuationToken<';'>>(m_lexer.peek())) {

            m_lexer.nextToken();
        }

        const Token t = m_lexer.peek();
        if(!MATCH<EOFToken>(t)) {

            throw std::invalid_argument("Invalid token type specified after complete query: " + getTokenString(t));
        }

        return std::move(std::make_unique<SelectStatement>(SelectStatement{selectedSet, tableReference, std::move(tableExpression)}));
    }


    SOSQLExpression Parser::parseTableExpression() {


        if (!MATCH_KEYWORD<KeywordToken::WHERE>(m_lexer.peek())) {

            return std::make_unique<NULLExpression>();
        }

        m_lexer.nextToken(); // this is the WHERE token;
        return parseORExpression();
    }


    SOSQLExpression Parser::parseORExpression() {

        SOSQLExpression expression = parseANDExpression();
        while (MATCH_KEYWORD<KeywordToken::OR>(m_lexer.peek())) {

            m_lexer.nextToken(); // scan past or token
            SOSQLExpression right = parseANDExpression();
            expression = std::make_unique<ORExpression>(ORExpression{std::move(expression), std::move(right)});
        }

        return expression;
    }


    SOSQLExpression Parser::parseANDExpression() {

        SOSQLExpression expression = parseBooleanFactor();
        while (MATCH_KEYWORD<KeywordToken::AND>(m_lexer.peek())) {

            m_lexer.nextToken(); // scan past and token
            SOSQLExpression right = parseBooleanFactor();
            expression = std::make_unique<ANDExpression>(ANDExpression{std::move(expression), std::move(right)});
        }

        return expression;
    }


    SOSQLExpression Parser::parseBooleanFactor() {

        if(MATCH_KEYWORD<KeywordToken::NOT>(m_lexer.peek())) {
            m_lexer.nextToken();
            return std::make_unique<NOTExpression>(NOTExpression{parseBooleanExpression()});
        }

        return parseBooleanExpression();
    }


    SOSQLExpression Parser::parseBooleanExpression() {

        const Token lhs = m_lexer.nextToken();
        if (!MATCH_TERMINAL(lhs)) {

            throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(lhs));
        }

        return std::visit(overloaded {
                [=] (ComparisonToken) { return parseComparisonExpression(lhs); },
                [=] (const KeywordToken k) {
                    switch (k.m_keyword) {
                        case KeywordToken::IS:
                            return parseISExpression(lhs);
                        case KeywordToken::BETWEEN:
                            return parseBETWEENExpression(lhs);
                        default:
                            const std::string exceptionString = "Invalid token in expression: " + getTokenString(k);
                            throw std::invalid_argument(exceptionString); 
                        }
                },
                [=] (auto const t) -> SOSQLExpression {
                    const std::string exceptionString = "Invalid operator token in expression: " + getTokenString(t);
                    throw std::invalid_argument(exceptionString); 
                } },
            m_lexer.peek());
    }

    SOSQLExpression Parser::parseComparisonExpression(const Token lhs) {

        const ComparisonToken comp = std::get<ComparisonToken>(m_lexer.nextToken());
        const Token rhs = m_lexer.nextToken();
        if (!MATCH_TERMINAL(rhs)) {

            throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(rhs));
        }

        return std::make_unique<ComparisonExpression>(ComparisonExpression{comp.m_opType, lhs, rhs});
    }


    SOSQLExpression Parser::parseISExpression(const Token lhs) {

        Token rhs = m_lexer.nextToken();
        ComparisonToken::OpType op = ComparisonToken::OP_EQ;
        if (MATCH_KEYWORD<KeywordToken::NOT>(rhs)) {
            op = ComparisonToken::OP_NE;
            rhs = m_lexer.nextToken();
        }

        if (!MATCH_TERMINAL(rhs)) {

            throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(rhs));
        }

        return std::make_unique<ComparisonExpression>(ComparisonExpression{op, lhs, rhs});
    }


    SOSQLExpression Parser::parseBETWEENExpression(const Token lhs) {
        
        m_lexer.nextToken();
        if (!MATCH<NumericToken>(m_lexer.peek())) {
            throw std::invalid_argument("Only numeric tokens can be specified in the BETWEEN clause");
        }

        const NumericToken lowerBound = std::get<NumericToken>(m_lexer.nextToken());
        if (!MATCH_KEYWORD<KeywordToken::AND>(m_lexer.nextToken())) {
            throw std::invalid_argument("AND keyword missing from BETWEEN clause");
        }

        else if (!MATCH<NumericToken>(m_lexer.peek())) {
            throw std::invalid_argument("Second numeric token missing from BETWEEN clause");
        }

        const NumericToken upperBound = std::get<NumericToken>(m_lexer.nextToken());
        return std::make_unique<BETWEENExpression>(BETWEENExpression{lowerBound.m_value, upperBound.m_value, lhs});
    }

    SelectSet Parser::parseSelectSetQuantifier() {

        return std::visit(overloaded {
                [=] (ColumnToken) { return parseSelectList(); },
                [=] (PunctuationToken<'*'>) -> SelectSet { 
                    m_lexer.nextToken(); 
                    return SelectSet{ ColumnToken{ColumnToken::PORT}, ColumnToken{ColumnToken::TCP}, ColumnToken{ColumnToken::UDP} };
                },
                [=] (auto t) -> SelectSet {
                    const std::string exceptionString = "Invalid token in select list: " + getTokenString(t);
                    throw std::invalid_argument(exceptionString); 
                } },
            m_lexer.peek());
    };


    SelectSet Parser::parseSelectList() {

        SelectSet selectedSet = { };
        for (bool moreColumns = true; moreColumns;) {
            const Token t = m_lexer.peek();
            if (MATCH<ColumnToken>(t)) {
                selectedSet.addColumn(std::get<ColumnToken>(t));
                m_lexer.nextToken();
                moreColumns = MATCH<PunctuationToken<','>>(m_lexer.peek());
            }

            else if (MATCH<PunctuationToken<','>>(t)) {
                m_lexer.nextToken();
            }

            else {
                std::string exceptionString = "Invalid token specified in select list: " + getTokenString(t);
                throw std::invalid_argument(exceptionString);
            }
        }

        return selectedSet;
    }


    std::string Parser::parseTableReference() {

        const Token t = m_lexer.nextToken();
        if (!MATCH_KEYWORD<KeywordToken::FROM>(t)) {
            const std::string exceptionString = "FROM token not following column list, invalid token specified: " + getTokenString(t);
            throw std::invalid_argument(exceptionString);
        }

        return std::visit(overloaded {
                [=] (UserToken u)  { return u.m_UserToken; },
                [=] (auto t) -> std::string { 
                    throw std::invalid_argument("Invalid token following FROM keyword: " + getTokenString(t)); } 
                }, 
            m_lexer.nextToken());
    }
}
