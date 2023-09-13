// #include "util.hpp"
// #include "level.hpp"
// #include "message.hpp"
// #include "format.hpp"
// #include "sink.hpp"
// #include "logger.hpp"
// #include "buffer.hpp"
#include "../logs/zbwlog.h"
#include <unistd.h>

/*
扩展一个以时间作为文件滚动切换类型的日志落地模块
    1、以时间进行文件滚动，实际上是以时间段进行滚动
       实现思想：以当前系统时间，取模时间段大小，可以得到当前时间段是第几个时间段
               每次以当前系统时间取模，判断与当前文件的时间段是否一致，不一致代表不是同一个时间段
*/
enum  class TimeGap
{
    GAP_SECOND,
    GAP_MINUTE,
    GAP_HOUR,
    GAP_DAY
};

class RollByTimeSink : public zbwlog::LogSink
    {
    public:
        //构造时传入文件名，并打开文件，将操作句柄管理起来
        RollByTimeSink(const std::string &basename, TimeGap gap_type)
            :_basename(basename)
        {
            switch (gap_type)
            {
                case TimeGap::GAP_SECOND: _gap_size = 1; break;
                case TimeGap::GAP_MINUTE: _gap_size = 60; break;
                case TimeGap::GAP_HOUR: _gap_size = 3600; break;
                case TimeGap::GAP_DAY: _gap_size = 3600*24; break;
            }
            _cur_gap = _gap_size == 1 ? zbwlog::util::Date::now() : zbwlog::util::Date::now() % _gap_size;//获取当前是第几个时间段
            std::string filename = createNewFile();
            zbwlog::util::File::createDirectory(zbwlog::util::File::path(filename));
            _ofs.open(filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }

        //将日志消息写入到标准输出,判断当前时间是否是当前文件的时间段，不是则切换文件
        void log(const char* data, size_t len)
        {
            time_t cur = zbwlog::util::Date::now();
            if((cur % _gap_size) != _cur_gap)
            {
                _ofs.close();
                std::string filename = createNewFile();
                _ofs.open(filename, std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
            } 
            _ofs.write(data, len);
            assert(_ofs.good());
        }
    private:
        std::string createNewFile()
        {
            time_t t = zbwlog::util::Date::now();
            struct tm lt;
            localtime_r(&t, &lt);
            std::stringstream filename;
            filename << _basename;
            filename << lt.tm_year + 1900;
            filename << lt.tm_mon + 1;
            filename << lt.tm_mday;
            filename << lt.tm_hour;
            filename << lt.tm_min;
            filename << lt.tm_sec;
            filename << ".log";
            return filename.str();
        }
    
    private:
        std::string _basename;
        std::ofstream _ofs;
        size_t _cur_gap; //当前是第几个时间段
        size_t _gap_size; //时间段大小
    };

void test_log()
{
    // zbwlog::Logger::ptr logger = zbwlog::LoggerManager::getInstance().getLogger("async_logger");
    // logger->debug("%s", "测试日志");
    // logger->info("%s", "测试日志");
    // logger->warn("%s", "测试日志");
    // logger->error("%s", "测试日志");
    // logger->fatal("%s", "测试日志");

    // size_t cursize = 0;
    // size_t count = 0;
    // std::string str = "测试日志";
    // while(cursize < 1024*1024*10)
    // {
    //     logger->fatal(__FILE__, __LINE__, "测试日志-%d", count++);
    //     cursize += 20;
    // }
    
    DEBUG("%s", "测试日志");
    INFO("%s", "测试日志");
    WARN("%s", "测试日志");
    ERROR("%s", "测试日志");
    FATAL("%s", "测试日志");

    
    size_t count = 0;
    
    while(count < 500000)
    {
        FATAL("测试日志"-"%d", count++);
    }
}

int main()
{
    //读取文件数据，一点一点写入缓冲区，最终将缓冲区数据写入文件，判断生成的新文件与源文件是否一致
    // std::ifstream ifs("./logfile/test.log", std::ios::binary);
    // if(ifs.is_open() == false) return -1;
    // ifs.seekg(0, std::ios::end); //读写位置跳转到文件末尾
    // size_t fsize = ifs.tellg(); //获取当前读写位置相对于起始位置的偏移量
    // ifs.seekg(0, std::ios::beg); //重新跳转到起始位置
    // std::string body;
    // body.resize(fsize);
    // ifs.read(&body[0], fsize);
    // if(ifs.good() == false)
    // {
    //     std::cout << "read error\n";
    //     return -1;
    // }
    // ifs.close();

    // zbwlog::Buffer buffer;
    // for(int i = 0; i < body.size(); ++i)
    // {
    //     buffer.push(&body[i], 1);
    // }
    // std::ofstream ofs("./logfile/tmp.log", std::ios::binary);
    // size_t rsize = buffer.readAbleSize();
    // for(int i = 0; i < rsize; ++i)
    // {
    //     ofs.write(buffer.begin(), 1);
    //     buffer.moveReader(1);
    // }
    // ofs.close();

    // std::string logger_name = "sync_logger";
    // zbwlog::LogLevel::value limit = zbwlog::LogLevel::value::WARN;
    // zbwlog::Formatter::ptr fmt(new zbwlog::Formatter("[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n"));
    // zbwlog::LogSink::ptr stdout_lsp = zbwlog::SinkFactory::create<zbwlog::StdoutSink>();
    // zbwlog::LogSink::ptr file_lsp = zbwlog::SinkFactory::create<zbwlog::FileSink>("./logfile/test.log");
    // zbwlog::LogSink::ptr roll_lsp = zbwlog::SinkFactory::create<zbwlog::RollBySizeSink>("./logfile/roll-", 1024*1024);
    // std::vector<zbwlog::LogSink::ptr> sinks = {stdout_lsp, file_lsp, roll_lsp};
    // zbwlog::Logger::ptr logger(new zbwlog::Synclogger(logger_name, limit, fmt, sinks));

    std::unique_ptr<zbwlog::LoggerBuilder> builder(new zbwlog::GlobalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(zbwlog::LogLevel::value::WARN);
    builder->buildFormatter("[%c]%m%n");
    builder->buildLoggerType(zbwlog::LoggerType::LOGGER_ASYNC);
    builder->buildSink<zbwlog::FileSink>("./logfile/async.log");
    builder->buildSink<zbwlog::StdoutSink>();
    builder->build();
    test_log();

    // zbwlog::LogMsg msg(zbwlog::LogLevel::value::INFO, 53, "main.c", "root", "格式化功能测试。。。");
    // zbwlog::Formatter fmt;
    // std::string str = fmt.format(msg);

    // // zbwlog::LogSink::ptr stdout_lsp = zbwlog::SinkFactory::create<zbwlog::StdoutSink>();
    // // zbwlog::LogSink::ptr file_lsp = zbwlog::SinkFactory::create<zbwlog::FileSink>("./logfile/test.log");
    // // zbwlog::LogSink::ptr roll_lsp = zbwlog::SinkFactory::create<zbwlog::RollBySizeSink>("./logfile/roll-", 1024*1024);
    // zbwlog::LogSink::ptr time_lsp = zbwlog::SinkFactory::create<RollByTimeSink>("./logfile/roll-", TimeGap::GAP_SECOND);
    
    // time_t old = zbwlog::util::Date::now();
    // while(zbwlog::util::Date::now() < old + 5)
    // {
    //     time_lsp->log(str.c_str(), str.size());
    //     usleep(1000);
    // }

    // stdout_lsp->log(str.c_str(), str.size());
    // file_lsp->log(str.c_str(), str.size());
    // size_t cursize = 0;
    // size_t count = 0;
    // while(cursize < 1024*1024*10)
    // {
    //     std::string tmp = str + std::to_string(count++);
    //     roll_lsp->log(tmp.c_str(), tmp.size());
    //     cursize += tmp.size();
    // }


    //std::cout << str << std::endl;

    // std::cout<<zbwlog::LogLevel::toString(zbwlog::LogLevel::value::DEBUG)<<std::endl;
    
    // std::cout<<zbwlog::util::Date::now()<<std::endl;
    // std::string pathname = "./abc/bcd/a.txt";
    // zbwlog::util::File::createDirectory(zbwlog::util::File::path(pathname));
    return 0;
}