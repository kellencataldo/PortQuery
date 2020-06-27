#include "Statement.h"


Tristate BETWEENExpression::attemptPreNetworkEval(const uint16_t port) const {

    return std::visit(overloaded {
            [=] (QueryResultToken q) { return Tristate::FALSE_STATE; },
            [=] (ColumnToken  c)     { return Tristate::FALSE_STATE; },
            [=] (NumericToken n)     { return Tristate::FALSE_STATE; } },
        m_terminal);
}


Tristate ComparisonExpression::attemptPreNetworkEval(const uint16_t port) const {
    return std::visit(overloaded {
            [=] (QueryResultToken q) { return Tristate::FALSE_STATE; },
            [=] (ColumnToken  c)     { return Tristate::FALSE_STATE; },
            [=] (NumericToken n)     { return Tristate::FALSE_STATE; } },
        m_terminal);
}



