#include "Log.h"
#include "common_includes.h"

Log Log::L;

Log::Log():
    _lv(LogLevel::OFF)
{

}

Log::~Log()
{

}

LOCAL_API bool _trace(const char* format, va_list argptr)
{
    //va_list argptr;
    //va_start(argptr, format);

    std::string fmt = format;
    if (*fmt.rbegin() != '\n')
        fmt += '\n';

#if defined(__WINDOWS__)
    if (IsDebuggerPresent())
    {
        va_list argptr2;
        va_copy(argptr2, argptr);
        int len = vsnprintf(nullptr, 0, fmt.c_str(), argptr);

        char* buffer = new char[++len];
        vsnprintf(buffer, len, fmt.c_str(), argptr2);

        va_end(argptr);
        va_end(argptr2);

        OutputDebugString(buffer);
        delete[]buffer;
    }
    else
#endif
    {
        vfprintf(stderr, fmt.c_str(), argptr);
    }

    return true;
}

void Log::operator()(LogLevel lv, const char* format, ...)
{
    if (lv >= _lv && _lv < LogLevel::MAX)
    {
        va_list argptr;
        va_start(argptr, format);
        _trace(format, argptr);
    }
}