/*
 * ============================================================================
 *           Date: AD 2012
 *         Author: Michał Szczepankiewicz
 *        Project: Zukala
 *        Version: 1.0
 *       Compiler: GCC (>=4.8), Clang (3.4)
 *          Notes: c++11x
 *
 *    Description:
 * ============================================================================
 */

#ifndef UTU_DETAILS_TASK_SUPPORT_H
#define UTU_DETAILS_TASK_SUPPORT_H

#include "Logger.h"

#define LOGGER_ADD_TASK_NAME(name) BOOST_LOG_SCOPED_THREAD_ATTR("Task", attrs::constant<std::string>(name));

#endif // UTU_TASK_SUPPORT_H
