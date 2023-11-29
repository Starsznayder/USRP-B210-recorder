#ifndef CONFIGEXCEPTION_H
#define CONFIGEXCEPTION_H

#include <string>
#include <stdexcept>

namespace config{

    struct ConfigFileException : public std::runtime_error
    {
        ConfigFileException(const std::string& msg)
            : std::runtime_error("config error: " + msg + ".") {}
    };

}

#endif //CONFIGEXCEPTION_H
