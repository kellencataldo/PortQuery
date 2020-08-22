#include "Environment.h"


namespace PortQuery {

    GeneratorFunction EnvironmentFactory::m_generator = defaultGenerator;

    void EnvironmentFactory::setGenerator(const GeneratorFunction generator) {

        m_generator = generator;
    }

    EnvironmentPtr EnvironmentFactory::createEnvironment(const unsigned int threadCount) {

        return m_generator(threadCount);
    }


    void IEnvironment::setProtocolsToScan(const NetworkProtocol protocols) {

        m_protocolsToScan = protocols;
    }

    void IEnvironment::setPort(const uint16_t port) {

        m_port = port;
    }

    uint16_t IEnvironment::getPort(void) const  {

        return m_port;
    }

    bool NetworkEnvironment::scanPort(void) {

        return true;
    };
}


