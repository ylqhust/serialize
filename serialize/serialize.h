#pragma once
#include "metatype.h"
#include "variant.h"
#include <vector>
#include <type_traits>
#include <cassert>

#define MAGIC_HEAD1 0xab
#define MAGIC_HEAD2 0xcd
#define MAGIC_HEAD3 0xef
#define MAGIC_HEAD4 0x00

class Serialize
{
public:
    Serialize()
    {
        /**
         * |MAGIC_HEAD 4bytes|data length 4bytes|.....data|
         * 
         * data:
         * |metatype 1byte|variant data|
         * 
         * 
         */
        //check header
        _data.push_back(MAGIC_HEAD1);
        _data.push_back(MAGIC_HEAD2);
        _data.push_back(MAGIC_HEAD3);
        _data.push_back(MAGIC_HEAD4);
        //data length
        _data.resize(_data.size() + sizeof(unsigned int));
        setLength();
    }

    const unsigned char* buffer() const
    {
        return _data.data();
    }

    unsigned int length() const
    {
        return _data.size();
    }

    Serialize& operator<<(const Variant& v)
    {
        if (v.isValid())
        {
            v.serialize(_data);
            setLength();
        }
        return *this;
    }

private:
    void setLength()
    {
        unsigned int length = (unsigned int)_data.size() - 4 - sizeof(unsigned int);
        memcpy(&_data[4], &length, sizeof(length));
    }
    Serialize(const Serialize&) = delete;
    Serialize(const Serialize&&) = delete;
    Serialize& operator=(const Serialize&) = delete;
    Serialize& operator=(const Serialize&&) = delete;
private:
    std::vector<unsigned char> _data;
};

class UnSerialize
{
public:
    UnSerialize(const unsigned char* data, unsigned int length)
    {
        _data = std::vector<unsigned char>(data, data + length);
        _pos = 0;
        check();
    }

    bool isEmpty() const
    {
        return _pos >= _data.size();
    }

    UnSerialize& operator>>(Variant& v)
    {
        v.reset();
        v.unserialize(_data, _pos);
        return *this;
    }

private:
    void check()
    {
        if (_data.size() <= 4 + sizeof(unsigned int))
        {
            _data.clear();
            _pos = 0;
            return;
        }
        if (_data[0] != MAGIC_HEAD1 || _data[1] != MAGIC_HEAD2
            || _data[2] != MAGIC_HEAD3 || _data[3] != MAGIC_HEAD4)
        {
            _data.clear();
            _pos = 0;
            return;
        }
        _pos += 4;
        unsigned int validDataLength = 0;
        memcpy(&validDataLength, &_data[_pos], sizeof(validDataLength));
        _pos += sizeof(validDataLength);
        if (_pos + validDataLength != _data.size())
        {
            _data.clear();
            _pos = 0;
            return;
        }
    }
    UnSerialize(const UnSerialize&) = delete;
    UnSerialize& operator=(const UnSerialize&) = delete;
    UnSerialize(const UnSerialize&&) = delete;
    UnSerialize& operator=(const UnSerialize&&) = delete;
private:
    std::vector<unsigned char> _data;
    unsigned int _pos = 0;
};
