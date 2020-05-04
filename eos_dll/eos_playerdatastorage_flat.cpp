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

#include "eos_playerdatastorage.h"

using namespace sdk;

/**
 * The following EOS_PlayerDataStorage_* functions allow you to query file metadata; create/upload files; and duplicate, read, and delete existing files
 */

 /**
  * Query a specific file's metadata, such as file names, size, and a MD5 hash of the data. This is not required before a file may be opened, saved, copied, or deleted. Once a file has
  * been queried, its metadata will be available by the EOS_PlayerDataStorage_CopyFileMetadataAtIndex and EOS_PlayerDataStorage_CopyFileMetadataByFilename functions.
  *
  * @param Options Object containing properties related to which user is querying files, and what file is being queried
  * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
  * @param CompletionCallback This function is called when the query operation completes
  *
  * @see EOS_PlayerDataStorage_GetFileMetadataCount
  * @see EOS_PlayerDataStorage_CopyFileMetadataAtIndex
  * @see EOS_PlayerDataStorage_CopyFileMetadataByFilename
  */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
}

/**
 * Query the file metadata, such as file names, size, and a MD5 hash of the data, for all files owned by this user for this application. This is not required before a file may be opened,
 * saved, copied, or deleted.
 *
 * @param Options Object containing properties related to which user is querying files
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the query operation completes
 *
 * @see EOS_PlayerDataStorage_GetFileMetadataCount
 * @see EOS_PlayerDataStorage_CopyFileMetadataAtIndex
 * @see EOS_PlayerDataStorage_CopyFileMetadataByFilename
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFileList(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileListOptions* QueryFileListOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileListCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
}

/**
 * Create the cached copy of a file's metadata by filename. The metadata will be for the last retrieved or successfully saved version, and will not include any changes that have not
 * completed writing. The returned pointer must be released by the user when no longer needed.
 *
 * @param CopyFileMetadataOptions Object containing properties related to which user is requesting metadata, and for which filename
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_PlayerDataStorage_FileMetadata_Release.
 * @return EOS_EResult::EOS_Success if the metadata is currently cached, otherwise an error result explaining what went wrong
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataByFilename(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Get the count of files we have previously queried information for and files we have previously read from / written to.
 *
 * @param GetFileMetadataCountOptions Object containing properties related to which user is requesting the metadata count
 * @param OutFileMetadataCount If successful, the count of metadata currently cached
 * @return EOS_EResult::EOS_Success if the input was valid, otherwise an error result explaining what went wrong
 *
 * @see EOS_PlayerDataStorage_CopyFileMetadataAtIndex
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_GetFileMetadataCount(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Get the cached copy of a file's metadata by index. The metadata will be for the last retrieved or successfully saved version, and will not include any local changes that have not been
 * committed by calling SaveFile. The returned pointer must be released by the user when no longer needed.
 *
 * @param CopyFileMetadataOptions Object containing properties related to which user is requesting metadata, and at what index
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_PlayerDataStorage_FileMetadata_Release.
 * @return EOS_EResult::EOS_Success if the requested metadata is currently cached, otherwise an error result explaining what went wrong
 *
 * @see EOS_PlayerDataStorage_GetFileMetadataCount
 * @see EOS_PlayerDataStorage_FileMetadata_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataAtIndex(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Copies the data of an existing file to a new filename. This action happens entirely on the server and will not upload the contents of the source destination file from the host. This
 * function paired with a subsequent EOS_PlayerDataStorage_DeleteFile can be used to rename a file. If successful, the destination file's metadata will be updated in our local cache.
 *
 * @param DuplicateOptions Object containing properties related to which user is duplicating the file, and what the source and destination file names are
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the duplicate operation completes
 *
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DuplicateFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
}

/**
 * Deletes an existing file in the cloud. If successful, the file's data will be removed from our local cache.
 *
 * @param DelteOptions Object containing properties related to which user is deleting the file, and what file name is
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the delete operation completes
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DeleteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteFileOptions* DeleteOptions, void* ClientData, const EOS_PlayerDataStorage_OnDeleteFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
}

/**
 * Retrieve the contents of a specific file, potentially downloading the contents if we do not have a local copy, from the cloud. This request will occur asynchronously, potentially over
 * multiple frames. All callbacks for this function will come from the same thread that the SDK is ticked from. If specified, the FileTransferProgressCallback will always be called at
 * least once if the request is started successfully.
 *
 * @param ReadOptions Object containing properties related to which user is opening the file, what the file's name is, and related mechanisms for copying the data
 * @param ClientData Optional pointer to help clients track this request, that is returned in associated callbacks
 * @param CompletionCallback This function is called when the read operation completes
 * @return A valid Player Data Storage File Request handle if successful, or NULL otherwise. Data contained in the completion callback will have more detailed information about issues with the request in failure cases. This handle must be released when it is no longer needed
 *
 * @see EOS_PlayerDataStorageFileTransferRequest_Release
 */
EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_ReadFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_ReadFileOptions* ReadOptions, void* ClientData, const EOS_PlayerDataStorage_OnReadFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return nullptr;
}

/**
 * Write new data to a specific file, potentially overwriting any existing file by the same name, to the cloud. This request will occur asynchronously, potentially over multiple frames.
 * All callbacks for this function will come from the same thread that the SDK is ticked from. If specified, the FileTransferProgressCallback will always be called at least once if the
 * request is started successfully.
 *
 * @param WriteOptions Object containing properties related to which user is writing the file, what the file's name is, and related mechanisms for writing the data
 * @param ClientData Optional pointer to help clients track this request, that is returned in associated callbacks
 * @param CompletionCallback This function is called when the write operation completes
 * @return A valid Player Data Storage File Request handle if successful, or NULL otherwise. Data contained in the completion callback will have more detailed information about issues with the request in failure cases. This handle must be released when it is no longer needed
 *
 * @see EOS_PlayerDataStorageFileTransferRequest_Release
 */
EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_WriteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_WriteFileOptions* WriteOptions, void* ClientData, const EOS_PlayerDataStorage_OnWriteFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return nullptr;
}

/**
 * Below are helper functions to retrieve information about a file request handle, or to attempt to cancel a request in progress.
 */

 /**
  * Get the current state of a file request.
  *
  * @return EOS_Success if complete and successful, EOS_EResult::EOS_Pending if the request is still in progress, or another state for failure.
  */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(EOS_HPlayerDataStorageFileTransferRequest Handle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Get the file name of the file this request is for. OutStringLength will always be set to the string length of the file name if it is not NULL.
 *
 * @param FilenameStringBufferSizeBytes The maximum number of bytes that can be written to OutStringBuffer
 * @param OutStringBuffer The buffer to write the NULL-terminated utf8 file name into, if successful
 * @param OutStringLength How long the file name is (not including null terminator)
 * @return EOS_Success if the file name was successfully written to OutFilenameBuffer, a failure result otherwise
 *
 * @see EOS_PLAYERDATASTORAGE_FILENAME_MAX_LENGTH
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFilename(EOS_HPlayerDataStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Attempt to cancel this file request in progress. This is a best-effort command and is not guaranteed to be successful if the request has completed before this function is called.
 *
 * @return EOS_Success if successful, or EOS_NoChange if request is not cancelable or is already canceled
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_CancelRequest(EOS_HPlayerDataStorageFileTransferRequest Handle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_PlayerDataStorage*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Free the memory used by the file metadata
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_FileMetadata_Release(EOS_PlayerDataStorage_FileMetadata* FileMetadata);

/**
 * Free the memory used by a cloud-storage file request handle. This will not cancel a request in progress.
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorageFileTransferRequest_Release(EOS_HPlayerDataStorageFileTransferRequest PlayerDataStorageFileTransferHandle);