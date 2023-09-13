/*
完成日志器模块：
    1、抽象日志器基类
    2、派生出不同的子类（同步日志器 & 异步日志器）
*/
#ifndef __M_LOGGER_H__
#define __M_LOGGER_H__

//#define _GNU_SOURCE
#include "util.hpp"
#include "level.hpp"
#include "format.hpp"
#include "sink.hpp"
#include <atomic>
#include <mutex>
#include <cstdarg>
#include "looper.hpp"
#include <functional>
#include <unordered_map>

namespace zbwlog
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name,
            LogLevel::value level,
            Formatter::ptr &formatter,
            std::vector<LogSink::ptr> &sinks):
                _logger_name(logger_name), 
                _limit_level(level), 
                _formatter(formatter),
                _sinks(sinks.begin(), sinks.end()){}
        
        const std::string &name()
        {
            return _logger_name;
        }

        //完成构造日志消息对象过程并进行格式化，得到格式化后的日志消息字符串--然后进行落地输出
        void debug(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            //通过传入的参数构造出一个日志消息对象，进行日志的格式化，最终落地
            //1、判断当前日志是否达到了输出等级
            if(LogLevel::value::DEBUG < _limit_level) { return; }

            //2、对fmt格式化字符串和不定参进行字符串组织，得到日志消息的字符串
            va_list ap;
            va_start(ap, fmt);
            char* res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if(ret == -1)
            {
                std::cout << "vasprintf failed\n";
                return;
            }
            va_end(ap);

            // //3、构造LogMsg对象
            // LogMsg msg(LogLevel::value::DEBUG, line, file, _logger_name, res);
            
            // //4、通过格式化工具对LogMsg进行格式化，得到格式化后的日志字符串
            // std::stringstream ss;
            // _formatter->format(ss, msg);

            // //5、进行日志落地
            // log(ss.str().c_str(), ss.str().size());
            // free(res);

            serialize(LogLevel::value::DEBUG, file, line, res);
            free(res);
        }

        void info(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            //通过传入的参数构造出一个日志消息对象，进行日志的格式化，最终落地
            //1、判断当前日志是否达到了输出等级
            if(LogLevel::value::INFO < _limit_level) { return; }

            //2、对fmt格式化字符串和不定参进行字符串组织，得到日志消息的字符串
            va_list ap;
            va_start(ap, fmt);
            char* res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if(ret == -1)
            {
                std::cout << "vasprintf failed\n";
                return;
            }
            va_end(ap);

            serialize(LogLevel::value::INFO, file, line, res);
            free(res);
        }

        void warn(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            //通过传入的参数构造出一个日志消息对象，进行日志的格式化，最终落地
            //1、判断当前日志是否达到了输出等级
            if(LogLevel::value::WARN < _limit_level) { return; }

            //2、对fmt格式化字符串和不定参进行字符串组织，得到日志消息的字符串
            va_list ap;
            va_start(ap, fmt);
            char* res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if(ret == -1)
            {
                std::cout << "vasprintf failed\n";
                return;
            }
            va_end(ap);

            serialize(LogLevel::value::WARN, file, line, res);
            free(res);
        }
        void error(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            //通过传入的参数构造出一个日志消息对象，进行日志的格式化，最终落地
            //1、判断当前日志是否达到了输出等级
            if(LogLevel::value::ERROR < _limit_level) { return; }

            //2、对fmt格式化字符串和不定参进行字符串组织，得到日志消息的字符串
            va_list ap;
            va_start(ap, fmt);
            char* res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if(ret == -1)
            {
                std::cout << "vasprintf failed\n";
                return;
            }
            va_end(ap);

            serialize(LogLevel::value::ERROR, file, line, res);
            free(res);
        }

        void fatal(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            //通过传入的参数构造出一个日志消息对象，进行日志的格式化，最终落地
            //1、判断当前日志是否达到了输出等级
            if(LogLevel::value::FATAL < _limit_level) { return; }

            //2、对fmt格式化字符串和不定参进行字符串组织，得到日志消息的字符串
            va_list ap;
            va_start(ap, fmt);
            char* res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if(ret == -1)
            {
                std::cout << "vasprintf failed\n";
                return;
            }
            va_end(ap);

            serialize(LogLevel::value::FATAL, file, line, res);
            free(res);
        }
    
    protected:
        void serialize(LogLevel::value level, const std::string &file, size_t line, char* str)
        {
            //3、构造LogMsg对象
            LogMsg msg(level, line, file, _logger_name, str);
            
            //4、通过格式化工具对LogMsg进行格式化，得到格式化后的日志字符串
            std::stringstream ss;
            _formatter->format(ss, msg);

            //5、进行日志落地
            log(ss.str().c_str(), ss.str().size());
        }
        //抽象接口完成实际的落地输出--不同的日志器会有不同的实际落地方式
        virtual void log(const char* data, size_t len) = 0;
    protected:
        std::mutex _mutex;
        std::string _logger_name;
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class Synclogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<Synclogger>;
        Synclogger(const std::string &logger_name,
            LogLevel::value level,
            Formatter::ptr &formatter,
            std::vector<LogSink::ptr> &sinks):
            Logger(logger_name, level, formatter, sinks){}
    protected:
        //同步日志器，是将日志直接通过落地模块句柄进行日志落地
        void log(const char* data, size_t len)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if(_sinks.empty()) return;
            for(auto &sink : _sinks)
            {
                sink->log(data, len);
            }
        }
    };

    class AsyncLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<AsyncLogger>;
        AsyncLogger(const std::string &logger_name,
            LogLevel::value level,
            Formatter::ptr &formatter,
            std::vector<LogSink::ptr> &sinks,
            AsyncType looper_type):
            Logger(logger_name, level, formatter, sinks),
            _looper(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::realLog, this, std::placeholders::_1), looper_type)){}
        
        void log(const char* data, size_t len)//将数据写入缓冲区
        {
            _looper->push(data, len);
        }

        //设计一个实际落地函数（将缓冲区中的数据落地）
        void realLog(Buffer &buf)
        {
            if(_sinks.empty()) return;
            for(auto &sink : _sinks)
            {
                sink->log(buf.begin(), buf.readAbleSize());
            }
        }
    protected:
        AsyncLooper::ptr _looper;
    };

    enum class LoggerType
    {
        LOGGER_SYNC = 0,
        LOGGER_ASYNC
    };
    //使用建造者模式来建造日志器，而不要让用户直接去构造日志器，简化用户的使用复杂度
    //1、抽象一个日志器建造者类（完成日志器对象所需零部件的构建 & 日志器构建）
    //   1、设置日志器类型
    //   2、将不同类型日志器的创建放到同一个日志器建造者类中完成
    class LoggerBuilder
    {
    public:
        LoggerBuilder(): 
            _logger_type(LoggerType::LOGGER_SYNC),
            _limit_level(LogLevel::value::DEBUG),
            _looper_type(AsyncType::ASYNC_SAFE){}

        void buildLoggerType(LoggerType type) {_logger_type = type;}

        void buildLoggerName(const std::string &name) {_logger_name = name;}

        void buildLoggerLevel(LogLevel::value level) {_limit_level = level;}

        void buildFormatter(const std::string &pattern)
        {
            _formatter = std::make_shared<Formatter>(pattern);
        }

        void buildEnableUnsafeAsync() {_looper_type = AsyncType::ASYNC_UNSAFE;}

        template<typename SinkType, typename ...Args>
        void buildSink(Args &&...args)
        {
            LogSink::ptr psink = SinkFactory::create<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(psink);
        }
        virtual Logger::ptr build() = 0;

    protected:
        LoggerType _logger_type;
        std::string _logger_name;
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
        AsyncType _looper_type;
    };

    //2、派生出具体的建造者类---局部日志器的建造者 & 全局日志器建造者 （后面添加了全局单例管理器之后，将日志器添加全局管理）
    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(_logger_name.empty() == false);//必须有日志器名称
            if(_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            if(_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            if(_logger_type == LoggerType::LOGGER_ASYNC)
            {
                return std::make_shared<AsyncLogger>(_logger_name, _limit_level, _formatter, _sinks, _logger_type);
            }
            return std::make_shared<Synclogger>(_logger_name, _limit_level, _formatter, _sinks);
        }
    };

    class LoggerManager
    {
    public:
        static LoggerManager& getInstance()
        {
            static LoggerManager eton;
            return eton;
        }

        void addLogger(Logger::ptr &logger)
        {
            if(hasLogger(logger->name())) return;
            std::unique_lock<std::mutex> lock(_mutex);
            _loggers.insert(std::make_pair(logger->name(), logger));
        }

        bool hasLogger(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _loggers.find(name);
            if(it == _loggers.end()) return false;
            return true;
        }

        Logger::ptr getLogger(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _loggers.find(name);
            if(it == _loggers.end()) return Logger::ptr();
            return it->second;
        }

        Logger::ptr rootLogger()
        {
            return _root_logger;
        }
    private:
        LoggerManager()
        {
            std::unique_ptr<zbwlog::LoggerBuilder> builder(new zbwlog::LocalLoggerBuilder());
            builder->buildLoggerName("root");
            _root_logger = builder->build();
            _loggers.insert(std::make_pair("root", _root_logger));
        }
    private:
        std::mutex _mutex;
        Logger::ptr _root_logger;//默认日志器
        std::unordered_map<std::string, Logger::ptr> _loggers;

    };

    //设计一个全局日志器的建造者--在局部的基础上增加了一个功能：将日志器添加到单例对象中
    class GlobalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(_logger_name.empty() == false);//必须有日志器名称
            if(_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            if(_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            Logger::ptr logger;
            if(_logger_type == LoggerType::LOGGER_ASYNC)
            {
                logger = std::make_shared<AsyncLogger>(_logger_name, _limit_level, _formatter, _sinks, _logger_type);
            }
            else
            {
                logger = std::make_shared<Synclogger>(_logger_name, _limit_level, _formatter, _sinks);
            }
            LoggerManager::getInstance().addLogger(logger);
            return logger;
        }
    };
}

#endif