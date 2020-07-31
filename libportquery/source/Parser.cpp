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
                return prefix + "EQ]";
            case ComparisonToken::OP_GT:
                return prefix + "GT]";
            case ComparisonToken::OP_LT:
                return prefix + "LT]";
            case ComparisonToken::OP_GTE:
                return prefix + "GTE]";
            case ComparisonToken::OP_LTE:
                return prefix + "LTE]";
            case ComparisonToken::OP_NE:
                return prefix + "NE]";
            default:
                return prefix + "UNKNOWN COMPARISON TOKEN]";
        }
    }


    std::string getTokenString(const Token t) {

        return std::visit(overloaded {
                    [=] (ALLToken)        { return std::string("[ALL KEYWORD]"); },
                    [=] (ANDToken)        { return std::string("[AND KEYWORD]"); },
                    [=] (ANYToken)        { return std::string("[ANY KEYWORD]"); },
                    [=] (BETWEENToken)    { return std::string("[BETWEEN KEYWORD]"); },
                    [=] (COUNTToken)      { return std::string("[COUNT KEYWORD]"); },
                    [=] (FROMToken)       { return std::string("[FROM KEYWORD]"); },
                    [=] (IFToken)         { return std::string("[IF KEYWORD]"); },
                    [=] (INToken)         { return std::string("[IN KEYWORD]"); },
                    [=] (ISToken)         { return std::string("[IS KEYWORD]"); },
                    [=] (LIMITToken)      { return std::string("[LIMIT KEYWORD]"); },
                    [=] (NOTToken)        { return std::string("[NOT KEYWORD]"); },
                    [=] (ORToken)         { return std::string("[OR KEYWORD]"); },
                    [=] (ORDERToken)      { return std::string("[ORDER KEYWORD]"); },
                    [=] (SELECTToken)     { return std::string("[SELECT KEYWORD]");},
                    [=] (WHEREToken)      { return std::string("[WHERE KEYWORD]"); },
                    [=] (EOFToken)        { return std::string("[END OF INPUT]"); },
                    [=] (OPENToken)       { return std::string("[OPEN]"); },
                    [=] (CLOSEDToken)     { return std::string("[CLOSED]"); },
                    [=] (REJECTEDToken)   { return std::string("[REJECTED]"); },
                    [=] (PORTToken)       { return std::string("[PORT]"); },
                    [=] (TCPToken)        { return std::string("[TCPToken]"); },
                    [=] (UDPToken)        { return std::string("[UDPToken]"); },
                    [=] (PunctuationToken<'*'>) { return std::string("[ * ]"); },
                    [=] (PunctuationToken<'('>) { return std::string("[ ( ]"); }, 
                    [=] (PunctuationToken<')'>) { return std::string("[ ) ]"); },
                    [=] (PunctuationToken<';'>) { return std::string("[ ; ]"); }, 
                    [=] (PunctuationToken<','>) { return std::string("[ , ]"); },
                    [=] (NumericToken n)    { return getExtendedTokenInfo(n); },
                    [=] (UserToken u)       { return getExtendedTokenInfo(u); },
                    [=] (ComparisonToken c) { return getExtendedTokenInfo(c); },
                    [=] (auto) -> std::string { return std::string("[UNKNOWN TOKEN]"); } }, 
                t);
        }


    SOSQLSelectStatement Parser::parseSOSQLStatement() {

        // SOSQL statements can obviously only begin with the "SELECT" keyword
        if (!MATCH<SELECTToken>(m_lexer.nextToken())) {

            throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
        }


        SelectSet selectedSet = parseSelectSetQuantifier();
        std::string tableReference = parseTableReference();
        SOSQLExpression tableExpression = parseTableExpression();

        // parse end here, check for EOF and semicolon
        // parse where statement here.
        return std::move(std::make_unique<SelectStatement>(SelectStatement{selectedSet, tableReference, std::move(tableExpression)}));
    }


    SOSQLExpression Parser::parseTableExpression() {


        if (!MATCH<WHEREToken>(m_lexer.peek())) {

            return std::make_unique<NULLExpression>();
        }

        m_lexer.nextToken(); // this is the WHERE token;
        return parseORExpression();
    }


    SOSQLExpression Parser::parseORExpression() {

        SOSQLExpression expression = parseANDExpression();
        while (MATCH<ORToken>(m_lexer.peek())) {

            m_lexer.nextToken(); // scan past or token
            SOSQLExpression right = parseANDExpression();
            expression = std::make_unique<ORExpression>(ORExpression{std::move(expression), std::move(right)});
        }

        return expression;
    }


    SOSQLExpression Parser::parseANDExpression() {

        SOSQLExpression expression = parseBooleanFactor();
        while (MATCH<ANDToken>(m_lexer.peek())) {

            m_lexer.nextToken(); // scan past and token
            SOSQLExpression right = parseBooleanFactor();
            expression = std::make_unique<ANDExpression>(ANDExpression{std::move(expression), std::move(right)});
        }

        return expression;
    }


    SOSQLExpression Parser::parseBooleanFactor() {

        if(MATCH<NOTToken>(m_lexer.peek())) {
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
                [=] (ISToken)             { return parseISExpression(lhs); },
                [=] (BETWEENToken) {  return parseBETWEENExpression(lhs); }, 
                [=] (auto const t) -> SOSQLExpression {
                    const std::string exceptionString = "Invalid operator token in expression: " + getTokenString(t);
                    throw std::invalid_argument(exceptionString); 
                } },
            m_lexer.peek());
    }


    bool canCompareProtocolOperands(const ComparisonToken::OpType op, const Token rhs) {

        if (ComparisonToken::OP_EQ != op || ComparisonToken::OP_NE != op) {

            return false;
        }

        return MATCH<OPENToken, CLOSEDToken, REJECTEDToken, TCPToken, UDPToken>(rhs);
    }

    bool canCompareOperands(const Token lhs, const ComparisonToken::OpType op, const Token rhs) {
        return std::visit(overloaded {
                [=] (PORTToken)     { return MATCH<PORTToken, NumericToken>(rhs); },
                [=] (NumericToken)  { return MATCH<PORTToken, NumericToken>(rhs);},
                [=] (OPENToken)     { return canCompareProtocolOperands(op, rhs); },
                [=] (CLOSEDToken)   { return canCompareProtocolOperands(op, rhs); },
                [=] (REJECTEDToken) { return canCompareProtocolOperands(op, rhs); },
                [=] (TCPToken)      { return canCompareProtocolOperands(op, rhs); },
                [=] (UDPToken)      { return canCompareProtocolOperands(op, rhs); },
                [=] (auto const t) -> bool {
                    const std::string exceptionString = "Invalid operator token in expression: " + getTokenString(t);
                    throw std::invalid_argument(exceptionString); 
                } },
            lhs);
    }


    SOSQLExpression Parser::parseComparisonExpression(const Token lhs) {

        const ComparisonToken comp = std::get<ComparisonToken>(m_lexer.nextToken());
        const Token rhs = m_lexer.nextToken();
        if (!MATCH_TERMINAL(rhs)) {

            throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(rhs));
        }

        else if (!canCompareOperands(lhs, comp.m_opType, rhs)) {

            std::string exceptionString = "Unable to compare operands: " + getTokenString(lhs) + " " + getTokenString(rhs);
            exceptionString += ". Operator: " + getTokenString(comp);
            throw std::invalid_argument(exceptionString);
        }

        return std::make_unique<ComparisonExpression>(ComparisonExpression{comp.m_opType, lhs, rhs});
    }


    SOSQLExpression Parser::parseISExpression(const Token lhs) {

        Token rhs = m_lexer.nextToken();
        ComparisonToken::OpType op = ComparisonToken::OP_EQ;
        if (MATCH<NOTToken>(rhs)) {
            op = ComparisonToken::OP_NE;
            rhs = m_lexer.nextToken();
        }

        if (!MATCH_TERMINAL(rhs)) {

            throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(rhs));
        }

        else if (!canCompareOperands(lhs, op, rhs)) {

            std::string exceptionString = "Unable to compare operands in IS expression: ";
            exceptionString += getTokenString(lhs) + " " + getTokenString(rhs);
            throw std::invalid_argument(exceptionString);
        }

        return std::make_unique<ComparisonExpression>(ComparisonExpression{op, lhs, rhs});
    }


    SOSQLExpression Parser::parseBETWEENExpression(const Token lhs) {
        
        m_lexer.nextToken();
        if (!MATCH<NumericToken>(m_lexer.peek())) {
            throw std::invalid_argument("Only numeric tokens can be specified in the BETWEEN clause");
        }

        const NumericToken lowerBound = std::get<NumericToken>(m_lexer.nextToken());
        if (!canCompareOperands(lhs, ComparisonToken::OP_GT, lowerBound)) {
            std::string exceptionString = "Unable to compare operands in BETWEEN expression: ";
            exceptionString += getTokenString(lhs) + " " + getTokenString(lowerBound);
            throw std::invalid_argument(exceptionString);
        }

        if (!MATCH<ANDToken>(m_lexer.nextToken())) {
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
                [=] (PORTToken) { return parseSelectList(); },
                [=] (TCPToken)  { return parseSelectList(); },
                [=] (UDPToken)  { return parseSelectList(); },

                [=] (PunctuationToken<'*'>) -> SelectSet { 
                    m_lexer.nextToken(); 
                    SelectSet selectedSet{ };
                    selectedSet.addColumn(PORTToken{});
                    selectedSet.addColumn(TCPToken{});
                    selectedSet.addColumn(UDPToken{});
                    return selectedSet;
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
            if (MATCH<PORTToken, TCPToken, UDPToken>(t)) {
                selectedSet.addColumn(t);
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
        if (!MATCH<FROMToken>(t)) {
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
