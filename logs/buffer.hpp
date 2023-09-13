/*
实现异步日志缓冲区
*/
#ifndef __M_BUF_H__
#define __M_BUF_H__

#include "util.hpp"
#include <vector>
#include <cassert>

namespace zbwlog
{
    #define DEFAUIT_BUFFER_SIZE (1*1024*1024)
    #define THRESHOLD_BUFFER_SIZE (8*1024*1024)
    #define INCREMENT_BUFFER_SIZE (1*1024*1024)
    class Buffer
    {
    public:
        Buffer() : _buffer(DEFAUIT_BUFFER_SIZE), _writer_idx(0), _reader_idx(0){}

        //向缓冲区写入数据
        void push(const char* data, size_t len)
        {
            //缓冲区剩余空间不够的情况：1、扩容 2、阻塞/返回false
            //if(len > writeAbleSize()) return;
            ensureEnoughSize(len);
            //1、将数据拷贝到缓冲区
            std::copy(data, data+len, &_buffer[_writer_idx]);
            //2、将当前位置向后偏移
            moveWriter(len);
        }

        //获取可写空间大小
        size_t writeAbleSize()//针对固定大小缓冲区
        {
            return (_buffer.size()-_writer_idx);
        }

        //返回可读数据的起始地址
        const char* begin()
        {
            return &_buffer[_reader_idx];
        }

        //返回可读数据的长度
        size_t readAbleSize()
        {
            //因为当前实现的缓冲区设计思想是双缓冲区，处理完就交换，所以不存在空间循环使用
            return (_writer_idx - _reader_idx);
        }

        //重置读写位置，初始化缓冲区
        void reset()
        {
            _reader_idx = 0;
            _writer_idx = 0;
        }

        //对buffer实现交换操作
        void swap(Buffer &buffer)
        {
            _buffer.swap(buffer._buffer);
            std::swap(_reader_idx, buffer._reader_idx);
            std::swap(_writer_idx, buffer._writer_idx);
        }

        //判断缓冲区是否为空
        bool empty()
        {
            return (_reader_idx == _writer_idx);
        }

        void moveReader(size_t len)
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }

    private:
        //对空间进行扩容
        void ensureEnoughSize(size_t len)
        {
            if(len <= writeAbleSize()) return;//不需要扩容
            size_t new_size = 0;
            if(_buffer.size() < THRESHOLD_BUFFER_SIZE)
            {
                new_size = _buffer.size() * 2;//小于阈值则翻倍增长
            }
            else
            {
                new_size = _buffer.size() + INCREMENT_BUFFER_SIZE;//否则线性增长
            }
            _buffer.resize(new_size);
        }
        //对读写指针进行向后偏移操作
        void moveWriter(size_t len)
        {
            assert((len+_writer_idx) <= _buffer.size());
            _writer_idx += len;
        }
    private:
        std::vector<char> _buffer;
        size_t _reader_idx; //当前可读数据的指针
        size_t _writer_idx; //当前可写数据的指针
    };
}

#endif