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

#ifndef UTU_LOG_H
#define UTU_LOG_H

#include "Logger.h"
#include "task_support.h"
#include <boost/log/utility/manipulators/add_value.hpp>

namespace utu {

using logger = utu::Logger<>;

}

#define CL_(lvl, proc) BOOST_LOG_NAMED_SCOPE(__FUNCTION__) \
BOOST_LOG_SEV((utu::logger::get()), lvl ) << boost::log::add_value("File", utu::logger::CutFilePath(__FILE__)) \
                                          << boost::log::add_value("Line", __LINE__) \
                                          << boost::log::add_value("Function", __FUNCTION__) \
                                          << boost::log::add_value("Process", proc)

//Rozwiązanie bezpieczne
#define _Dsafety(type, proc, message) {std::ostringstream ss; ss << message; CL_(type, proc) << ss.str();}
#define _DT(proc, message) _Dsafety(utu::logger::level_type::trace, proc, message)
#define _DI(proc, message) _Dsafety(utu::logger::level_type::info, proc, message)
#define _DD(proc, message) _Dsafety(utu::logger::level_type::debug, proc, message)
#define _DW(proc, message) _Dsafety(utu::logger::level_type::warning, proc, message)
#define _DE(proc, message) _Dsafety(utu::logger::level_type::error, proc, message)
#define _DC(proc, message) _Dsafety(utu::logger::level_type::fatal, proc, message)

#endif /* UTU_LOG_H */
