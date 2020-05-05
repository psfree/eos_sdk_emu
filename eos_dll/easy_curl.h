/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Epic Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The Nemirtingas's Epic Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Epic Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <curl/curl.h>
#include <string>

class CurlGlobal
{
    bool _init;

    CurlGlobal();
    ~CurlGlobal();

public:
    static CurlGlobal& Inst();

    CURLcode init(long flags = CURL_GLOBAL_DEFAULT);

    void cleanup();
};

class CurlEasy
{
    CURL* _me;
    bool _init;
    std::string _buffer;

    static int writer(char* data, size_t size, size_t nmemb, CurlEasy* _this);

public:
    CurlEasy();
    ~CurlEasy();

    bool init();

    void cleanup();

    CURLcode set_url(const std::string& url);

    CURLcode skip_verifypeer(bool skip = true);

    CURLcode skip_verifyhost(bool skip = true);

    CURLcode connect_only(bool connect = true);

    CURLcode perform();

    CURLcode recv(void* buffer, size_t buflen, size_t* read_len);

    CURLcode get_html_code(long& code);

    std::string const& get_answer() const;
};