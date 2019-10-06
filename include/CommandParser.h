#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>
#include <limits>

#include "LogFormat.h"


class commandBase {
    public:
        commandBase(const std::string _helpText ) : m_helpText(_helpText), m_registered(false) { }
        virtual std::string getArgText(void) const = 0;
        virtual bool addArgument(std::string argument) = 0;
        virtual bool notifyDisconnect(void) = 0;

        bool m_registered;
        std::string m_helpText;
};


template <typename T>
class commandOption : public commandBase {
    public:
        commandOption(const std::string _helpText, const T _default) :
            commandBase(_helpText),  m_value(_default), m_found(false) { }
        std::string getArgText(void) const {
            return commandOption<T>::getArgRepresentation();
        }

        static std::string getArgRepresentation(void) {
            return "";
        }

        bool addArgument(std::string argument) {
            if(m_found) {
                std::cout << "SECOND ARGUMENT: " + argument + " SPECIFIED. ONLY ONE ALLOWED" << std::endl;
                return false;
            }

            m_value = commandOption<T>::convertArgument(argument);
            m_found = true;
            return true;
        }

        bool notifyDisconnect(void) {
            if(m_registered && !m_found) {
                std::cout << "SPECIFIED OPTION REQUIRES AN ARGUMENT" << std::endl;
                return false;
            }
            return true;
        }

        T retrieveValue(void) const {
            return m_value;
        }

        static T convertArgument(std::string argument);

    private:
        T m_value;
        bool m_found;
};

template <> std::string commandOption<std::string>::getArgRepresentation(void) {
    return "[string]";
}

template <> std::string commandOption<int>::getArgRepresentation(void) {
    return "[int]";
}

template <> bool commandOption<bool>::addArgument(std::string argument)
{
    std::cout << "BOOLEAN FLAGS DO NOT TAKE OPTIONS" << std::endl;
    return false;
}

template <> int commandOption<int>::convertArgument(std::string argument) {
    return std::stoi(argument);
}

template <> std::string commandOption<std::string>::convertArgument(std::string argument) {
    return argument;
}

template <> bool commandOption<bool>::notifyDisconnect(void) {
    m_value = true;
    return true;
}


template <typename T>
class commandList : public commandBase {
    public:
        commandList(const std::string _helpText, unsigned int _minArguments, unsigned int _maxArguments) :
            commandBase(_helpText), m_minArguments(_minArguments), m_maxArguments(_maxArguments) {
            if(m_minArguments > m_maxArguments) {
                std::string message("minimum value exceeds maximum value");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            } else if(m_maxArguments == 0) {
                std::string message("maximum value is zero");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }
        }

        std::string getArgText(void) const {
            std::string subtypeRepr = commandOption<T>::getArgRepresentation();
            std::string min = m_minArguments ? "" : "min=" + std::to_string(m_minArguments);
            std::string max = m_maxArguments == std::numeric_limits<size_t>::max() ?
                "" : "max=" + std::to_string(m_maxArguments);
            return "{" + subtypeRepr + " " + "... } " + min + " " + max + " ";
        }

        bool addArgument(std::string argument) {
            if(m_maxArguments <= m_argList.size()) {
                std::cout << "MAXIMUM NUMBER OF ARGUMENTS EXCEEDED" << std::endl;
                return false;
            }

            m_argList.push_back(commandOption<T>::convertArgument(argument));
            return true;
        }

        bool notifyDisconnect(void) {
            if(m_minArguments >= m_argList.size()) {
                std::cout << "TOO FEW ARGUMENTS SPECIFIED" << std::endl;
                return false;
            }

            return true;
        }

        std::vector<T> retrieveValue(void) const {
            return m_argList;
        }

    private:
        std::vector<T> m_argList;
        unsigned int m_minArguments;
        unsigned int m_maxArguments;
};



class commandParser {
    public:
        commandParser(const int argc, const char * const argv[]) : m_arguments(argv + 1, argv + argc) { }

        template <typename T>
        void addOption(const std::string option, const std::string helpText, const T defaultValue) {
            validateOption(option);
            m_options.emplace(option, std::make_shared<commandOption<T>>(helpText, defaultValue));
        }

        template <typename T>
        void addOptionList(const std::string option, const std::string helpText, const size_t minArguments=0,
                const size_t maxArguments=std::numeric_limits<size_t>::max()) {
            validateOption(option);
            m_options.emplace(option, std::make_shared<commandList<T>>(helpText, minArguments, maxArguments));
        }

        void addFlag(const std::string flag, const std::string helpText) {
            validateOption(flag);
            m_options.emplace(flag, std::make_shared<commandOption<bool>>(helpText, false));
        }

        template <typename T>
        T retrieveValue(const std::string option) const {
            if(m_options.find(option) == m_options.end()) {
                std::string message("Unable to locate argument: " + option);
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }

        }

        bool parse(void) {
            if (m_arguments.empty() || m_arguments.at(0) == "--help") {
                size_t maxLength = std::max_element(m_options.begin(), m_options.end(),
                        [](const auto& lhs, const auto& rhs)
                        {return lhs.first.size() < rhs.first.size();})->first.size() + 50;
                std::cout << "WELCOME TO PORT QUERY. WHY ARE YOU USING THIS." << std::endl;
                std::cout << "USAGE: [OPTION1, OPTION2...] QUERY_STRING" << std::endl;
                std::cout.width(maxLength);
                std::cout << std::left << "    OPTION" << "HELP" << std::endl;
                std::for_each(m_options.begin(), m_options.end(), [&maxLength](const auto& e){
                        std::cout.width(maxLength);
                        std::cout  << ("    " + e.first + " " + e.second->getArgText());
                        std::cout << e.second->m_helpText << std::endl;});
                return false;
            }

            if(!parseLoop()) {
                std::cout << "USE --help TO SEE COMMANDS AND USAGE" << std::endl;
                return false;
            }

            return true;
        }


    private:
        static bool isOption(const std::string candidate) {
            if(candidate.size() > 2 && candidate[0] == '-' && candidate[1] == '-' &&
               std::all_of(candidate.begin() + 2, candidate.end(), [](const auto c){return std::isalpha(c);})) {
                return true;
            }
            return false;
        }

        void validateOption(const std::string option) {
            if(!isOption(option)) {
                std::string message("Option format is two hyphens followed by alphabetical characters");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }

            if(m_options.find(option) != m_options.end()) {
                std::string message("Duplicate option detected");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }
        }

        bool parseLoop(void) {
            std::shared_ptr<commandBase> sliding = nullptr;
            for (auto argument = m_arguments.begin(); argument != m_arguments.end()-1; argument++) {
                if(isOption(*argument)) {
                    if(m_options.find(*argument) == m_options.end()) {
                        std::cout << "UNKNOWN COMMAND SPECIFIED: " + *argument << std::endl;
                        return false;
                    }
                    if(sliding && !sliding->notifyDisconnect()) {
                        std::cout << "INCORRECT NUMBER OF ARGUMENTS FOR OPTION: " << *argument << std::endl;
                        return false;
                    }

                    sliding = m_options[*argument];
                    if(sliding->m_registered) {
                        std::cout << "DUPLICATE OPTION SPECIFIED: " + *argument << std::endl;
                        return false;
                    }

                    sliding->m_registered = true;
                }

                else if(sliding == nullptr) {
                    std::cout << "MISMATCHED ARGUMENT: " + *argument << std::endl;
                    return false;
                }

                else if(!sliding->addArgument(*argument)) {
                    std::cout << "UNABLE TO ADD ARGUMENTS FOR OPTION: " << *argument << std::endl;
                    return false;
                }
            }

            if(sliding && !sliding->notifyDisconnect()) {
                std::string argument = *(m_arguments.rbegin() + 1);
                std::cout << "INCORRECT NUMBER OF ARGUMENTS FOR OPTION: " << argument << std::endl;
                return false;
            }

            m_queryString = *m_arguments.rbegin();
            return true;
        }

        std::vector<std::string> m_arguments;
        std::map<std::string, std::shared_ptr<commandBase>> m_options;
        std::string m_queryString;
};

