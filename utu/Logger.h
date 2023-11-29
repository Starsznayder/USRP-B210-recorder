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
#ifndef UTU_LOG_DETAILS_LOGGER_H
#define UTU_LOG_DETAILS_LOGGER_H


#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <mutex>


#include "details/sinks.h"

namespace src = boost::log::sources;
namespace attrs = boost::log::attributes;
namespace logging = boost::log;

namespace utu {

typedef uint32_t uint32_t;
typedef uint32_t elem_num;
using fixed_bool = uint8_t;


template <typename Level = logging::trivial::severity_level>
struct Logger
{
    constexpr static details::ConsoleSink<Level> consoleSink = details::ConsoleSink<Level>();
    constexpr static details::NetworkSink<Level> networkSink = details::NetworkSink<Level>();
    constexpr static details::FileSink<Level> fileSink = details::FileSink<Level>();
    constexpr static details::StatusSink<Level> statusSink = details::StatusSink<Level>();

    using level_type = Level;
    using logger_type = src::severity_logger_mt<level_type>;

    static void setProcess(const std::string& processName)
    {
        //std::call_once(initFlag_, std::bind(&Logger<Level>::__init));
        //lg_.add_attribute("Process", attrs::constant<std::string>(processName));
    }

    template<typename sink>
    static void init(sink, const std::string& addr = "235.0.0.111", const uint16_t& port = 37942)
    {
        std::call_once(initFlag_, std::bind(&Logger<Level>::__init));
        logging::core::get()->add_sink(sink::get(addr, port));
    }

    template<typename sink>
    static void init(sink, const std::string& processName, const std::string& path, const std::string& file, const uint32_t& maxSize, const uint32_t& fileNum)
    {
        std::call_once(initFlag_, std::bind(&Logger<Level>::__init));
        logging::core::get()->add_sink(sink::get(processName, path, file, maxSize, fileNum));
    }

    static logger_type& get() { return lg_; }

    Logger() = delete;
    Logger(const Logger&) = delete;

    struct NullLogger
    {
        template <typename T>
        inline NullLogger& operator<<(const T&) { return *this; }

        static NullLogger& get() { return nlg_; }
    private:
        NullLogger() = default;
        NullLogger(const NullLogger&) = default;
        static NullLogger nlg_;
    };

    static const char* CutFilePath(const char *filename);

private:
    static void __init();

private:
    static logger_type lg_;
    static std::once_flag initFlag_;
};

template <typename Level>
typename Logger<Level>::logger_type Logger<Level>::lg_ = Logger<Level>::logger_type();

template <typename Level>
std::once_flag Logger<Level>::initFlag_;

template <typename Level>
typename Logger<Level>::NullLogger Logger<Level>::NullLogger::nlg_ = Logger<Level>::NullLogger();


template <typename Level>
void Logger<Level>::__init()
{
    logging::add_common_attributes();
    lg_.add_attribute("Scope", attrs::named_scope());
}

template <typename Level>
const char* Logger<Level>::CutFilePath(const char *filename)
{
    const char *ptr = strrchr(filename, '/');
    if (ptr != nullptr)
    {
        return ptr+1;
    }

    ptr = strrchr(filename, '\\');
    if (ptr != nullptr)
    {
        return ptr+1;
    }
    return filename;
}

} /* namespace utu */

#endif /* UTU_LOGGER_H */
