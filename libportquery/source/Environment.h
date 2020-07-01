#pragma once

#include <cstdint>
#include <memory>

#include "Network.h"


class IEnvironment {

    public:
        bool submitPortForScan(const uint16_t port, const NetworkProtocols requestedProtocols);
};

typedef std::shared_ptr<IEnvironment> (*FactoryGenerator)(const unsigned int threadCount);

class EnvironmentFactory {

    public:

         static void setGenerator(const FactoryGenerator generator) {
            
             m_generator = generator;
         }

         static std::shared_ptr<IEnvironment> createEnvironment(const unsigned int threadCount) {

             return m_generator(threadCount);
         }

    private:
        static std::shared_ptr<IEnvironment> defaultGenerator(const unsigned int threadCount);
        static FactoryGenerator m_generator;
};

FactoryGenerator EnvironmentFactory::m_generator = defaultGenerator;

