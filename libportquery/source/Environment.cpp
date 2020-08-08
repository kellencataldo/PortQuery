#include "Environment.h"

namespace PortQuery {

    GeneratorFunction EnvironmentFactory::m_generator = defaultGenerator;

    void EnvironmentFactory::setGenerator(const GeneratorFunction generator) {

        m_generator = generator;
    }

    EnvironmentPtr EnvironmentFactory::createEnvironment(const unsigned int threadCount) {

        return m_generator(threadCount);
    }

    void IEnvironment::setPort(const uint16_t port) {

        m_port = port;
    }

    bool IEnvironment::getPort(void) const  {

        return m_port;
    }

    bool NetworkEnvironment::submitPortForScan(const uint16_t port, NetworkProtocol requestedProtocols) {

        return true;
    };
}


