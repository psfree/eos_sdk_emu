/* Copyright (C) Nemirtingas
 * This file is part of utils.
 *
 * utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with utils.  If not, see <https://www.gnu.org/licenses/>
 */

#pragma once

#include <type_traits>

namespace utils {
    template<typename Enum>
    struct EnableBitMaskOperators
    {
        static constexpr bool enable = false;
    };

    template<typename T>
    constexpr typename std::underlying_type<T>::type GetEnumValue(T enum_value)
    {
        return static_cast<typename std::underlying_type<T>::type>(enum_value);
    }
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type
operator |(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs) |
        static_cast<underlying>(rhs)
        );
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type
operator &(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
        );
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type
operator ^(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
        );
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type
operator ~(Enum lhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (~static_cast<underlying>(lhs));
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type&
operator |=(Enum& lhs, Enum rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type&
operator &=(Enum& lhs, Enum rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, Enum>::type&
operator ^=(Enum& lhs, Enum rhs)
{
    lhs = lhs ^ rhs;
    return lhs;
}

template<typename Enum>
constexpr typename std::enable_if<utils::EnableBitMaskOperators<Enum>::enable, bool>::type
operator !(Enum lhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return !static_cast<underlying>(lhs);
}

#define UTILS_ENABLE_BITMASK_OPERATORS(T) \
template<>                                \
struct utils::EnableBitMaskOperators<T>   \
{                                         \
    static constexpr bool enable = true;  \
}
