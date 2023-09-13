#ifndef __M_ZBWLOG_H__
#define __M_ZBWLOG_H__
#include "logger.hpp"

namespace zbwlog
{
    //1、提供获取指定日志器的全局接口（避免用户自己操作单例对象）
    Logger::ptr getLogger(const std::string &name)
    {
        return zbwlog::LoggerManager::getInstance().getLogger(name);
    }
    Logger::ptr rootLogger()
    {
        return zbwlog::LoggerManager::getInstance().rootLogger();
    }

    //2、使用宏函数对日志器的接口进行代理（代理模式）
    #define debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define info(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define warn(fmt, ...) warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define error(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define fatal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    
    //3、提供宏函数，直接通过默认日志器进行日志的标准输出打印（不同获取日志器了）
    #define DEBUG(fmt, ...) zbwlog::rootLogger()->debug(fmt, ##__VA_ARGS__)
    #define INFO(fmt, ...) zbwlog::rootLogger()->info(fmt, ##__VA_ARGS__)
    #define WARN(fmt, ...) zbwlog::rootLogger()->warn(fmt, ##__VA_ARGS__)
    #define ERROR(fmt, ...) zbwlog::rootLogger()->error(fmt, ##__VA_ARGS__)
    #define FATAL(fmt, ...) zbwlog::rootLogger()->fatal(fmt, ##__VA_ARGS__)

}


#endif