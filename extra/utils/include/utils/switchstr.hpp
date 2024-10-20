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

#include <cstdint>
#include <string>

namespace utils {
    // switch case on a string
    constexpr uint64_t _hash(const char* input)
    {
        return (*input ? static_cast<uint64_t>(*input) + 33 * _hash(input + 1) : 5381);
    }

    constexpr uint64_t _hash(const char* input, size_t len)
    {
        return (len > 0 ? static_cast<uint64_t>(*input) + 33 * _hash(input + 1, len - 1) : 5381);
    }

    inline uint64_t _hash(const std::string& input)
    {
        return _hash(input.c_str());
    }
}

#define switchstr(x) switch( utils::_hash(x) )
#define casestr(x) case utils::_hash(x)

#define switchstrn(x, s) switch( utils::_hash(x, s) )
#define casestrn(x, s) case utils::_hash(x, s)