#include <iostream>
#include <chrono>
#include <thread>

#include "Statement.h"
#include "PortQuery.h"
#include "Parser.h"
#include "Network.h"
#include "Environment.h"


namespace PortQuery { 

    bool PQConn::prepare(std::string queryString) {

        if (m_selectStatement) {

            // maybe just finalize automatically here?
            m_errorString = "Previous query has not been finalized";
            return false;
        }

        Parser parseEngine{queryString};
        try {

            m_selectStatement = std::move(parseEngine.parseSOSQLStatement());
        } 
        catch (std::invalid_argument& e) {

            m_errorString = e.what();
            return false;
        }

        m_errorString.clear();
        return true;
    }


    bool PQConn::run() {

        // should this throw error if no userprovided callback is present?

        EnvironmentPtr env = EnvironmentFactory::createEnvironment(m_threadCount);
        env->setProtocolsToScan(m_selectStatement->collectRequiredProtocols());

        static constexpr uint32_t MAX_PORT = static_cast<uint16_t>(-1);
        for (uint32_t port = 0; port <= MAX_PORT; port++) {

            env->setPort(port);
            if (Tristate::FALSE_STATE != m_selectStatement->attemptPreNetworkEval(env)) {

                env->scanPort();
                std::this_thread::sleep_for(std::chrono::milliseconds(m_delayMS));
            }
        }

        /*

        for (uint32_t port = 0; port <= MAX_PORT; port++) {

            env->setPort(port);
            if (m_selectStatement->postNetworkEval(env)) {

                // call back here. m_selectStatement->getCurrentRepresentation()

            }

        }

        */

        return true;
    }


    bool PQConn::finalize() {

        m_errorString.clear();
        if (m_selectStatement) {

            m_selectStatement.reset();
            m_errorString.clear();
            return true;
        }

        return false;
    }

    bool PQConn::execute(std::string queryString) { 
        if (prepare(queryString) && run() && finalize()) {

            return true;
        }

        return false;
    }

    PQConn::PQConn(PQCallback const callback, const std::any context, const int timeout, const int threadCount,
                  const int delayMS) : 
        m_userCallback(callback), m_userContext(context), m_timeout(timeout), m_threadCount(threadCount),
        m_delayMS(delayMS) { }

    PQConn::~PQConn() = default;
    PQConn::PQConn(PQConn&&) = default;
    PQConn& PQConn::operator=(PQConn&&) = default;
}
