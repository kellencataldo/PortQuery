#include "Statement.h"


bool BETWEENExpression::shouldSubmitForScan(Environment env) const {

    if (env.availablePreSubmission(m_terminal)) {

        return evaluate(env);
    }

    return true;
}


bool ComparisonExpression::shouldSubmitForScan(Environment env) const {
    
    if (env.availablePreSubmission(m_LHSTerminal) && env.availablePreSubmission(m_RHSTerminal)) {

        return evaluate(env);
    }

    return true;
}
