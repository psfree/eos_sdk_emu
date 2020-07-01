#pragma once

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
    #define __WINDOWS_64__
    #define __64BITS__
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define __WINDOWS_32__
    #define __32BITS__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
    #define __WINDOWS__
#endif

#if defined(__clang__) || defined(__GNUC__)
    #define CLANG_GCC_DONT_OPTIMIZE __attribute__((optnone))
#else
    #define CLANG_GCC_DONT_OPTIMIZE
#endif

inline const char* EOS_Bool_2_str(EOS_Bool v)
{
    return (v == EOS_TRUE ? "EOS_TRUE" : "EOS_FALSE");
}

std::ostream& operator <<(std::ostream& os, EOS_ELeaderboardAggregation v)
{
    switch (v)
    {
#define ENUM_STR(X) case decltype(v)::X: return (os << #X)
        ENUM_STR(EOS_LA_Min);
        ENUM_STR(EOS_LA_Max);
        ENUM_STR(EOS_LA_Sum);
        ENUM_STR(EOS_LA_Latest);
#undef ENUM_STR
    }

    return (os << "Unknown enum value: " << (int32_t)v);
}

std::ostream& operator <<(std::ostream& os, EOS_ENATType v)
{
    switch (v)
    {
#define ENUM_STR(X) case decltype(v)::X: return (os << #X)
        ENUM_STR(EOS_NAT_Unknown);
        ENUM_STR(EOS_NAT_Open);
        ENUM_STR(EOS_NAT_Moderate);
        ENUM_STR(EOS_NAT_Strict);
#undef ENUM_STR
    }

    return (os << "Unknown enum value: " << (int32_t)v);
}

std::ostream& operator <<(std::ostream& os, EOS_Presence_EStatus v)
{
    switch (v)
    {
#define ENUM_STR(X) case decltype(v)::X: return (os << #X)
        ENUM_STR(EOS_PS_Offline);
        ENUM_STR(EOS_PS_Online);
        ENUM_STR(EOS_PS_Away);
        ENUM_STR(EOS_PS_ExtendedAway);
        ENUM_STR(EOS_PS_DoNotDisturb);
#undef ENUM_STR
    }

    return (os << "Unknown enum value: " << (int32_t)v);
}

inline std::ostream& operator <<(std::ostream& os, EOS_EResult v)
{
    return (os << EOS_EResult_ToString(v));
}