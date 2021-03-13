#pragma once

#include "metatype.h"
#include <string>
#include <vector>
#include <map>

class Variant
{
    friend class Serialize;
    friend class UnSerialize;
public:
    // construct
    Variant()
    {
    }
    ~Variant()
    {
        reset();
    }

    Variant(const Variant& other)
    {
        construct(other);
    }

    Variant(Variant&& other)
    {
        construct(std::move(other));
    }

    Variant& operator=(const Variant& other)
    {
        construct(other);
        return *this;
    }

    Variant& operator=(Variant&& other)
    {
        construct(std::move(other));
        return *this;
    }

    bool isValid() const
    {
        return _type != MetaType::kUnknow;
    }

    MetaType type() const
    {
        return _type;
    }

    template<typename T>
    Variant(T v)
    {
        static_assert(std::is_integral_v<T>, "T is not integer type");
        static_assert(sizeof(T) > 0, "T size is zero");
        const MetaType type = ToMetaType<T>();
        static_assert(type != MetaType::kUnknow, "ToMetaType failed.");
        create(type, (void*)&v, sizeof(v));
    }

    template<>
    Variant(float v)
    {
        create(MetaType::kFloat, (void*)&v, sizeof(v));
    }

    template<>
    Variant(double v)
    {
        create(MetaType::kDouble, (void*)&v, sizeof(v));
    }

    template<>
    Variant(const char* v)
    {
        char* end = (char*)v;
        while (*end)++end;
        create(MetaType::kString, (void*)v, (end - v) * sizeof(*v));
    }

    template<>
    Variant(const std::string& v)
    {
        create(MetaType::kString, (void*)v.c_str(), v.size() * sizeof(v[0]));
    }

    template<>
    Variant(std::string v)
    {
        create(MetaType::kString, (void*)v.c_str(), v.size() * sizeof(v[0]));
    }

    template<>
    Variant(const wchar_t* v)
    {
        wchar_t* end = (wchar_t*)v;
        while (*end)++end;
        create(MetaType::kWString, (void*)v, (end - v) * sizeof(*v));
    }

    template<>
    Variant(const std::wstring& v)
    {
        create(MetaType::kWString, (void*)v.c_str(), v.size() * sizeof(v[0]));
    }

    template<>
    Variant(std::wstring v)
    {
        create(MetaType::kWString, (void*)v.c_str(), v.size() * sizeof(v[0]));
    }

    template<typename T, template<typename U> typename Container>
    Variant(Container<T> v)
    {
        static_assert(sizeof(T) < 0, "not implement");
    }
    template<typename T>
    Variant(const std::vector<T>& v)
    {
        _type = MetaType::kVector;
        for (size_t i = 0; i < v.size(); ++i)
        {
            Variant* newV = new Variant(v[i]);
            _variants.push_back(newV);
        }
    }
    template<typename T>
    Variant(std::vector<T>&& v)
    {
        _type = MetaType::kVector;
        for (size_t i = 0; i < v.size(); ++i)
        {
            Variant* newV = new Variant(std::move(v[i]));
            _variants.push_back(newV);
        }
    }
    template<typename Key, typename Value, template<typename K, typename V>typename Container>
    Variant(Container<Key, Value> v)
    {
        static_assert(sizeof(Key) < 0, "not implement");
    }
    template<typename Key, typename Value>
    Variant(const std::map<Key, Value>& v)
    {
        _type = MetaType::kMap;
        for (auto pos = v.begin(); pos != v.end(); ++pos)
        {
            Variant* newKey = new Variant(pos->first);
            Variant* newValue = new Variant(pos->second);
            _variants.push_back(newKey);
            _variants.push_back(newValue);
        }
    }
    template<typename Key, typename Value>
    Variant(std::map<Key, Value>&& v)
    {
        _type = MetaType::kMap;
        for (auto pos = v.begin(); pos != v.end(); ++pos)
        {
            Variant* newKey = new Variant(std::move(pos->first));
            Variant* newValue = new Variant(std::move(pos->second));
            _variants.push_back(newKey);
            _variants.push_back(newValue);
        }
    }

    // isType
#define IS_TYPE_MACRO(sysType, myType)\
bool is ## myType () const { return _type == myType; }


    USE_MACRO(IS_TYPE_MACRO);
    bool iskString() const { return _type == MetaType::kString; }
    bool iskWString() const { return _type == MetaType::kWString; }
    bool iskVector() const { return _type == MetaType::kVector; }
    bool iskMap() const { return _type == MetaType::kMap; }

    // to
    template<typename T>
    T to() const
    {
        static_assert(std::is_pod_v<T>, "T is not pod type");
        T t = 0;
        const MetaType type = ToMetaType<T>();
        if (type != _type)
            return t;
        memcpy(&t, _buffer.data(), sizeof(t));
        return t;
    }

    template<>
    std::string to() const
    {
        if (_type != MetaType::kString)return "";
        return std::string(_buffer.begin(), _buffer.end());
    }

    template<>
    std::wstring to() const
    {
        if (_type != MetaType::kWString)return L"";
        return std::wstring((const wchar_t*)_buffer.data(), (const wchar_t*)(_buffer.data() + _buffer.size()));
    }

    template<typename T>
    bool to(T& o) const
    {
        static_assert(std::is_pod_v<T>, "T is not pod type");
        const MetaType type = ToMetaType<T>();
        if (type != _type)
            return false;
        memcpy(&o, _buffer.data(), sizeof(o));
        return true;
    }

    template<>
    bool to(std::string& o) const
    {
        if (_type != MetaType::kString)return false;
        o = std::string(_buffer.begin(), _buffer.end());
        return true;
    }

    template<>
    bool to(std::wstring& o) const
    {
        if (_type != MetaType::kWString)return false;
        o = std::wstring((const wchar_t*)_buffer.data(), (const wchar_t*)(_buffer.data() + _buffer.size()));
        return true;
    }

    template<typename T>
    bool to(std::vector<T>& o)
    {
        if (_type != MetaType::kVector) return false;
        for (size_t i = 0; i < _variants.size(); ++i)
        {
            T tmp;
            if (_variants[i]->to(tmp))
                o.push_back(tmp);
            else
                return false;
        }
        return true;
    }

    template<typename Key, typename Value>
    bool to(std::map<Key, Value>& o)
    {
        if (_type != MetaType::kMap) return false;
        for (size_t i = 0; i < _variants.size(); i += 2)
        {
            Key key;
            Value value;
            if (_variants[i]->to(key) && _variants[i + 1]->to(value))
                o.insert(std::make_pair<Key, Value>(std::move(key), std::move(value)));
            else
                return false;
        }
        return true;
    }

private:
    void create(const MetaType& type, void* src, unsigned int bytes)
    {
        _type = type;
        _buffer.resize(bytes);
        if (bytes > 0)
        {
            memcpy(_buffer.data(), src, bytes);
        }
    }

    void construct(const Variant& other)
    {
        reset();
        _type = other._type;
        _buffer = other._buffer;
        for (size_t i = 0; i < other._variants.size(); ++i)
        {
            Variant* newV = new Variant(*other._variants[i]);
            _variants.push_back(newV);
        }
    }
    void construct(Variant&& other)
    {
        reset();
        _type = other._type;
        _buffer = std::move(other._buffer);
        _variants = std::move(other._variants);
        other.reset();
    } 
    void reset() const 
    {
        _buffer.clear();
        _type = MetaType::kUnknow;
        for (size_t i = 0; i < _variants.size(); ++i)
        {
            delete _variants[i];
        }
        _variants.clear();
    }

#define COPY_TO_STORAGE_MACRO(sysType, myType)\
case myType:\
{storage.push_back(myType);\
storage.resize(storage.size()+_buffer.size());\
memcpy(&storage[storage.size()-_buffer.size()],_buffer.data(),_buffer.size());\
return;}

    void serialize(std::vector<unsigned char>& storage) const
    {
        if (!isValid())return;
        switch (_type)
        {
            USE_MACRO(COPY_TO_STORAGE_MACRO);
        case MetaType::kString:
        case MetaType::kWString:
        {
            storage.push_back(_type);
            unsigned int strLength = _buffer.size();
            storage.resize(storage.size() + sizeof(strLength) + strLength);
            memcpy(&storage[storage.size() - strLength - sizeof(strLength)], &strLength, sizeof(strLength));
            if (strLength > 0)
            {
                memcpy(&storage[storage.size() - strLength], _buffer.data(), strLength);
            }
            return;
        }
        case MetaType::kVector:
        {
            storage.push_back(_type);
            unsigned int count = _variants.size();
            storage.resize(storage.size() + sizeof(count));
            memcpy(&storage[storage.size() - sizeof(count)], &count, sizeof(count));
            for (size_t i = 0; i < _variants.size(); ++i)
            {
                _variants[i]->serialize(storage);
            }
            return;
        }
        case MetaType::kMap:
        {
            storage.push_back(_type);
            unsigned int count = _variants.size() / 2;
            storage.resize(storage.size() + sizeof(count));
            memcpy(&storage[storage.size() - sizeof(count)], &count, sizeof(count));
            for (size_t i = 0; i < _variants.size(); i += 2)
            {
                _variants[i]->serialize(storage);
                _variants[i + 1]->serialize(storage);
            }
            return;
        }
        }
    }

#define COPY_FROM_DATA_MACRO(sysType, myType)\
case myType:\
{++pos;\
_type=myType;\
_buffer.resize(sizeof(sysType));\
memcpy(_buffer.data(),&data[pos],sizeof(sysType));\
pos+=sizeof(sysType);\
return;}

    void unserialize(const std::vector<unsigned char>& data, unsigned int& pos)
    {
        MetaType type = (MetaType)data[pos];
        switch (type)
        {
            USE_MACRO(COPY_FROM_DATA_MACRO);
        case MetaType::kString:
        case MetaType::kWString:
        {
            ++pos;
            _type = type;
            unsigned int strLength = 0;
            memcpy(&strLength, &data[pos], sizeof(strLength));
            pos += sizeof(strLength);
            _buffer.resize(strLength);
            if (strLength > 0)
            {
                memcpy(_buffer.data(), &data[pos], strLength);
                pos += strLength;
            }
            return;
        }
        case MetaType::kVector:
        {
            ++pos;
            _type = type;
            unsigned int count = 0;
            memcpy(&count, &data[pos], sizeof(count));
            pos += sizeof(count);
            for (unsigned int i = 0; i < count; ++i)
            {
                Variant* newV = new Variant();
                newV->unserialize(data, pos);
                _variants.push_back(newV);
            }
            return;
        }
        case MetaType::kMap:
        {
            ++pos;
            _type = type;
            unsigned int count = 0;
            memcpy(&count, &data[pos], sizeof(count));
            pos += sizeof(count);
            for (unsigned int i = 0; i < count; ++i)
            {
                Variant* key = new Variant();
                Variant* value = new Variant();
                key->unserialize(data, pos);
                value->unserialize(data, pos);
                _variants.push_back(key);
                _variants.push_back(value);
            }
            return;
        }
        }
    }

private:
    mutable MetaType _type = kUnknow;
    mutable std::vector<unsigned char> _buffer;
    mutable std::vector<Variant*> _variants;
};
