#include "eos_epicaccountiddetails.h"

EOS_Bool EOS_EpicAccountIdDetails::IsValid()
{
    LOCAL_LOCK();

    return _id.length() ? EOS_TRUE : EOS_FALSE;
}

EOS_EResult EOS_EpicAccountIdDetails::ToString(char* outBuffer, int32_t* outBufferSize)
{
    LOCAL_LOCK();

    if (outBuffer == nullptr || outBufferSize == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (_id.length() > *outBufferSize)
    {
        *outBufferSize = _id.length() + 1;
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(outBuffer, _id.c_str(), _id.length() + 1);
    *outBufferSize = _id.length() + 1;
    return EOS_EResult::EOS_Success;
}

void EOS_EpicAccountIdDetails::FromString(const char* accountIdStr)
{
    LOCAL_LOCK();

    if (accountIdStr != nullptr)
        _id = accountIdStr;
}