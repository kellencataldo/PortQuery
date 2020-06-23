#pragma once

#include <cstdint>


class Environment {

    public:

        Environment() : m_port(0) { }

        void setPort(const uint16_t port) {

            m_port = port;
        }

    private:

        uint16_t m_port;


};
