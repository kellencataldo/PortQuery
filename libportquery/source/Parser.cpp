#include "Parser.h"

#include <algorithm>


template <typename Subset, typename Superset> Subset variantSubsetCast(Superset from) {

    return std::visit([] (auto&& elem) -> Subset {
            using Subtype = std::decay_t<decltype(elem)>;
            if constexpr (std::is_constructible_v<Subset, Subtype>) {
                return Subset(std::forward<decltype(elem)>(elem));
            }
            else {
                throw std::invalid_argument("Unable to convert variant to subset");
            }
        }, std::forward<Superset>(from));
}


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


std::string getExtendedTokenInfo(const ColumnToken c) {
    std::string prefix{"[COLUMN TOKEN: "};
    switch (c.m_column) {
        case ColumnToken::TCP:
            return prefix + "TCP]";
        case ColumnToken::UDP:
            return prefix + "UDP]";
        case ColumnToken::PORT:
            return prefix + "PORT]";
        default:
            return prefix + "UNKNOWN COLUMN]";
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
                [=] (PunctuationToken<'*'>) { return std::string("[ * ]"); },
                [=] (PunctuationToken<'('>) { return std::string("[ ( ]"); }, 
                [=] (PunctuationToken<')'>) { return std::string("[ ) ]"); },
                [=] (PunctuationToken<';'>) { return std::string("[ ; ]"); }, 
                [=] (PunctuationToken<','>) { return std::string("[ , ]"); },
                [=] (NumericToken n)    { return getExtendedTokenInfo(n); },
                [=] (UserToken u)       { return getExtendedTokenInfo(u); },
                [=] (ComparisonToken c) { return getExtendedTokenInfo(c); },
                [=] (ColumnToken c)     { return getExtendedTokenInfo(c); },
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
    return std::make_unique<SelectStatement>(SelectStatement{selectedSet, tableReference, std::move(tableExpression)});
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
    if (!MATCH<ColumnToken, QueryResultToken, NumericToken>(lhs)) {

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


bool canCompareOperands(const ColumnToken c, const ComparisonToken::OpType op, const Token rhs) {

    UNUSED_PARAMETER(op);

    if (c.m_column == ColumnToken::PORT) {
        return std::visit(overloaded {
            [=] (const ColumnToken c) {      return c.m_column == ColumnToken::PORT; },
            [=] (const NumericToken n) {     return true; },
            [=] (const QueryResultToken q) { return false; },
            [=] (auto t) {                   return false; } },
        rhs);
    }

    // otherwise all other columns besides PORT are of the QUERY RESULT data type
    return MATCH<QueryResultToken>(rhs);
}


bool canCompareOperands(const NumericToken n, const ComparisonToken::OpType op, const Token rhs) {

    UNUSED_PARAMETER(op);

    if (MATCH<ColumnToken>(rhs)) {

        return ColumnToken::PORT == std::get<ColumnToken>(rhs).m_column;
    }

    return MATCH<NumericToken>(rhs);
}


bool canCompareOperands(const QueryResultToken q, const ComparisonToken::OpType op, const Token rhs) {

    if (MATCH<ColumnToken>(rhs)) {

        // all other columns besides port are QUERY RESULT data type
        return ColumnToken::PORT != std::get<ColumnToken>(rhs).m_column;
    }

    else if (MATCH<QueryResultToken>(rhs) && ComparisonToken::OP_EQ == op) {

        return ComparisonToken::OP_EQ == op || ComparisonToken::OP_NE == op;
    }

    return false;
}

bool canCompareOperands(const Token lhs, const ComparisonToken::OpType op, const Token rhs) {

    return std::visit( [=] (auto&&t) { return canCompareOperands(t, op, rhs); }, lhs);
}


SOSQLExpression Parser::parseComparisonExpression(const Token lhs) {

    const ComparisonToken comp = std::get<ComparisonToken>(m_lexer.nextToken());
    const Token rhs = m_lexer.nextToken();
    if (!MATCH<ColumnToken, QueryResultToken, NumericToken>(rhs)) {

        throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(rhs));
    }

    else if (!canCompareOperands(lhs, comp.m_opType, rhs)) {

        std::string exceptionString = "Unable to compare operands: " + getTokenString(lhs) + " " + getTokenString(rhs);
        exceptionString += ". Operator: " + getTokenString(comp);
        throw std::invalid_argument(exceptionString);
    }

    Terminal LHSTerminal = variantSubsetCast<Terminal, Token>(lhs);
    Terminal RHSTerminal = variantSubsetCast<Terminal, Token>(rhs);

    return std::make_unique<ComparisonExpression>(ComparisonExpression{comp.m_opType, LHSTerminal, RHSTerminal});
}


SOSQLExpression Parser::parseISExpression(const Token lhs) {

    Token rhs = m_lexer.nextToken();
    ComparisonToken::OpType op = ComparisonToken::OP_EQ;
    if (MATCH<NOTToken>(rhs)) {
        op = ComparisonToken::OP_NE;
        rhs = m_lexer.nextToken();
    }

    if (!MATCH<ColumnToken, QueryResultToken, NumericToken>(rhs)) {

        throw std::invalid_argument("Invalid token type specified in expression: " + getTokenString(rhs));
    }

    else if (!canCompareOperands(lhs, op, rhs)) {

        std::string exceptionString = "Unable to compare operands in IS expression: ";
        exceptionString += getTokenString(lhs) + " " + getTokenString(rhs);
        throw std::invalid_argument(exceptionString);
    }

    Terminal LHSTerminal = variantSubsetCast<Terminal, Token>(lhs);
    Terminal RHSTerminal = variantSubsetCast<Terminal, Token>(rhs);

    return std::make_unique<ComparisonExpression>(ComparisonExpression{op, LHSTerminal, RHSTerminal});
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
    Terminal terminal = variantSubsetCast<Terminal, Token>(lhs);
    return std::make_unique<BETWEENExpression>(BETWEENExpression{terminal, lowerBound.m_value, upperBound.m_value});
}

SelectSet Parser::parseSelectSetQuantifier() {

    return std::visit(overloaded {
            [=] (ColumnToken) { 
                return parseSelectList(); 
            },
            [=] (PunctuationToken<'*'>) -> SelectSet { 
                m_lexer.nextToken(); 
                return { ColumnToken::PORT, ColumnToken::TCP, ColumnToken::UDP }; 
            },
            [=] (auto t) -> SelectSet {
                const std::string exceptionString = "Invalid token in select list: " + getTokenString(t);
                throw std::invalid_argument(exceptionString); 
            } },
        m_lexer.peek());
};

std::vector<ColumnToken::Column> Parser::parseSelectList() {

    SelectSet selectedSet = { };
    for (bool moreColumns = true; moreColumns;) {
        std::visit(overloaded { 
                [&] (ColumnToken c) { 
                    if (selectedSet.end() != std::find(selectedSet.begin(), selectedSet.end(), c.m_column)) {
                        std::string exceptionString = "Invalid token specified in select list: " + getTokenString(c);
                        throw std::invalid_argument(exceptionString);
                    }

                    selectedSet.push_back(c.m_column);
                    m_lexer.nextToken();
                    moreColumns = MATCH<PunctuationToken<','>>(m_lexer.peek());
                },
                [&] (PunctuationToken<','>) {
                    m_lexer.nextToken();
                },
                [=] (const auto t) -> void { 
                    std::string exceptionString = "Invalid token specified in select list: " + getTokenString(t);
                    throw std::invalid_argument(exceptionString);
                } },
           
            m_lexer.peek());
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
