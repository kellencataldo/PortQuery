#pragma once

#include <cstdint>
#include <memory>

#include "Network.h"


class IEnvironment {

    public:
        virtual bool submitPortForScan(const uint16_t port, const NetworkProtocols requestedProtocols) = 0;
};


using GeneratorFunction = std::shared_ptr<IEnvironment> (*)(const int threadCount);
using EnvironmentPtr = std::shared_ptr<IEnvironment>;

class EnvironmentFactory {

    public:

         static void setGenerator(const GeneratorFunction generator) {
            
             m_generator = generator;
         }

         static EnvironmentPtr createEnvironment(const unsigned int threadCount) {

             return m_generator(threadCount);
         }

    private:
        static EnvironmentPtr defaultGenerator(const int threadCount);
        static GeneratorFunction m_generator;
};
