#pragma once

#include <cstdint>

#include "Lexer.h"


class Environment {

    public:

        Environment() : m_port(0) { }

        void setPort(const uint16_t port) {

            m_port = port;
        }

        bool availablePreSubmission(const Token t) const;
        uint16_t retrieveTokenValue(const Token t);

    private:

        uint16_t getColumnResult(ColumnToken::Column c);

        uint16_t m_port;


};
