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

inline constexpr const char* str_or_empty(const char* v)
{
    return (v == nullptr ? "" : v);
}

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

std::ostream& operator <<(std::ostream& os, EOS_EOwnershipStatus v)
{
    switch (v)
    {
#define ENUM_STR(X) case decltype(v)::X: return (os << #X)
        ENUM_STR(EOS_OS_NotOwned);
        ENUM_STR(EOS_OS_Owned);
#undef ENUM_STR
    }

    return (os << "Unknown enum value: " << (int32_t)v);
}

std::ostream& operator <<(std::ostream& os, EOS_EOnlineSessionPermissionLevel v)
{
    switch (v)
    {
#define ENUM_STR(X) case decltype(v)::X: return (os << #X)
        ENUM_STR(EOS_OSPF_PublicAdvertised);
        ENUM_STR(EOS_OSPF_JoinViaPresence);
        ENUM_STR(EOS_OSPF_InviteOnly);
#undef ENUM_STR
    }

    return (os << "Unknown enum value: " << (int32_t)v);
}

std::ostream& operator <<(std::ostream& os, EOS_EOnlineSessionState v)
{
    switch (v)
    {
#define ENUM_STR(X) case decltype(v)::X: return (os << #X)
        ENUM_STR(EOS_OSS_NoSession);
        ENUM_STR(EOS_OSS_Creating);
        ENUM_STR(EOS_OSS_Pending);
        ENUM_STR(EOS_OSS_Starting);
        ENUM_STR(EOS_OSS_InProgress);
        ENUM_STR(EOS_OSS_Ending);
        ENUM_STR(EOS_OSS_Ended);
        ENUM_STR(EOS_OSS_Destroying);
#undef ENUM_STR
    }

    return (os << "Unknown enum value: " << (int32_t)v);
}

inline std::ostream& operator <<(std::ostream& os, EOS_EResult v)
{
    return (os << EOS_EResult_ToString(v));
}

// Workaround to use fifo_map in json
// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
template<class K, class V, class dummy_compare, class A>
using my_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using fifo_json = nlohmann::basic_json<my_fifo_map>;

template<template<typename, typename, typename...> class ObjectType,
    template<typename, typename...> class ArrayType,
    class StringType, class BooleanType, class NumberIntegerType,
    class NumberUnsignedType, class NumberFloatType,
    template<typename> class AllocatorType,
    template<typename, typename = void> class JSONSerializer>
bool load_json(std::string const& file_path, nlohmann::basic_json<ObjectType, ArrayType, StringType, BooleanType, NumberIntegerType, NumberUnsignedType, NumberFloatType, AllocatorType, JSONSerializer>& json)
{
    std::ifstream file(file_path);
    if (file)
    {
        file.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        std::string buffer(size, '\0');

        file.read(&buffer[0], size);
        file.close();

        try
        {
            json = std::move(nlohmann::json::parse(buffer));

            return true;
        }
        catch (std::exception & e)
        {
            LOG(Log::LogLevel::ERR, "Error while parsing JSON %s: %s", file_path.c_str(), e.what());
        }
    }
    else
    {
        LOG(Log::LogLevel::WARN, "File not found: %s", file_path.c_str());
    }
    return false;
}

template<template<typename, typename, typename...> class ObjectType,
    template<typename, typename...> class ArrayType,
    class StringType, class BooleanType, class NumberIntegerType,
    class NumberUnsignedType, class NumberFloatType,
    template<typename> class AllocatorType,
    template<typename, typename = void> class JSONSerializer>
bool save_json(std::string const& file_path, nlohmann::basic_json<ObjectType, ArrayType, StringType, BooleanType, NumberIntegerType, NumberUnsignedType, NumberFloatType, AllocatorType, JSONSerializer> const& json)
{
    std::ofstream file(file_path, std::ios::trunc | std::ios::out);
    if (!file)
    {
        LOG(Log::LogLevel::ERR, "Failed to save: %s", file_path.c_str());
        return false;
    }
    file << std::setw(2) << json;
    return true;
}