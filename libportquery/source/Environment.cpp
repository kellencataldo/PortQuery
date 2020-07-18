#include "Environment.h"


GeneratorFunction EnvironmentFactory::m_generator = defaultGenerator;


bool NetworkEnvironment::submitPortForScan(const uint16_t port, NetworkProtocols requestedProtocols) {

    return true;

};


