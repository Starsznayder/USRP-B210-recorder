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
#ifndef UTU_DETAILS_SINKS_H
#define UTU_DETAILS_SINKS_H


#include <boost/log/sinks.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

#include "formatters.h"

namespace utu {
namespace details {

namespace sinks = boost::log::sinks;

template <typename Level = boost::log::trivial::severity_level>
struct ConsoleSink
{
    typedef sinks::text_ostream_backend Backend;
    typedef sinks::synchronous_sink<Backend> Sink;
    typedef boost::shared_ptr<Sink> SinkPtr;

    static SinkPtr get(const std::string&, const uint16_t&)
    {
        SinkPtr spSink = boost::make_shared<Sink>();

        spSink->locked_backend()->add_stream(boost::shared_ptr< std::ostream >(&std::cout, boost::null_deleter()));
        spSink->locked_backend()->auto_flush(true);
        spSink->set_formatter(details::ColorFormatter<Level>::get());
        return spSink;
    }
};

template <typename Level = boost::log::trivial::severity_level>
struct NetworkSink
{
    typedef sinks::syslog_backend Backend;
    typedef sinks::synchronous_sink<Backend> Sink;
    typedef boost::shared_ptr<Sink> SinkPtr;

    static SinkPtr get(const std::string& addr, const uint16_t& port)
    {
        SinkPtr spSink = boost::make_shared<Sink>();

        spSink->locked_backend()->set_target_address(addr, port);
        spSink->reset_formatter();
        spSink->set_formatter(details::BasicFormatter<Level>::get());
        return spSink;
    }
};

template <typename Level = boost::log::trivial::severity_level>
struct FileSink
{
    typedef sinks::text_file_backend Backend;
    typedef sinks::synchronous_sink<Backend> Sink;
    typedef boost::shared_ptr<Sink> SinkPtr;

    static SinkPtr get(const std::string& processName, const std::string& path, const std::string& file, const uint32_t& maxSize, const uint32_t& fileNum)
    {
        SinkPtr spSink = boost::make_shared<Sink>();

        boost::filesystem::path pattern(processName + file + "_%Y-%m-%d_%H-%M-%S.%N.log");

        spSink->locked_backend()->auto_flush(true);
        spSink->locked_backend()->set_rotation_size(maxSize);
        spSink->locked_backend()->set_file_name_pattern(pattern);

        spSink->locked_backend()->set_file_collector
        (
            sinks::file::make_collector
            (
                keywords::target = path,
                keywords::max_size = maxSize * fileNum
            )
        );
        spSink->set_formatter(details::BasicFormatter<Level>::get());
        return spSink;
    }
};

template <typename Level = boost::log::trivial::severity_level>
struct StatusSink
{
    typedef sinks::syslog_backend Backend;
    typedef sinks::synchronous_sink<Backend> Sink;
    typedef boost::shared_ptr<Sink> SinkPtr;

    static SinkPtr get(const std::string& addr, const uint16_t& port)
    {
        SinkPtr spSink = boost::make_shared<Sink>();

        spSink->set_filter((expr::has_attr(tagStatus)));
        spSink->locked_backend()->set_target_address(addr, port);
        spSink->reset_formatter();
        spSink->set_formatter(details::StatusFormatter<Level>::get());
        return spSink;
    }
};

}
}

#endif /* UTU_DETAILS_SINKS_H */
