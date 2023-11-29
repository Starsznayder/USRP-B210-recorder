/*
 * ============================================================================
 *           Date: AD 2019
 *         Author: Marek Ciesielski
 *        Project: Utu
 *        Version: 1.0
 *       Compiler: GCC (>=4.8), Clang (3.4)
 *          Notes: c++11x
 *
 *    Description:
 * ============================================================================
 */

#ifndef UTU_DETAILS_COLOR_H
#define UTU_DETAILS_COLOR_H

#include <boost/log/trivial.hpp>

namespace utu {
namespace details {

struct Color
{
    using value_type = const char*;
    constexpr static const char* RESET = "\033[0m";
    constexpr static const char* BLACK = "\033[30m";
    constexpr static const char* RED = "\033[31m";
    constexpr static const char* GREEN = "\033[32m";
    constexpr static const char* YELLOW  = "\033[33m";
    constexpr static const char* BLUE = "\033[34m";
    constexpr static const char* MAGENTA = "\033[35m";
    constexpr static const char* CYAN = "\033[36m";
    constexpr static const char* WHITE  = "\033[37m";
    constexpr static const char* BOLDBLACK = "\033[1m\033[30m";
    constexpr static const char* BOLDRED  = "\033[1m\033[31m" ;
    constexpr static const char* BOLDGREEN = "\033[1m\033[32m";
    constexpr static const char* BOLDYELLOW  = "\033[1m\033[33m";
    constexpr static const char* BOLDBLUE  =  "\033[1m\033[34m";
    constexpr static const char* BOLDMAGENTA = "\033[1m\033[35m";
    constexpr static const char* BOLDCYAN  = "\033[1m\033[36m";
    constexpr static const char* BOLDWHITE = "\033[1m\033[37m";
};

namespace logging = boost::log;

template <typename Level = logging::trivial::severity_level>
Color::value_type severityColor(Level lvl)
{
    switch (lvl)
    {
    case Level::trace:
        return Color::CYAN;
    case Level::debug:
        return Color::BOLDYELLOW;
    case Level::info:
        return Color::GREEN;
    case Level::warning:
        return Color::YELLOW;
    case Level::error:
        return Color::RED;
    case Level::fatal:
        return Color::BOLDRED;
    default:
        return Color::RESET;
    }
}

}
}

#endif /* UTU_DETAILS_COLOR_H */
