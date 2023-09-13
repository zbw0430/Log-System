#ifndef __M_FMT_H__
#define __M_FMT_H__

#include "level.hpp"
#include "message.hpp"
#include <iostream>
#include <ctime>
#include <vector>
#include <cassert>
#include <sstream>
#include <string>

namespace zbwlog
{
    //抽象格式化子项基类
    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream &out, const LogMsg &msg) = 0;
    };

    //派生格式化子项子类--消息、等级、时间、文件名、行号、线程ID、日志器名、制表符、换行、其他
    class MsgFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._payload;
        }
    };

    class LevelFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << LogLevel::toString(msg._level);
        }
    };

    class TimeFormatItem : public FormatItem
    {
    public:
        TimeFormatItem(const std::string &fmt = "%H:%M:%S"):_time_fmt(fmt){}
        void format(std::ostream &out, const LogMsg &msg) override
        {
            struct tm t;
            localtime_r(&msg._ctime, &t);
            char tmp[32] = {0};
            strftime(tmp, 31, _time_fmt.c_str(), &t);//将时间结构体转换为指定的字符串格式
            out << tmp;
        }
    private:
        std::string _time_fmt; //%H:%M:%S
    };

    class FileFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._file;
        }
    };

    class LineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._line;
        }
    };

    class ThreadFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._tid;
        }
    };

    class LoggerFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._logger;
        }
    };

    class TabFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << "\t";
        }
    };

    class NLineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << "\n";
        }
    };

    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string &str):_str(str){}
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << _str;
        }
    private:
        std::string _str;
    };

    /*
        %d 表示日期，包含子格式{%H:%M:%S}
        %t 表示线程ID
        %c 表示日志器名称
        %f 表示源码文件名
        %l 表示源码行号
        %p 表示日志级别
        %T 表示制表符缩进
        %m 表示主体消息
        %n 表示换行
    */

    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n"):
            _pattern(pattern)
        {
            assert(parsePattern());
        }
        
        //对msg进行格式化
        void format(std::ostream &out, const LogMsg &msg)
        {
            for(auto &item : _items)
            {
                item->format(out, msg);
            }
        }

        std::string format(const LogMsg &msg)
        {
            std::stringstream ss;
            format(ss, msg);
            return ss.str();
        }
    
    private:
        //对格式化规则字符串进行解析
        bool parsePattern()
        {
            //1、对格式化规则字符串进行解析
            std::vector<std::pair<std::string, std::string>> fmt_order;
            size_t pos = 0;
            std::string key, val;
            while(pos < _pattern.size())
            {
                //1、处理原始字符--判断是否是%，不是就是原始字符
                if(_pattern[pos] != '%')
                {
                    val.push_back(_pattern[pos++]);
                    continue;
                }
                //如果pos位置是%
                //%%
                if(pos+1 < _pattern.size() && _pattern[pos+1] == '%')
                {
                    val.push_back('%');
                    pos += 2;
                    continue;
                }

                //原始字符串处理完毕后存入vector
                if(val.empty() == false) fmt_order.push_back(std::make_pair("", val));
                val.clear();

                //处理格式化字符
                pos += 1;
                if(pos == _pattern.size())
                {
                    std::cout << "%之后没有对应的格式化字符\n";
                    return false;
                }
                key = _pattern[pos];
                pos += 1;

                if(pos < _pattern.size() && _pattern[pos] == '{')
                {
                    pos += 1;
                    while(pos < _pattern.size() && _pattern[pos] != '}')
                    {
                        val.push_back(_pattern[pos++]);
                    }
                    if(pos == _pattern.size())
                    {
                        std::cout << "子规则{}匹配出错\n";
                        return false; //没找到 }
                    }
                    pos += 1;
                }

                fmt_order.push_back(std::make_pair(key, val));
                key.clear();
                val.clear();
                


            }
            //2、根据解析得到的数据初始化格式化子项数组成员
            for(auto &it : fmt_order)
            {
                _items.push_back(createItem(it.first, it.second));
            }

            return true;
            
        }
    private:
        //根据不同的格式化字符创建不同的格式化子项对象
        FormatItem::ptr createItem(const std::string &key, const std::string &val)
        {
            if(key == "d") return std::make_shared<TimeFormatItem>(val);
            if(key == "t") return std::make_shared<ThreadFormatItem>();
            if(key == "c") return std::make_shared<LoggerFormatItem>();
            if(key == "f") return std::make_shared<FileFormatItem>();
            if(key == "l") return std::make_shared<LineFormatItem>();
            if(key == "p") return std::make_shared<LevelFormatItem>();
            if(key == "T") return std::make_shared<TabFormatItem>();
            if(key == "m") return std::make_shared<MsgFormatItem>();
            if(key == "n") return std::make_shared<NLineFormatItem>();
            if(key == "") return std::make_shared<OtherFormatItem>(val);
            std::cout << "没有对应的格式化字符：%" << key << std::endl;
            abort();
            return FormatItem::ptr();
        }

    private:
        std::string _pattern;//格式化规则字符串
        std::vector<FormatItem::ptr> _items;

    };
}

#endif