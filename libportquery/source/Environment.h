#pragma once

#include <cstdint>
#include <memory>

#include "Network.h"
#include "ThreadPool.h"


namespace PortQuery {


    class IEnvironment {

        public:
            virtual bool submitPortForScan(const uint16_t port, const NetworkProtocols requestedProtocols) = 0;
    };


    using GeneratorFunction = std::shared_ptr<IEnvironment> (*)(const int threadCount);
    using EnvironmentPtr = std::shared_ptr<IEnvironment>;


    class NetworkEnvironment : public IEnvironment {

        public:

            NetworkEnvironment(const int threadCount) : m_threadPool(threadCount) { }
            virtual bool submitPortForScan(const uint16_t port, NetworkProtocols requestedProtocols) override;

        private:

            ThreadPool m_threadPool;
    };


    class EnvironmentFactory {

        public:

             static void setGenerator(const GeneratorFunction generator) {
                
                 m_generator = generator;
             }

             static EnvironmentPtr createEnvironment(const unsigned int threadCount) {

                 return m_generator(threadCount);
             }

        private:
            static EnvironmentPtr defaultGenerator(const int threadCount) {

                EnvironmentPtr out = std::make_shared<NetworkEnvironment>(threadCount);
                return out;
            }

            static GeneratorFunction m_generator;
    };

}
