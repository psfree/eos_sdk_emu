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

decltype(EOSSDK_TitleStorage::title_directory) EOSSDK_TitleStorage::title_directory("titlestorage");

EOSSDK_TitleStorage::EOSSDK_TitleStorage()
{
    APP_LOG(Log::LogLevel::INFO, "TitleStorage files will be search in %s", FileManager::canonical_path(title_directory).c_str());

    GetCB_Manager().register_callbacks(this);
    GetCB_Manager().register_frame(this);
}

EOSSDK_TitleStorage::~EOSSDK_TitleStorage()
{
    GetCB_Manager().unregister_frame(this);
    GetCB_Manager().unregister_callbacks(this);
}

bool EOSSDK_TitleStorage::get_metadata(std::string const& filename)
{
    std::string file_path(FileManager::join(title_directory, FileManager::clean_path(filename)));
    std::ifstream in_file = FileManager::open_read(file_path, std::ios::binary);
    if (in_file)
    {
        auto& metadata = _files_cache[filename];
        in_file.seekg(0, std::ios::end);
        metadata.file_size = in_file.tellg();
        in_file.seekg(0, std::ios::beg);

        std::string buffer(metadata.file_size, '\0');
        in_file.read(&buffer[0], metadata.file_size);

        metadata.md5sum = std::move(md5(buffer));

        metadata.file_path = std::move(file_path);

        return true;
    }

    auto it = _files_cache.find(filename);
    if (it != _files_cache.end())
    {// Couldn't open the file, remove it from known files
        _files_cache.erase(it);
    }

    return false;
}

/**
 * The following EOS_TitleStorage_* functions allow you to query metadata for available files from title storage as well as download and read their contents.
 */

/**
 * Query a specific file's metadata, such as file names, size, and a MD5 hash of the data. This is not required before a file may be opened. Once a file has
 * been queried, its metadata will be available by the EOS_TitleStorage_CopyFileMetadataAtIndex and EOS_TitleStorage_CopyFileMetadataByFilename functions.
 *
 * @param Options Object containing properties related to which user is querying files, and what file is being queried
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the query operation completes
 *
 * @see EOS_TitleStorage_GetFileMetadataCount
 * @see EOS_TitleStorage_CopyFileMetadataAtIndex
 * @see EOS_TitleStorage_CopyFileMetadataByFilename
 */
void EOSSDK_TitleStorage::QueryFile(const EOS_TitleStorage_QueryFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_TitleStorage_QueryFileCallbackInfo& qfci = res->CreateCallback<EOS_TitleStorage_QueryFileCallbackInfo>((CallbackFunc)CompletionCallback);

    qfci.ClientData = ClientData;
    qfci.LocalUserId = GetEOS_Connect().get_myself()->first;

    if (Options == nullptr || Options->Filename == nullptr)
    {
        qfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        std::vector<std::string> files(std::move(FileManager::list_files(title_directory, true)));

        auto it = std::find(files.begin(), files.end(), std::string(Options->Filename));

        if (it == files.end())
        {
            qfci.ResultCode = EOS_EResult::EOS_NotFound;
        }
        else
        {
            std::replace(it->begin(), it->end(), '\\', '/');
            get_metadata(*it);
            qfci.ResultCode = EOS_EResult::EOS_Success;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Query the file metadata, such as file names, size, and a MD5 hash of the data, for all files available for current user based on their settings (such as game role) and tags provided.
 * This is not required before a file can be downloaded by name.
 *
 * @param Options Object containing properties related to which user is querying files and the list of tags
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the query operation completes
 *
 */
void EOSSDK_TitleStorage::QueryFileList(const EOS_TitleStorage_QueryFileListOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileListCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_TitleStorage_QueryFileListCallbackInfo& qflci = res->CreateCallback<EOS_TitleStorage_QueryFileListCallbackInfo>((CallbackFunc)CompletionCallback);

    qflci.ClientData = ClientData;
    qflci.LocalUserId = GetEOS_Connect().get_myself()->first;

    if (Options == nullptr)
    {
        qflci.FileCount = 0;
        qflci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        std::vector<std::string> files(std::move(FileManager::list_files(title_directory, true)));

        _files_cache.clear();
        for (auto& file_name : files)
        {
            std::replace(file_name.begin(), file_name.end(), '\\', '/');
            get_metadata(file_name);
        }

        qflci.FileCount = _files_cache.size();
        qflci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Create a cached copy of a file's metadata by filename. The metadata will be for the last retrieved or successfully saved version, and will not include any changes that have not
 * completed writing. The returned pointer must be released by the user when no longer needed.
 *
 * @param Options Object containing properties related to which user is requesting metadata, and for which filename
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_TitleStorage_FileMetadata_Release.
 * @return EOS_Success if the metadata is currently cached, otherwise an error result explaining what went wrong
 */
EOS_EResult EOSSDK_TitleStorage::CopyFileMetadataByFilename(const EOS_TitleStorage_CopyFileMetadataByFilenameOptions* Options, EOS_TitleStorage_FileMetadata** OutMetadata)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->Filename == nullptr || OutMetadata == nullptr)
    {
        set_nullptr(OutMetadata);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _files_cache.find(Options->Filename);
    if (it == _files_cache.end())
    {
        set_nullptr(OutMetadata);
        return EOS_EResult::EOS_NotFound;
    }
    else
    {
        EOS_TitleStorage_FileMetadata* metadata = new EOS_TitleStorage_FileMetadata;
        
        metadata->ApiVersion = EOS_TITLESTORAGE_FILEMETADATA_API_LATEST;
        {
            size_t len = it->first.length() + 1;
            char* str = new char[len];
            strncpy(str, it->first.c_str(), len);
            metadata->Filename = str;
        }
        {
            size_t len = it->second.md5sum.length() + 1;
            char* str = new char[len];
            strncpy(str, it->second.md5sum.c_str(), len);
            metadata->MD5Hash = str;
        }
        metadata->FileSizeBytes = it->second.file_size;

        *OutMetadata = metadata;
    }

    return EOS_EResult::EOS_Success;
}

/**
 * Get the count of files we have previously queried information for and files we have previously read from / written to.
 *
 * @param Options Object containing properties related to which user is requesting the metadata count
 * @return If successful, the count of metadata currently cached. Returns 0 on failure.
 *
 * @see EOS_TitleStorage_CopyFileMetadataAtIndex
 */
uint32_t EOSSDK_TitleStorage::GetFileMetadataCount(const EOS_TitleStorage_GetFileMetadataCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr)
    {
        return 0;
    }

    return _files_cache.size();
}

/**
 * Get the cached copy of a file's metadata by index. The metadata will be for the last retrieved version. The returned pointer must be released by the user when no longer needed.
 *
 * @param Options Object containing properties related to which user is requesting metadata, and at what index
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_TitleStorage_FileMetadata_Release.
 * @return EOS_Success if the requested metadata is currently cached, otherwise an error result explaining what went wrong.
 *
 * @see EOS_TitleStorage_GetFileMetadataCount
 * @see EOS_TitleStorage_FileMetadata_Release
 */
EOS_EResult EOSSDK_TitleStorage::CopyFileMetadataAtIndex(const EOS_TitleStorage_CopyFileMetadataAtIndexOptions* Options, EOS_TitleStorage_FileMetadata** OutMetadata)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->Index >= _files_cache.size() || OutMetadata == nullptr)
    {
        set_nullptr(OutMetadata);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _files_cache.begin();
    std::advance(it, Options->Index);

    EOS_TitleStorage_FileMetadata* metadata = new EOS_TitleStorage_FileMetadata;

    metadata->ApiVersion = EOS_TITLESTORAGE_FILEMETADATA_API_LATEST;
    {
        size_t len = it->first.length() + 1;
        char* str = new char[len];
        strncpy(str, it->first.c_str(), len);
        metadata->Filename = str;
    }
    {
        size_t len = it->second.md5sum.length() + 1;
        char* str = new char[len];
        strncpy(str, it->second.md5sum.c_str(), len);
        metadata->MD5Hash = str;
    }
    metadata->FileSizeBytes = it->second.file_size;

    *OutMetadata = metadata;

    return EOS_EResult::EOS_Success;
}

/**
 * Retrieve the contents of a specific file, potentially downloading the contents if we do not have a local copy, from the cloud. This request will occur asynchronously, potentially over
 * multiple frames. All callbacks for this function will come from the same thread that the SDK is ticked from. If specified, the FileTransferProgressCallback will always be called at
 * least once if the request is started successfully.
 *
 * @param Options Object containing properties related to which user is opening the file, what the file's name is, and related mechanisms for copying the data
 * @param ClientData Optional pointer to help clients track this request, that is returned in associated callbacks
 * @param CompletionCallback This function is called when the read operation completes
 * @return A valid Title Storage File Request handle if successful, or NULL otherwise. Data contained in the completion callback will have more detailed information about issues with the request in failure cases. This handle must be released when it is no longer needed
 *
 * @see EOS_TitleStorageFileTransferRequest_Release
 */
EOS_HTitleStorageFileTransferRequest EOSSDK_TitleStorage::ReadFile(const EOS_TitleStorage_ReadFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnReadFileCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return nullptr;

    EOS_HTitleStorageFileTransferRequest func_result = nullptr;
    pFrameResult_t res(new FrameResult);

    EOS_TitleStorage_ReadFileCallbackInfo& rfci = res->CreateCallback<EOS_TitleStorage_ReadFileCallbackInfo>((CallbackFunc)CompletionCallback);
    rfci.ClientData = ClientData;
    rfci.LocalUserId = Settings::Inst().productuserid;

    if (Options == nullptr || Options->Filename == nullptr)
    {
        char* str = new char[1];
        *str = '\0';
        rfci.Filename = str;
    }
    else
    {
        size_t len = strlen(Options->Filename) + 1;
        char* str = new char[len];
        strncpy(str, Options->Filename, len);
        rfci.Filename = str;
    }

    if (Options == nullptr || Options->Filename == nullptr || Options->ReadFileDataCallback == nullptr)
    {
        rfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        res->done = true;
    }
    else
    {
        size_t len = strlen(Options->Filename) + 1;
        char* str = new char[len];
        strncpy(str, Options->Filename, len);
        rfci.Filename = str;

        std::string file_path(FileManager::join(title_directory, FileManager::clean_path(rfci.Filename)));
        if (FileManager::is_file(file_path))
        {
            APP_LOG(Log::LogLevel::INFO, "Start Reading file: %s", file_path.c_str());

            EOSSDK_TitleStorageFileTransferRequest*& res_obj = _transferts[res];
            res_obj = new EOSSDK_TitleStorageFileTransferRequest;
            res_obj->set_read_transfert(Options);

            func_result = reinterpret_cast<EOS_HTitleStorageFileTransferRequest>(res_obj);
        }
        else
        {
            APP_LOG(Log::LogLevel::INFO, "File not found: %s", file_path.c_str());
            rfci.ResultCode = EOS_EResult::EOS_NotFound;

            res->done = true;
        }
    }

    GetCB_Manager().add_callback(this, res);
    return func_result;
}

/**
 * Clear previously cached file data. This operation will be done asynchronously. All cached files except those corresponding to the transfers in progress will be removed.
 * Warning: Use this with care. Cache system generally tries to clear old and unused cached files from time to time. Unnecessarily clearing cache can degrade performance as SDK will have to re-download data.
 *
 * @param Options Object containing properties related to which user is deleting cache
 * @param ClientData Optional pointer to help clients track this request, that is returned in associated callbacks
 * @param CompletionCallback This function is called when the delete cache operation completes
 * @return EOS_Success if the operation was started correctly, otherwise an error result explaining what went wrong
 */
EOS_EResult EOSSDK_TitleStorage::DeleteCache(const EOS_TitleStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_TitleStorage_OnDeleteCacheCompleteCallback CompletionCallback)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionCallback != nullptr)
    {
        pFrameResult_t res(new FrameResult);
        EOS_TitleStorage_DeleteCacheCallbackInfo& dcci = res->CreateCallback<EOS_TitleStorage_DeleteCacheCallbackInfo>((CallbackFunc)CompletionCallback);

        dcci.ClientData = ClientData;
        dcci.LocalUserId = Settings::Inst().productuserid;
        dcci.ResultCode = EOS_EResult::EOS_Success;

        res->done = true;
        GetCB_Manager().add_callback(this, res);
    }

    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_TitleStorage::CBRunFrame()
{
    GLOBAL_LOCK();

    for (auto it = _transferts.begin(); it != _transferts.end();)
    {
        if (it->second->released())
        {
            delete it->second;
            it = _transferts.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return true;
}

bool EOSSDK_TitleStorage::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_TitleStorage::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
    {
        case EOS_TitleStorage_ReadFileCallbackInfo::k_iCallback:
        {
            EOS_TitleStorage_ReadFileCallbackInfo& callback = res->GetCallback<EOS_TitleStorage_ReadFileCallbackInfo>();
            EOSSDK_TitleStorageFileTransferRequest& transfert = *_transferts[res];

            if (transfert.canceled())
            {
                callback.ResultCode = EOS_EResult::EOS_Canceled;
                transfert._done = true;
                res->done = true;
            }
            else
            {
                EOS_TitleStorage_ReadFileDataCallbackInfo rfdci;
                rfdci.ClientData = callback.ClientData;
                rfdci.Filename = callback.Filename;
                rfdci.LocalUserId = callback.LocalUserId;
                rfdci.TotalFileSizeBytes = transfert._file_size;

                transfert._input_file.read((char*)&transfert._file_buffer[0], transfert._chunk_size);
                size_t read_len = transfert._input_file.gcount();

                rfdci.bIsLastChunk = (read_len != transfert._chunk_size ? EOS_TRUE : EOS_FALSE);
                if (rfdci.bIsLastChunk == EOS_TRUE)
                {
                    transfert._done = true;
                    res->done = true;
                    callback.ResultCode = EOS_EResult::EOS_Success;
                }

                rfdci.DataChunk = &transfert._file_buffer[0];
                rfdci.DataChunkLengthBytes = read_len;
                switch (transfert._read_callback(&rfdci))
                {
                    case EOS_TitleStorage_EReadResult::EOS_TS_RR_FailRequest:
                    {
                        callback.ResultCode = EOS_EResult::EOS_PlayerDataStorage_UserErrorFromDataCallback;
                        transfert._done = true;
                        res->done = true;
                    }
                    break;

                    case EOS_TitleStorage_EReadResult::EOS_TS_RR_CancelRequest:
                    {
                        callback.ResultCode = EOS_EResult::EOS_Canceled;
                        transfert._canceled = true;
                        transfert._done = true;
                        res->done = true;
                    }
                    break;

                    case EOS_TitleStorage_EReadResult::EOS_TS_RR_ContinueReading:
                    {
                    }
                    break;
                }
            }
        }
        break;
    }

    return res->done;
}

void EOSSDK_TitleStorage::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        case EOS_TitleStorage_ReadFileCallbackInfo::k_iCallback:
        {
            EOS_TitleStorage_ReadFileCallbackInfo& callback = res->GetCallback<EOS_TitleStorage_ReadFileCallbackInfo>();
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