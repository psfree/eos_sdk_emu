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

#include "eossdk_playerdatastorage.h"
#include "eossdk_platform.h"
#include "settings.h"

namespace sdk
{

constexpr decltype(EOSSDK_PlayerDataStorage::remote_folder) EOSSDK_PlayerDataStorage::remote_folder;

EOSSDK_PlayerDataStorage::EOSSDK_PlayerDataStorage():
    _game_save_folder(Settings::Inst().savepath)
{
    GetCB_Manager().register_frame(this);
}

EOSSDK_PlayerDataStorage::~EOSSDK_PlayerDataStorage()
{
    GetCB_Manager().unregister_frame(this);
}

std::string EOSSDK_PlayerDataStorage::build_path_string(std::string const& base_folder, std::string file)
{
    std::string path_str(_game_save_folder);
    path_str += PATH_SEPARATOR;
    path_str += base_folder;
    path_str += PATH_SEPARATOR;
    path_str += clean_path(file);

    return path_str;
}

bool EOSSDK_PlayerDataStorage::file_exists(std::string const& base_folder, std::string const& file)
{
    std::string file_path = std::move(build_path_string(base_folder, file));
    std::ifstream in_file(file_path);
    if (in_file)
        return true;

    return false;
}

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
void EOSSDK_PlayerDataStorage::QueryFile(const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
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
void EOSSDK_PlayerDataStorage::QueryFileList(const EOS_PlayerDataStorage_QueryFileListOptions* QueryFileListOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileListCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
}

/**
 * Create the cached copy of a file's metadata by filename. The metadata will be for the last retrieved or successfully saved version, and will not include any changes that have not
 * completed writing. The returned pointer must be released by the user when no longer needed.
 *
 * @param CopyFileMetadataOptions Object containing properties related to which user is requesting metadata, and for which filename
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_PlayerDataStorage_FileMetadata_Release.
 * @return EOS_EResult::EOS_Success if the metadata is currently cached, otherwise an error result explaining what went wrong
 */
EOS_EResult EOSSDK_PlayerDataStorage::CopyFileMetadataByFilename(const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    TRACE_FUNC();

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
EOS_EResult EOSSDK_PlayerDataStorage::GetFileMetadataCount(const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount)
{
    TRACE_FUNC();

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
EOS_EResult EOSSDK_PlayerDataStorage::CopyFileMetadataAtIndex(const EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    TRACE_FUNC();

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
void EOSSDK_PlayerDataStorage::DuplicateFile(const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
}

/**
 * Deletes an existing file in the cloud. If successful, the file's data will be removed from our local cache.
 *
 * @param DelteOptions Object containing properties related to which user is deleting the file, and what file name is
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the delete operation completes
 */
void EOSSDK_PlayerDataStorage::DeleteFile(const EOS_PlayerDataStorage_DeleteFileOptions* DeleteOptions, void* ClientData, const EOS_PlayerDataStorage_OnDeleteFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
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
EOS_HPlayerDataStorageFileTransferRequest EOSSDK_PlayerDataStorage::ReadFile(const EOS_PlayerDataStorage_ReadFileOptions* ReadOptions, void* ClientData, const EOS_PlayerDataStorage_OnReadFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();

    if (CompletionCallback == nullptr || ReadOptions == nullptr || ReadOptions->Filename == nullptr)
        return nullptr;

    EOS_HPlayerDataStorageFileTransferRequest func_result = nullptr;
    pFrameResult_t res(new FrameResult);

    EOS_PlayerDataStorage_ReadFileCallbackInfo& rfci = res->CreateCallback<EOS_PlayerDataStorage_ReadFileCallbackInfo>((CallbackFunc)CompletionCallback);

    rfci.ClientData = ClientData;
    {
        size_t len = strlen(ReadOptions->Filename) + 1;
        char* filename = new char[len];
        strncpy(filename, ReadOptions->Filename, len);
        rfci.Filename = filename;
    }
    rfci.LocalUserId = GetEOS_Connect().product_id();
    
    std::string file_path = std::move(build_path_string(remote_folder, ReadOptions->Filename));
    if (file_exists(remote_folder, ReadOptions->Filename))
    {
        LOG(Log::LogLevel::INFO, "Reading file: %s", file_path.c_str());

        auto res_handle = new EOSSDK_PlayerDataStorageFileTransferRequest;
        res_handle->_file_name = ReadOptions->Filename;
        res_handle->_file_path = std::move(file_path);

        func_result = reinterpret_cast<EOS_HPlayerDataStorageFileTransferRequest>(res_handle);
        rfci.ResultCode = EOS_EResult::EOS_Success;
    }
    else
    {
        LOG(Log::LogLevel::INFO, "File not found: %s", file_path.c_str());
        rfci.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);

    return func_result;
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
EOS_HPlayerDataStorageFileTransferRequest EOSSDK_PlayerDataStorage::WriteFile(const EOS_PlayerDataStorage_WriteFileOptions* WriteOptions, void* ClientData, const EOS_PlayerDataStorage_OnWriteFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_PlayerDataStorage::CBRunFrame()
{
    return false;
}

bool EOSSDK_PlayerDataStorage::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_PlayerDataStorage::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_PlayerDataStorage::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        case EOS_PlayerDataStorage_ReadFileCallbackInfo::k_iCallback:
        {
            EOS_PlayerDataStorage_ReadFileCallbackInfo& callback = res->GetCallback<EOS_PlayerDataStorage_ReadFileCallbackInfo>();
            // Free resources
            delete[] callback.Filename;
        }
        break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

}