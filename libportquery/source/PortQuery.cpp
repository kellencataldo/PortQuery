#include <iostream>

#include "PortQuery.h"
#include "Parser.h"
#include "Network.h"
#include "Environment.h"

#include <iostream>


bool PortQuery::prepare(std::string queryString) {

    if (m_selectStatement) {

        // maybe just finalize automatically here?
        m_errorString = "Previous query has not been finalized";
        return false;
    }

    Parser parseEngine{queryString};
    try {

        m_selectStatement = parseEngine.parseSOSQLStatement();
    } 
    catch (std::invalid_argument& e) {

        m_errorString = e.what();
        return false;
    }

    m_errorString.clear();
    return true;
}


bool PortQuery::run() {

    static constexpr uint32_t MAX_PORT = static_cast<uint16_t>(-1);
    const NetworkProtocols requiredProtocols = m_selectStatement->collectRequiredProtocols();

    EnvironmentPtr env = EnvironmentFactory::createEnvironment(m_threadCount);
    for (uint32_t port = 0; port <= MAX_PORT; port++) {

        if (Tristate::FALSE_STATE != m_selectStatement->attemptPreNetworkEval(port)) {

            env->submitPortForScan(static_cast<uint16_t>(port), requiredProtocols);
        }
    }

    return true;
}


bool PortQuery::finalize() {

    m_errorString.clear();
    if (m_selectStatement) {

        m_selectStatement.reset();
        m_errorString.clear();
        return true;
    }

    return false;
}

bool PortQuery::execute(std::string queryString) { 
    return false;
}


