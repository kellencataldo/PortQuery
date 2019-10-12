#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>
#include <limits>

#include "LogFormat.h"


// @todo: comment and test this please

class argumentParser {
    public:
        explicit argumentParser(const int argc, const char * const argv[]) :
            m_arguments(argv + 1, argv + argc) { }

        template <typename T>
        void addCommand(const std::string command, const std::string helpText, const T defaultValue) {
            validateThenAdd<T>(command, defaultValue, COMMAND_TYPE_POSITIONAL, helpText);
        }

        template <typename T>
        T getCommand(const std::string command) {
            return getValue<T>(command, COMMAND_TYPE_POSITIONAL);
        }

        template <typename T>
        void addCommandList(const std::string command, const std::string helpText) {
            validateThenAdd<std::vector<T>>(command, std::vector<T>(), COMMAND_TYPE_LIST, helpText);
        }

        template <typename T>
        std::vector<T> getCommandList(const std::string command) {
            return getValue<std::vector<T>>(command, COMMAND_TYPE_LIST);
        }

        void addCommandFlag(const std::string command, const std::string helpText) {
            validateThenAdd<bool>(command, false, COMMAND_TYPE_FLAG, helpText);
        }

        bool getCommandFlag(const std::string command) {
            return getValue<bool>(command, COMMAND_TYPE_FLAG);
        }

        bool parse(void) {
            if(parseEngine()) {
                return true;
            }
            m_commands.clear();
            m_queryString.clear();
            return false;
        }

        std::string getQueryString(void) const {
            return m_queryString;
        }

    private:
        enum COMMAND_TYPE {
            COMMAND_TYPE_POSITIONAL,
            COMMAND_TYPE_LIST,
            COMMAND_TYPE_FLAG
        };

        class commandBase {
            public:
                explicit commandBase(const COMMAND_TYPE commandType, const std::string helpText) :
                    m_commandType(commandType), m_helpText(helpText) { }
                virtual ~commandBase(void) { }
                virtual bool parseArgument(const std::string argument) = 0;
                virtual std::string getArgString(void) const = 0;

                COMMAND_TYPE m_commandType;
                std::string m_helpText;
        };

        template <typename T>
        class commandArgument : public commandBase {
            public:
                explicit commandArgument(const T defaultValue, const COMMAND_TYPE commandType,
                        const std::string helpText) :
                    commandBase(commandType, helpText), m_value(defaultValue) { }

                bool parseArgument(const std::string argument) {
                    try {
                        argumentParser::convertArgument(argument, m_value);
                    }
                    catch(std::exception& e) {
                        std::cout << "EXCEPTION INFORMATION: " << e.what() << std::endl;
                        return false;
                    }
                    return true;
                }

                std::string getArgString(void) const {
                    return argumentParser::getRepresentation(m_value);
                }

                T m_value;
        };

        static bool isCommand(const std::string arg) {
            if(arg.size() > 2 && arg[0] == '-' && arg[1] == '-' && std::all_of(arg.begin()+2, arg.end(),
               [](const auto c){return std::isalpha(c);})) {
                return true;
            }
            return false;
        }

        template<typename T>
        void validateThenAdd(const std::string command, const T defaultVal, const COMMAND_TYPE commandType,
                const std::string helpText) {
            if(!isCommand(command)) {
                std::string message("Option format is two hyphens followed by alphabetical characters");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }

            if(m_commands.find(command) != m_commands.end()) {
                std::string message("Duplicate option detected");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }

            m_commands.emplace(command, std::make_shared<commandArgument<T>>(defaultVal, commandType, helpText));
        }

        static void convertArgument(std::string argument, int& value) {
            value = std::stoi(argument);
        }

        static void convertArgument(std::string argument, std::string& value) {
            value = argument;
        }

        static void convertArgument(std::string argument, bool& value) {
            std::string message("Boolean commands do not get converted");
            throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
        }

        template<typename T>
        static void convertArgument(std::string argument, std::vector<T>& valueVector) {
            T value;
            argumentParser::convertArgument(argument, value);
            valueVector.push_back(value);
        }

        static std::string getRepresentation(const std::string value) {
            return "[string]";
        }

        static std::string getRepresentation(const int value) {
            return "[int]";
        }

        static std::string getRepresentation(const bool value) {
            return "";
        }

        template<typename T>
        static std::string getRepresentation(const std::vector<T> value) {
            T baseValue;
            return "{" + argumentParser::getRepresentation(baseValue) + "...}";
        }

        template <typename T>
        T getValue(const std::string command, const COMMAND_TYPE commandType) const {
            const auto commandCandidate = m_commands.find(command);
            if(commandCandidate == m_commands.end()) {
                std::string message("Unknown command: " + command);
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            } else if (commandCandidate->second->m_commandType != commandType) {
                std::string message("Requested command type does not match actual command type");
                throw std::invalid_argument(formatMessage(__FILE__, __func__, __LINE__, message));
            }

            return std::dynamic_pointer_cast<commandArgument<T>>(commandCandidate->second)->m_value;
        }

        bool parseEngine(void) {
            if(std::find(m_arguments.begin(), m_arguments.end(), "--usage") != m_arguments.end() ||
                m_arguments.empty()) {
                std::cout << "WELCOME TO PORT QUERY. WHY ARE YOU USING THIS." << std::endl;
                std::cout << "USAGE: [OPTION1, OPTION2...] QUERY_STRING" << std::endl;
                if(m_commands.empty()) { return false; }
                size_t maxLength = std::max_element(m_commands.begin(), m_commands.end(),
                    [](const auto& lhs, const auto& rhs)
                    {return lhs.first.size() < rhs.first.size();})->first.size() + 50;
                std::cout.width(maxLength);
                std::cout << std::left << "    OPTION" << "HELP" << std::endl;
                std::for_each(m_commands.begin(), m_commands.end(), [&maxLength](const auto& e){
                    std::cout.width(maxLength);
                    std::cout  << ("    " + e.first + " " + e.second->getArgString());
                    std::cout << e.second->m_helpText << std::endl;});
                return false;
            }

            std::shared_ptr<commandBase> sliding = nullptr;
            for (auto argument = m_arguments.begin(); argument != m_arguments.end()-1; argument++) {
                if(isCommand(*argument) && m_commands.find(*argument) != m_commands.end()) {
                    if(sliding != nullptr && sliding->m_commandType == COMMAND_TYPE_POSITIONAL) {
                        std::cout << "ERROR: ARGUMENTS REQUIRED FOR FLAG. SEE --usage" << std::endl;
                        return false;
                    }

                    sliding = m_commands.find(*argument)->second;
                    if(sliding->m_commandType == COMMAND_TYPE_FLAG) {
                        std::dynamic_pointer_cast<commandArgument<bool>>(sliding)->m_value = true;
                        sliding = nullptr;
                    }
                    continue;
                }

                else if(sliding != nullptr && sliding->parseArgument(*argument)) {
                    sliding = (sliding->m_commandType == COMMAND_TYPE_POSITIONAL ? nullptr : sliding);
                    continue;
                }

                std::cout << "MISMATCHED ARGUMENT SPECIFIED: " << *argument << ". SEE --usage" << std::endl;
                return false;
            }

            if(sliding && sliding->m_commandType == COMMAND_TYPE_POSITIONAL) {
                std::cout << "ERROR: ARGUMENTS REQUIRED FOR FLAG. SEE --usage" << std::endl;
                return false;
            }

            m_queryString = *m_arguments.rbegin();
            return true;
        }

        std::string m_queryString;
        std::vector<std::string> m_arguments;
        std::map<std::string, std::shared_ptr<commandBase>> m_commands;
};
