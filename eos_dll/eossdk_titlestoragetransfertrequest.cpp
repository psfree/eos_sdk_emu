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

#include "eossdk_titlestorage.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

EOSSDK_TitleStorageFileTransferRequest::EOSSDK_TitleStorageFileTransferRequest():
    _done(false),
    _canceled(false),
    _released(false)
{

}

EOSSDK_TitleStorageFileTransferRequest::~EOSSDK_TitleStorageFileTransferRequest()
{

}

void EOSSDK_TitleStorageFileTransferRequest::set_read_transfert(const EOS_TitleStorage_ReadFileOptions* ReadOptions)
{
    std::lock_guard<std::mutex> _lk(_local_mutex);
    std::string file_path = FileManager::join(EOSSDK_PlayerDataStorage::remote_directory, FileManager::clean_path(ReadOptions->Filename));

    _read_callback = ReadOptions->ReadFileDataCallback;
    _progress_callback = ReadOptions->FileTransferProgressCallback;
    _chunk_size = ReadOptions->ReadChunkLengthBytes;
    _file_name = ReadOptions->Filename;
    _file_size = FileManager::file_size(file_path);

    _file_buffer.resize(_chunk_size);
    _input_file = FileManager::open_read(file_path, std::ios::binary);
}

bool EOSSDK_TitleStorageFileTransferRequest::canceled()
{
    std::lock_guard<std::mutex> _lk(_local_mutex);
    return _canceled;
}

bool EOSSDK_TitleStorageFileTransferRequest::released()
{
    std::lock_guard<std::mutex> _lk(_local_mutex);
    return _released;
}

/**
 * Below are helper functions to retrieve information about a file request handle, or to attempt to cancel a request in progress.
 */

 /**
  * Get the current state of a file request.
  *
  * @return EOS_Success if complete and successful, EOS_RequestInProgress if the request is still in progress, or another state for failure.
  */
EOS_EResult EOSSDK_TitleStorageFileTransferRequest::GetFileRequestState()
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> _lk(_local_mutex);

    return (_done ? EOS_EResult::EOS_Success : EOS_EResult::EOS_RequestInProgress);
}

/**
 * Get the file name of the file this request is for. OutStringLength will always be set to the string length of the file name if it is not NULL.
 *
 * @param FilenameStringBufferSizeBytes The maximum number of bytes that can be written to OutStringBuffer
 * @param OutStringBuffer The buffer to write the NULL-terminated utf8 file name into, if successful
 * @param OutStringLength How long the file name is (not including null terminator)
 * @return EOS_Success if the file name was successfully written to OutFilenameBuffer, a failure result otherwise
 *
 * @see EOS_TITLESTORAGE_FILENAME_MAX_LENGTH_BYTES
 */
EOS_EResult EOSSDK_TitleStorageFileTransferRequest::GetFilename(uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> _lk(_local_mutex);

    if (OutStringLength == nullptr || OutStringBuffer == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    *OutStringLength = _file_name.length();
    size_t copy_len = std::min<size_t>(FilenameStringBufferSizeBytes, _file_name.length());
    _file_name.copy(OutStringBuffer, copy_len);
    OutStringBuffer[copy_len - 1] = '\0';

    return EOS_EResult::EOS_Success;
}

/**
 * Attempt to cancel this file request in progress. This is a best-effort command and is not guaranteed to be successful if the request has completed before this function is called.
 *
 * @return EOS_Success if successful or already canceled, or EOS_NoChange if request is not cancelable.
 */
EOS_EResult EOSSDK_TitleStorageFileTransferRequest::CancelRequest()
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> _lk(_local_mutex);

    if (_done)
        return EOS_EResult::EOS_NoChange;

    _canceled = true;
    return EOS_EResult::EOS_Success;
}

void EOSSDK_TitleStorageFileTransferRequest::Release()
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> _lk(_local_mutex);
    _released = true;
}

}