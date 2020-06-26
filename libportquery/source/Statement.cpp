#include "Statement.h"


IExpression::PreNetworkEvaluation ORExpression::attemptPreNetworkEval(const uint16_t port) const {

    const PreNetworkEvaluation LHSResult = m_left->attemptPreNetworkEval(port);
    const PreNetworkEvaluation RHSResult = m_right->attemptPreNetworkEval(port);

    if (LHSResult == RHSResult) {

        return LHSResult;
    }

    else if (EvaluatedTruePreNet == LHSResult || EvaluatedTruePreNet == RHSResult) {

        return EvaluatedTruePreNet;
    }

    // is they are both false it would be caught in teh call above
    return UnableToEvaluatePreNet;
}


IExpression::PreNetworkEvaluation ANDExpression::attemptPreNetworkEval(const uint16_t port) const {

    const PreNetworkEvaluation LHSResult = m_left->attemptPreNetworkEval(port);
    const PreNetworkEvaluation RHSResult = m_right->attemptPreNetworkEval(port);

    if (LHSResult == RHSResult) {

        return LHSResult;
    }

    else if (EvaluatedFalsePreNet == LHSResult || EvaluatedFalsePreNet == RHSResult) {

        return EvaluatedFalsePreNet;
    }

    // is they are both false it would be caught in teh call above
    return UnableToEvaluatePreNet;
}
