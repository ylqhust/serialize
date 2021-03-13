#pragma once
#include <cassert>

enum MetaType
{
    kUnknow = 0,
    kBool,
    kChar,
    kUChar,
    kWChar,
    kShort,
    kUShort,
    kInt,
    kUInt,
    kLong,
    kULong,
    kLLong,
    kULLong,
    kFloat,
    kDouble,
    kString,
    kWString,
    kVector,
    kMap,
};

#define USE_MACRO(MACRO)\
MACRO(bool, kBool)\
MACRO(char, kChar)\
MACRO(unsigned char, kUChar)\
MACRO(wchar_t, kWChar)\
MACRO(short, kShort)\
MACRO(unsigned short, kUShort)\
MACRO(int, kInt)\
MACRO(unsigned int, kUInt)\
MACRO(long, kLong)\
MACRO(unsigned long ,kULong)\
MACRO(long long, kLLong)\
MACRO(unsigned long long, kULLong)\
MACRO(float, kFloat)\
MACRO(double, kDouble)\

template<typename T>
constexpr MetaType ToMetaType()
{
    return MetaType::kUnknow;
}

#define TO_METATYPE_MACRO(sysType, myType) \
template<>\
constexpr MetaType ToMetaType<sysType>()\
{return MetaType:: ## myType;}

USE_MACRO(TO_METATYPE_MACRO)
