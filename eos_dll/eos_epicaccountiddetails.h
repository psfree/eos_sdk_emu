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

#pragma once

#include "common_includes.h"

struct EOS_EpicAccountIdDetails
{
private:
    std::recursive_mutex local_mutex;
    std::string _idstr;
    bool _valid;

public:
    EOS_EpicAccountIdDetails();
    EOS_EpicAccountIdDetails(std::string const& id);
    EOS_EpicAccountIdDetails(EOS_EpicAccountIdDetails const&);
    EOS_EpicAccountIdDetails(EOS_EpicAccountIdDetails &&) noexcept;
    ~EOS_EpicAccountIdDetails();

    EOS_EpicAccountIdDetails& operator=(std::string const&);
    EOS_EpicAccountIdDetails& operator=(EOS_EpicAccountIdDetails const&);
    EOS_EpicAccountIdDetails& operator=(EOS_EpicAccountIdDetails &&) noexcept;

    EOS_Bool IsValid();
    EOS_EResult ToString(char* outBuffer, int32_t* outBufferSize);
    void FromString(const char* accountIdStr);

    std::string to_string() const;

    inline bool operator ==(EOS_EpicAccountIdDetails const& other) { return (_idstr == other._idstr); }
    inline bool operator !=(EOS_EpicAccountIdDetails const& other) { return !(*this == other); }
};

struct EOS_ProductUserIdDetails
{
private:
    std::recursive_mutex local_mutex;
    std::string _idstr;
    bool _valid;

public:
    EOS_ProductUserIdDetails();
    EOS_ProductUserIdDetails(std::string const& id);
    EOS_ProductUserIdDetails(EOS_ProductUserIdDetails const&);
    EOS_ProductUserIdDetails(EOS_ProductUserIdDetails&&) noexcept;
    ~EOS_ProductUserIdDetails();

    EOS_ProductUserIdDetails& operator=(std::string const&);
    EOS_ProductUserIdDetails& operator=(EOS_ProductUserIdDetails const&);
    EOS_ProductUserIdDetails& operator=(EOS_ProductUserIdDetails&&) noexcept;

    EOS_Bool IsValid();
    EOS_EResult ToString(char* outBuffer, int32_t* outBufferSize);
    void FromString(const char* accountIdStr);

    std::string to_string() const;

    inline bool operator ==(EOS_ProductUserIdDetails const& other) { return (_idstr == other._idstr); }
    inline bool operator !=(EOS_ProductUserIdDetails const& other) { return !(*this == other); }
};