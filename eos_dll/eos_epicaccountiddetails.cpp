#include "eos_epicaccountiddetails.h"

EOS_EpicAccountIdDetails::EOS_EpicAccountIdDetails():
    _idstr(),
    _valid(false)
{}

EOS_EpicAccountIdDetails::EOS_EpicAccountIdDetails(std::string const& id)
{
    FromString(id.c_str());
}

EOS_EpicAccountIdDetails::EOS_EpicAccountIdDetails(EOS_EpicAccountIdDetails const& other):
    _idstr(other._idstr),
    _valid(other._valid)
{}

EOS_EpicAccountIdDetails::EOS_EpicAccountIdDetails(EOS_EpicAccountIdDetails && other) noexcept :
    _idstr(std::move(other._idstr)),
    _valid(std::move(other._valid))
{}

EOS_EpicAccountIdDetails::~EOS_EpicAccountIdDetails()
{}

EOS_EpicAccountIdDetails& EOS_EpicAccountIdDetails::operator=(std::string const& other)
{
    FromString(other.c_str());
    return *this;
}

EOS_EpicAccountIdDetails& EOS_EpicAccountIdDetails::operator=(EOS_EpicAccountIdDetails const& other)
{
    _idstr = other._idstr;
    _valid = other._valid;
    return *this;
}

EOS_EpicAccountIdDetails& EOS_EpicAccountIdDetails::operator=(EOS_EpicAccountIdDetails && other) noexcept
{
    _idstr = std::move(other._idstr);
    _valid = other._valid;
    return *this;
}

EOS_Bool EOS_EpicAccountIdDetails::IsValid()
{
    LOCAL_LOCK();
    return _valid;
}

EOS_EResult EOS_EpicAccountIdDetails::ToString(char* outBuffer, int32_t* outBufferSize)
{
    LOCAL_LOCK();

    if (outBuffer == nullptr || outBufferSize == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    size_t len = _idstr.length() + 1;
    if (*outBufferSize < len)
    {
        *outBufferSize = len;
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(outBuffer, _idstr.c_str(), len);
    *outBufferSize = len;
    return EOS_EResult::EOS_Success;
}

void EOS_EpicAccountIdDetails::FromString(const char* accountIdStr)
{
    LOCAL_LOCK();

    _idstr = accountIdStr;
    if (accountIdStr != nullptr)
    {
        size_t len = strlen(accountIdStr);
        if (len >= 2 &&
            accountIdStr[0] == '0' &&
            accountIdStr[1] == 'x')
        {
            accountIdStr += 2;
            len -= 2;
        }

        if (len > 0)
        {
            _valid = true;
            for (int i = 0; i < len; ++i)
            {
                char c = accountIdStr[i];
                if ((c < '0' || c > '9') &&
                    (c < 'A' || c > 'Z') &&
                    (c < 'a' || c > 'z')
                    )
                {
                    _valid = false;
                    break;
                }
            }
        }
    }
    else
        _valid = false;
}

std::string EOS_EpicAccountIdDetails::to_string() const
{
    return _idstr;
}

/////////////////////////////////////////////////////////

EOS_ProductUserIdDetails::EOS_ProductUserIdDetails() :
    _idstr(),
    _valid(false)
{}

EOS_ProductUserIdDetails::EOS_ProductUserIdDetails(std::string const& id)
{
    FromString(id.c_str());
}

EOS_ProductUserIdDetails::EOS_ProductUserIdDetails(EOS_ProductUserIdDetails const& other) :
    _idstr(other._idstr),
    _valid(other._valid)
{}

EOS_ProductUserIdDetails::EOS_ProductUserIdDetails(EOS_ProductUserIdDetails&& other) noexcept :
    _idstr(std::move(other._idstr)),
    _valid(std::move(other._valid))
{}

EOS_ProductUserIdDetails::~EOS_ProductUserIdDetails()
{}

EOS_ProductUserIdDetails& EOS_ProductUserIdDetails::operator=(std::string const& other)
{
    FromString(other.c_str());
    return *this;
}

EOS_ProductUserIdDetails& EOS_ProductUserIdDetails::operator=(EOS_ProductUserIdDetails const& other)
{
    _idstr = other._idstr;
    _valid = other._valid;
    return *this;
}

EOS_ProductUserIdDetails& EOS_ProductUserIdDetails::operator=(EOS_ProductUserIdDetails&& other) noexcept
{
    _idstr = std::move(other._idstr);
    _valid = other._valid;
    return *this;
}

EOS_Bool EOS_ProductUserIdDetails::IsValid()
{
    LOCAL_LOCK();
    return _valid;
}

EOS_EResult EOS_ProductUserIdDetails::ToString(char* outBuffer, int32_t* outBufferSize)
{
    LOCAL_LOCK();

    if (outBuffer == nullptr || outBufferSize == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    size_t len = _idstr.length() + 1;
    if (*outBufferSize < len)
    {
        *outBufferSize = len;
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(outBuffer, _idstr.c_str(), len);
    *outBufferSize = len;
    return EOS_EResult::EOS_Success;
}

void EOS_ProductUserIdDetails::FromString(const char* accountIdStr)
{
    LOCAL_LOCK();

    _idstr = accountIdStr;
    if (accountIdStr != nullptr)
    {
        size_t len = strlen(accountIdStr);
        if (len >= 2 &&
            accountIdStr[0] == '0' &&
            accountIdStr[1] == 'x')
        {
            accountIdStr += 2;
            len -= 2;
        }

        if (len > 0)
        {
            _valid = true;
            for (int i = 0; i < len; ++i)
            {
                char c = accountIdStr[i];
                if ((c < '0' || c > '9') &&
                    (c < 'A' || c > 'Z') &&
                    (c < 'a' || c > 'z')
                    )
                {
                    _valid = false;
                    break;
                }
            }
        }
    }
    else
        _valid = false;
}

std::string EOS_ProductUserIdDetails::to_string() const
{
    return _idstr;
}