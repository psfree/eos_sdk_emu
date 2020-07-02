#include "eos_epicaccountiddetails.h"

EOS_EpicAccountIdDetails::EOS_EpicAccountIdDetails():
    _idstr(sdk::NULL_USER_ID),
    _valid(false)
{}

EOS_EpicAccountIdDetails::EOS_EpicAccountIdDetails(EOS_EpicAccountIdDetails && other) noexcept :
    _idstr(std::move(other._idstr)),
    _valid(std::move(other._valid))
{}

EOS_EpicAccountIdDetails::~EOS_EpicAccountIdDetails()
{}

EOS_EpicAccountIdDetails& EOS_EpicAccountIdDetails::operator=(EOS_EpicAccountIdDetails && other) noexcept
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    _idstr = std::move(other._idstr);
    _valid = other._valid;
    return *this;
}

EOS_Bool EOS_EpicAccountIdDetails::IsValid()
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    return _valid;
}

EOS_EResult EOS_EpicAccountIdDetails::ToString(char* outBuffer, int32_t* outBufferSize)
{
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (outBuffer == nullptr || outBufferSize == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    size_t len = _idstr.length() + 1;
    if (*outBufferSize < len)
    {
        *outBufferSize = static_cast<int32_t>(len);
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(outBuffer, _idstr.c_str(), len);
    *outBufferSize = static_cast<int32_t>(len);
    return EOS_EResult::EOS_Success;
}

void EOS_EpicAccountIdDetails::FromString(const char* accountIdStr)
{
    if (accountIdStr != nullptr)
    {
        from_string(accountIdStr);
    }
    else
    {
        std::lock_guard<std::mutex> lk(_local_mutex);
        _valid = false;
        _idstr = sdk::NULL_USER_ID;
    }
}

void EOS_EpicAccountIdDetails::from_string(std::string const& accountIdStr)
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    _idstr = accountIdStr;
    validate();
}

std::string const& EOS_EpicAccountIdDetails::to_string()
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    return _idstr;
}

void EOS_EpicAccountIdDetails::validate()
{
    auto it = _idstr.begin();
    _valid = false;

    if (_idstr.length() > 2 &&
        _idstr[0] == '0' &&
        _idstr[1] == 'x')
    {
        it = _idstr.erase(_idstr.begin(), _idstr.begin() + 2);
    }

    if (_idstr.length() == (sdk::max_id_length - 3))
    {
        if (std::string(it, _idstr.end()) == sdk::NULL_USER_ID)
            return;

        _valid = true;
        for (; it != _idstr.end(); ++it)
        {
            char c = *it;
            if ((c < '0' || c > '9') &&
                (c < 'A' || c > 'F') &&
                (c < 'a' || c > 'f')
                )
            {
                _valid = false;
                return;
            }
        }
    }
}

/////////////////////////////////////////////////////////

EOS_ProductUserIdDetails::EOS_ProductUserIdDetails() :
    _idstr(),
    _valid(false)
{}

EOS_ProductUserIdDetails::EOS_ProductUserIdDetails(EOS_ProductUserIdDetails&& other) noexcept :
    _idstr(std::move(other._idstr)),
    _valid(std::move(other._valid))
{}

EOS_ProductUserIdDetails::~EOS_ProductUserIdDetails()
{}

EOS_ProductUserIdDetails& EOS_ProductUserIdDetails::operator=(EOS_ProductUserIdDetails&& other) noexcept
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    _idstr = std::move(other._idstr);
    _valid = other._valid;
    return *this;
}

EOS_Bool EOS_ProductUserIdDetails::IsValid()
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    return _valid;
}

EOS_EResult EOS_ProductUserIdDetails::ToString(char* outBuffer, int32_t* outBufferSize)
{
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (outBuffer == nullptr || outBufferSize == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    size_t len = _idstr.length() + 1;
    if (*outBufferSize < len)
    {
        *outBufferSize = static_cast<int32_t>(len);
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(outBuffer, _idstr.c_str(), len);
    *outBufferSize = static_cast<int32_t>(len);
    return EOS_EResult::EOS_Success;
}

void EOS_ProductUserIdDetails::FromString(const char* accountIdStr)
{
    if (accountIdStr != nullptr)
    {
        from_string(accountIdStr);
    }
    else
    {
        std::lock_guard<std::mutex> lk(_local_mutex);
        _idstr = sdk::NULL_USER_ID;
        _valid = false;
    }
}

std::string const& EOS_ProductUserIdDetails::to_string()
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    return _idstr;
}

void EOS_ProductUserIdDetails::from_string(std::string const& accountIdStr)
{
    std::lock_guard<std::mutex> lk(_local_mutex);
    _idstr = accountIdStr;
    validate();
}

void EOS_ProductUserIdDetails::validate()
{
    auto it = _idstr.begin();
    _valid = false;

    if (_idstr.length() > 2 &&
        _idstr[0] == '0' &&
        _idstr[1] == 'x')
    {
        it = _idstr.erase(_idstr.begin(), _idstr.begin() + 2);
    }

    if (_idstr.length() == (sdk::max_id_length - 3))
    {
        if (std::string(it, _idstr.end()) == sdk::NULL_USER_ID)
            return;

        _valid = true;
        for (; it != _idstr.end(); ++it)
        {
            char c = *it;
            if ((c < '0' || c > '9') &&
                (c < 'A' || c > 'F') &&
                (c < 'a' || c > 'f')
                )
            {
                _valid = false;
                return;
            }
        }
    }
}