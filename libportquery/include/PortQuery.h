
#pragma once

#include <string>
#include <functional>
#include <any>
#include <variant>

#include "../source/Parser.h"


class PortQuery {

    public:

        using PQ_PORT = uint16_t;
        enum PQ_QUERY_RESULT { 
            OPEN = 0,
            CLOSED = 1,
            REJECTED = 2
        };

        using PQ_COLUMN = std::variant<PQ_PORT, PQ_QUERY_RESULT>;
        using PQ_ROW = std::vector<PQ_COLUMN>;
        using PQCallback = std::function<void(std::any, PQ_ROW)>;

        PortQuery(PQCallback const callback=nullptr, 
                const std::any context=nullptr, 
                const int timeout=TIMEOUT_DEFAULT,
                const int threadCount=THREADCOUNT_DEFAULT) : 
            m_userCallback(callback), m_userContext(context), m_timeout(timeout), m_threadCount(threadCount),
            m_selectStatement(nullptr) { }

        bool prepare(std::string queryString);
        bool run();
        bool finalize();
        bool execute(std::string queryString);


        void setUserCallback(const PQCallback userCallback) {

            m_userCallback = userCallback;
        }

        void setUserData(const std::any userContext) {

            m_userContext = userContext;
        }

        void setTimeout(const int timeout) {

            m_timeout = timeout;
        }

        std::string getErrorString() const {

            return m_errorString;
        }

    private:

        static constexpr int TIMEOUT_DEFAULT = 2;
        int m_timeout;
        static constexpr int THREADCOUNT_DEFAULT = 0;
        int m_threadCount;

        PQCallback m_userCallback;
        std::any m_userContext;

        SOSQLSelectStatement m_selectStatement;
        std::string m_errorString;
};
