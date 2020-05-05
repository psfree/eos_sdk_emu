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

#include "easy_curl.h"

CurlGlobal::CurlGlobal() :
    _init(false)
{}

CurlGlobal::~CurlGlobal()
{
    cleanup();
}

CurlGlobal& CurlGlobal::Inst()
{
    static CurlGlobal _this;
    return _this;
}

CURLcode CurlGlobal::init(long flags)
{
    return curl_global_init(flags);
}

void CurlGlobal::cleanup()
{
    if (_init)
    {
        curl_global_cleanup();
        _init = false;
    }
}

// CurlEasy //
int CurlEasy::writer(char* data, size_t size, size_t nmemb, CurlEasy* _this)
{
    if (_this == nullptr)
        return 0;

    _this->_buffer.append(data, size * nmemb);

    return size * nmemb;
}

CurlEasy::CurlEasy() :
    _me(nullptr),
    _init(false)
{}

CurlEasy::~CurlEasy()
{
    cleanup();
}

bool CurlEasy::init()
{
    _init = (_me = curl_easy_init()) != nullptr;
    if (_init)
    {
        if (curl_easy_setopt(_me, CURLOPT_WRITEFUNCTION, writer) != CURLE_OK)
        {
            cleanup();
            return false;
        }

        if (curl_easy_setopt(_me, CURLOPT_WRITEDATA, this) != CURLE_OK)
        {
            cleanup();
            return false;
        }
    }
    return _init;
}

void CurlEasy::cleanup()
{
    if (_init)
    {
        curl_easy_cleanup(_me);
    }
}

CURLcode CurlEasy::set_url(const std::string& url)
{
    return curl_easy_setopt(_me, CURLOPT_URL, url.c_str());
}

CURLcode CurlEasy::skip_verifypeer(bool skip)
{
    return curl_easy_setopt(_me, CURLOPT_SSL_VERIFYPEER, skip ? 0L : 1L);
}

CURLcode CurlEasy::skip_verifyhost(bool skip)
{
    return curl_easy_setopt(_me, CURLOPT_SSL_VERIFYHOST, skip ? 0L : 1L);
}

CURLcode CurlEasy::connect_only(bool connect)
{
    return curl_easy_setopt(_me, CURLOPT_CONNECT_ONLY, connect ? 1L : 0L);
}

CURLcode CurlEasy::perform()
{
    _buffer.clear();
    return curl_easy_perform(_me);
}

CURLcode CurlEasy::recv(void* buffer, size_t buflen, size_t* read_len)
{
    return curl_easy_recv(_me, buffer, buflen, read_len);
}

CURLcode CurlEasy::get_html_code(long& code)
{
    return curl_easy_getinfo(_me, CURLINFO_RESPONSE_CODE, &code);
}

std::string const& CurlEasy::get_answer() const
{
    return _buffer;
}