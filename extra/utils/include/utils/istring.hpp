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

#include <string>

namespace utils {
    // Case insensible string
    struct ichar_traits : public std::char_traits<char>
    {
        static bool eq(char c1, char c2) { return tolower(c1) == tolower(c2); }
        static bool ne(char c1, char c2) { return tolower(c1) != tolower(c2); }
        static bool lt(char c1, char c2) { return tolower(c1) <  tolower(c2); }
        static int compare(const char* s1, const char* s2, size_t n)
        {
            while( n-- != 0 )
            {
                if( tolower(*s1) < tolower(*s2) ) return -1;
                if( tolower(*s1) > tolower(*s2) ) return 1;
                ++s1; ++s2;
            }
            return 0;
        }
        static const char* find(const char* s, int n, char a)
        {
            while( n-- > 0 && tolower(*s) != tolower(a) )
            {
                ++s;
            }
            return s;
        }
    };

    using istring = std::basic_string<char, ichar_traits>;
}