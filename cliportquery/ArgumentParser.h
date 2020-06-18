#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>
#include <limits>


// This empty output policy exists so that when testing these methods the console
// is not clogged up with error messages from the argument parser. This is the default policy
// To use this in any meaningful way, simply create a new output policy which supports the methods
// present in the example empty output class. Then instantiate the argumentParser 
// template with this new class

struct emptyOutput {
    public:
        static void output(const std::string outputString) { }
        static void setWidth(const size_t width) { }
    protected:
        ~emptyOutput() { }
};


// This is the argument parser policy, a simple rudimentary argument parser which can handle
// a flag (IE --flag) a positional commands (IE --positional argument) or a list of arguments
// (IE --list arg1 arg2 arg2). The query string is assumed to be the last argument and is 
// stored as a string
//
// The types that are currently supported as command parameters are: strings and ints
// Flag arguments are treated as bools: (flag present == true, flag not present == false)
// Arguments supplied to list commands must all be the same type

template <typename outputPolicy = emptyOutput> class argumentParser : public outputPolicy {
    public:

        // The argument parser constructor is designed to take arguments directly from the arguments
        // passed on the command line to the main entry of the program
        explicit argumentParser(const int argc, const char * const argv[]) :
            // This first argument is skipped, because this is the name of the image being executed, and
            // this is not important to our application
            m_arguments(argv + 1, argv + argc) { }

        // This method registers a command with the parser. Callers must supply the type of argument
        // they respect to receive upon retrieval, a default value for the command must be supplied
        // A help text which describes the functionality of the command
        template <typename T>
        void addCommand(const std::string command, const std::string helpText, const T defaultValue) {
            validateThenAdd<T>(command, defaultValue, COMMAND_TYPE_POSITIONAL, helpText);
        }

        // This method retrieves the specified command, the template parameter used here must match what 
        // template parameter was used when calling addCommand
        template <typename T>
        T getCommand(const std::string command) {
            return getValue<T>(command, COMMAND_TYPE_POSITIONAL);
        }

        // This method registers a command list with the parser, parsers need only supply the help text
        // The default value is simply an empty list. Every argument following this command will be converted
        // To the template argument type. A help text is required to describe the functionality
        // of this command
        template <typename T>
        void addCommandList(const std::string command, const std::string helpText) {
            validateThenAdd<std::vector<T>>(command, std::vector<T>(), COMMAND_TYPE_LIST, helpText);
        }

        // This method retrieves the specified command list, the template parameter used here must
        // match what template parameter was used when calling addCommandList
        template <typename T>
        std::vector<T> getCommandList(const std::string command) {
            return getValue<std::vector<T>>(command, COMMAND_TYPE_LIST);
        }

        // This method registers a flag with the parser. There is no default parameter needed as 
        // presence is used instead. If the flag has been supplied, that means the value is true.
        // If it is not supplied as a part of the command line arguments, it is false.
        void addCommandFlag(const std::string command, const std::string helpText) {
            validateThenAdd<bool>(command, false, COMMAND_TYPE_FLAG, helpText);
        }

        // This method checks if the registered flag was supplied on the command line or not
        bool getCommandFlag(const std::string command) {
            return getValue<bool>(command, COMMAND_TYPE_FLAG);
        }

        bool parse(void) {

            // This method is a wrapper around the actual parsing engine below, on failure, it
            // wipes out all state in the parser
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

        // An enum containing all the different types of commands which are supported by the parser,
        // These are stored in the base class below and used to downcast base class pointers to the
        // inherited command subtypes
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
                        outputPolicy::output(std::string("EXCEPTION INFORMATION: ") + e.what());
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
                throw std::invalid_argument("Option format is two hyphens followed by alphabetical characters");
            }

            if(m_commands.find(command) != m_commands.end()) {
                throw std::invalid_argument("duplicate option detected");
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
            throw std::invalid_argument("boolean commands do not get converted");
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
                throw std::invalid_argument("Unknown command: " + command);
            } else if (commandCandidate->second->m_commandType != commandType) {
                throw std::invalid_argument("Requested command type does not match actual command type");
            }

            return std::dynamic_pointer_cast<commandArgument<T>>(commandCandidate->second)->m_value;
        }

        bool parseEngine(void) {
            if(std::find(m_arguments.begin(), m_arguments.end(), "--usage") != m_arguments.end() || m_arguments.empty()) {
                outputPolicy::output("WELCOME TO PORT QUERY. WHY ARE YOU USING THIS.");
                outputPolicy::output("USAGE: [OPTION1, OPTION2...] QUERY_STRING");
                if(m_commands.empty()) { 
                    return false; 
                }

                size_t maxLength = std::max_element(m_commands.begin(), m_commands.end(),
                    [] (const auto& lhs, const auto& rhs) {
                        return lhs.first.size() < rhs.first.size();
                    })->first.size() + 50;
                this->setWidth(maxLength);
                outputPolicy::output("    OPTION HELP");
                std::for_each(m_commands.begin(), m_commands.end(), [maxLength](const auto& e){
                    outputPolicy::setWidth(maxLength);
                    outputPolicy::output("    " + e.first + " " + e.second->getArgString());
                    outputPolicy::output(e.second->m_helpText);});
                return false;
            }

            std::shared_ptr<commandBase> sliding = nullptr;
            for (auto argument = m_arguments.begin(); argument != m_arguments.end()-1; argument++) {
                if(isCommand(*argument) && m_commands.find(*argument) != m_commands.end()) {
                    if(sliding != nullptr && sliding->m_commandType == COMMAND_TYPE_POSITIONAL) {
                        outputPolicy::output("ERROR: ARGUMENTS REQUIRED FOR FLAG. SEE --usage");
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

                outputPolicy::output("MISMATCHED ARGUMENT SPECIFIED: " + *argument + ". SEE --usage");
                return false;
            }

            if(sliding && sliding->m_commandType == COMMAND_TYPE_POSITIONAL) {
                outputPolicy::output("ERROR: ARGUMENTS REQUIRED FOR FLAG. SEE --usage");
                return false;
            }

            m_queryString = *m_arguments.rbegin();
            return true;
        }

        std::string m_queryString;
        std::vector<std::string> m_arguments;
        std::map<std::string, std::shared_ptr<commandBase>> m_commands;
};
