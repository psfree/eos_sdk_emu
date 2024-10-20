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

#include <cstddef> // size_t

namespace utils {
    template<typename T, size_t N>
    constexpr size_t count_of(T(&)[N])
    {
        return N;
    }

    template<typename T, size_t N>
    constexpr size_t static_strlen(T(&)[N])
    {
        return N-1;
    }

    // Sum of all parameter pack sizeof
    template <typename... Ts>
    constexpr size_t size_of = 0;

    template <typename T, typename... Ts>
    constexpr size_t size_of<T, Ts...> = sizeof(T) + size_of<Ts...>;
}