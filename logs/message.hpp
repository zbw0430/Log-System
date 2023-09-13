/*
    定义日志消息类，进行日志中间信息的存储：
        1、日志的输出时间      用于过滤日志输出时间
        2、日志等级           用于进行日志过滤分析
        3、源文件名称
        4、源代码行号         用于定位出现错误的代码位置
        5、线程ID            用于过滤出错的线程
        6、日志主体消息
        7、日志器名称        （当前支持多日志器的同时使用）

*/
#ifndef __M_MSG_H__
#define __M_MSG_H__

#include "level.hpp"
#include "util.hpp"
#include <iostream>
#include <string>
#include <thread>


namespace zbwlog
{
    struct LogMsg
    {
        time_t _ctime; //日志产生的时间戳
        LogLevel::value _level; //日志等级
        std::string _file; //源码文件名
        size_t _line; //行号
        std::thread::id _tid; //线程id
        std::string _logger; //日志器名称
        std::string _payload; //有效消息数据

        LogMsg( LogLevel::value level,
            size_t line,
            const std::string file,
            const std::string logger,
            const std::string msg):
            _ctime(util::Date::now()), 
            _level(level), _line(line), 
            _tid(std::this_thread::get_id()),
            _file(file), _logger(logger), _payload(msg)
        {}
    };
    
    
}


#endif