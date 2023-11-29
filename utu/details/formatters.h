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
#ifndef UTU_DETAILS_FORMATTERS_H
#define UTU_DETAILS_FORMATTERS_H

#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/core.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "Color.h"

namespace utu {
namespace details {

namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(task, "Task", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(tagParam, "Param", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(tagThreadID, "ThreadID", attrs::current_thread_id::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(processID, "ProcessID", logging::attributes::current_process_id::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(tagStatus, "Status", std::string)

template <typename Level = boost::log::trivial::severity_level>
struct BasicFormatter
{
   struct severity_tag_start{};
   struct severity_tag_stop{};
   using level_type = Level;

   static logging::formatter get()
   {
        return expr::stream << " | <" << logging::trivial::severity << "> : "
                            << "{" << expr::attr<std::string>("Process") << " : "
                            << expr::attr<attrs::current_process_id::value_type>("ProcessID") << "."
                            << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << "} "
                            << expr::if_(expr::has_attr(task)) [ expr::stream << "[" << task << "] " ]
                            << "(" << expr::attr<std::string>("File") << ":"
                            << expr::attr<std::string>("Function") << ":"
                            << expr::attr<int>("Line") << ") "
                            << boost::log::expressions::message;
    }

};

template <typename Level = boost::log::trivial::severity_level>
struct ColorFormatter
{
   struct severity_tag_start{};
   struct severity_tag_stop{};
   using level_type = Level;

   static logging::formatter get()
   {
        return expr::stream << expr::attr<level_type, severity_tag_start>("Severity")
                            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                                                      "%H:%M:%S.%f")
                            << " : <" << logging::trivial::severity << "> : "
                            << "{" << expr::attr<std::string>("Process") << " : "
                            << expr::attr<attrs::current_process_id::value_type>("ProcessID") << "."
                            << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << "} "
                            << expr::if_(expr::has_attr(task)) [ expr::stream << "[" << task << "] " ]
                            << "(" << expr::attr<std::string>("File") << ":"
                            << expr::attr<std::string>("Function") << ":"
                            << expr::attr<int>("Line") << ") "
                            << boost::log::expressions::message
                            << expr::attr<level_type, severity_tag_stop>("Severity");
    }

};

template <typename Level = boost::log::trivial::severity_level>
struct StatusFormatter
{
   struct severity_tag_start{};
   struct severity_tag_stop{};
   using level_type = Level;

   static logging::formatter get()
   {
        return expr::stream << ": <" << boost::log::trivial::severity << "> "
                            << ": {" << expr::attr<std::string>("Process") << "}"
                            << expr::if_(expr::has_attr(tagStatus)) [ expr::stream << "#" << tagStatus << "# " ]
                            << expr::format_named_scope("Scope", keywords::format = " (%c:%l) ", keywords::delimiter = "", keywords::depth = 1)
                            << boost::log::expressions::message;
    }

};

template <typename Level = boost::log::trivial::severity_level>
logging::formatting_ostream& operator<<(logging::formatting_ostream& strm, logging::to_log_manip<typename ColorFormatter<Level>::level_type, typename ColorFormatter<Level>::severity_tag_start> const& manip)
{
    auto level = manip.get();
    strm << severityColor<Level>(level);
    return strm;
}

template <typename Level = boost::log::trivial::severity_level>
logging::formatting_ostream& operator<<(logging::formatting_ostream& strm, logging::to_log_manip<typename ColorFormatter<Level>::level_type, typename ColorFormatter<Level>::severity_tag_stop > const&)
{
    strm << Color::RESET;
    return strm;
}

}
}

#endif /* UTU_DETAILS_FORMATTERS_H */
