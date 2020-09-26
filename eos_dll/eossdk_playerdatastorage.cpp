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

decltype(EOSSDK_PlayerDataStorage::remote_directory) EOSSDK_PlayerDataStorage::remote_directory("remote");

EOSSDK_PlayerDataStorage::EOSSDK_PlayerDataStorage()
{
    APP_LOG(Log::LogLevel::INFO, "PlayerDataStorage files will be search in %s", FileManager::canonical_path(remote_directory).c_str());

    GetCB_Manager().register_callbacks(this);
    GetCB_Manager().register_frame(this);
}

EOSSDK_PlayerDataStorage::~EOSSDK_PlayerDataStorage()
{
    GetCB_Manager().unregister_frame(this);
    GetCB_Manager().unregister_callbacks(this);
}

bool EOSSDK_PlayerDataStorage::get_metadata(std::string const& filename)
{
    std::string file_path(FileManager::join(remote_directory, FileManager::clean_path(filename)));
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
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_PlayerDataStorage_QueryFileCallbackInfo& qfci = res->CreateCallback<EOS_PlayerDataStorage_QueryFileCallbackInfo>((CallbackFunc)CompletionCallback);

    qfci.ClientData = ClientData;
    qfci.LocalUserId = GetEOS_Connect().get_myself()->first;

    if (QueryFileOptions == nullptr)
    {
        qfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        std::vector<std::string> files(std::move(FileManager::list_files(remote_directory, true)));

        auto it = std::find(files.begin(), files.end(), std::string(QueryFileOptions->Filename));

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
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_PlayerDataStorage_QueryFileListCallbackInfo& qflci = res->CreateCallback<EOS_PlayerDataStorage_QueryFileListCallbackInfo>((CallbackFunc)CompletionCallback);

    qflci.ClientData = ClientData;
    qflci.LocalUserId = GetEOS_Connect().get_myself()->first;

    if (QueryFileListOptions == nullptr)
    {
        qflci.FileCount = 0;
        qflci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        std::vector<std::string> files(std::move(FileManager::list_files(remote_directory, true)));

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
    GLOBAL_LOCK();

    if (CopyFileMetadataOptions == nullptr || CopyFileMetadataOptions->Filename == nullptr || OutMetadata == nullptr)
    {
        set_nullptr(OutMetadata);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _files_cache.find(CopyFileMetadataOptions->Filename);
    if (it == _files_cache.end())
    {
        set_nullptr(OutMetadata);
        return EOS_EResult::EOS_NotFound;
    }
    else
    {
        EOS_PlayerDataStorage_FileMetadata* metadata = new EOS_PlayerDataStorage_FileMetadata;

        metadata->ApiVersion = EOS_PLAYERDATASTORAGE_FILEMETADATA_API_LATEST;
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
 * @param GetFileMetadataCountOptions Object containing properties related to which user is requesting the metadata count
 * @param OutFileMetadataCount If successful, the count of metadata currently cached
 * @return EOS_EResult::EOS_Success if the input was valid, otherwise an error result explaining what went wrong
 *
 * @see EOS_PlayerDataStorage_CopyFileMetadataAtIndex
 */
EOS_EResult EOSSDK_PlayerDataStorage::GetFileMetadataCount(const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (GetFileMetadataCountOptions == nullptr || OutFileMetadataCount == nullptr)
    {
        if (OutFileMetadataCount != nullptr)
            *OutFileMetadataCount = 0;
        
        return EOS_EResult::EOS_InvalidParameters;
    }

    *OutFileMetadataCount = _files_cache.size();
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
    GLOBAL_LOCK();

    if (CopyFileMetadataOptions == nullptr || CopyFileMetadataOptions->Index >= _files_cache.size() || OutMetadata == nullptr)
    {
        set_nullptr(OutMetadata);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _files_cache.begin();
    std::advance(it, CopyFileMetadataOptions->Index);

    EOS_PlayerDataStorage_FileMetadata* metadata = new EOS_PlayerDataStorage_FileMetadata;

    metadata->ApiVersion = EOS_PLAYERDATASTORAGE_FILEMETADATA_API_LATEST;
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
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_PlayerDataStorage_DuplicateFileCallbackInfo& dfci = res->CreateCallback<EOS_PlayerDataStorage_DuplicateFileCallbackInfo>((CallbackFunc)CompletionCallback);

    dfci.ClientData = ClientData;
    dfci.LocalUserId = GetEOS_Connect().get_myself()->first;
    
    if (DuplicateOptions == nullptr || DuplicateOptions->SourceFilename == nullptr || DuplicateOptions->DestinationFilename == nullptr)
    {
        dfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        std::string src_file(FileManager::join(remote_directory, FileManager::clean_path(DuplicateOptions->SourceFilename)));
        std::ifstream in_file = FileManager::open_read(src_file, std::ios::binary);

        if (in_file)
        {
            std::string dst_file(FileManager::join(remote_directory, FileManager::clean_path(DuplicateOptions->DestinationFilename)));
            std::ofstream out_file = FileManager::open_write(dst_file, std::ios::binary | std::ios::trunc);
            if (out_file)
            {
                char* buff = new char[1024 * 1024];

                while(in_file.read(buff, 1024 * 1024).good())
                {
                    out_file.write(buff, in_file.gcount());
                }

                delete[]buff;

                // Also duplicate metadatas
                auto& src_cache = _files_cache[DuplicateOptions->SourceFilename];
                auto& dst_cache = _files_cache[DuplicateOptions->DestinationFilename];
                dst_cache.file_path = dst_file;
                dst_cache.file_size = src_cache.file_size;
                dst_cache.md5sum = src_cache.md5sum;
            }
            else
            {
                dfci.ResultCode = EOS_EResult::EOS_UnexpectedError;
            }
        }
        else
        {
            dfci.ResultCode = EOS_EResult::EOS_NotFound;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
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
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_PlayerDataStorage_DeleteFileCallbackInfo& dfci = res->CreateCallback<EOS_PlayerDataStorage_DeleteFileCallbackInfo>((CallbackFunc)CompletionCallback);

    dfci.ClientData = ClientData;
    dfci.LocalUserId = GetEOS_Connect().get_myself()->first;
    
    if (DeleteOptions == nullptr || DeleteOptions->Filename == nullptr)
    {
        dfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        auto it = _files_cache.find(DeleteOptions->Filename);
        if (it != _files_cache.end())
        {
            _files_cache.erase(it);
        }

        if (FileManager::delete_file(FileManager::join(remote_directory, DeleteOptions->Filename)))
        {
            dfci.ResultCode = EOS_EResult::EOS_Success;
        }
        else
        {
            dfci.ResultCode = EOS_EResult::EOS_NotFound;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
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
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
        return nullptr;

    EOS_HPlayerDataStorageFileTransferRequest func_result = nullptr;
    pFrameResult_t res(new FrameResult);

    EOS_PlayerDataStorage_ReadFileCallbackInfo& rfci = res->CreateCallback<EOS_PlayerDataStorage_ReadFileCallbackInfo>((CallbackFunc)CompletionCallback);
    rfci.ClientData = ClientData;
    rfci.LocalUserId = Settings::Inst().productuserid;

    if (ReadOptions == nullptr || ReadOptions->Filename == nullptr)
    {
        char* str = new char[1];
        *str = '\0';
        rfci.Filename = str;
    }
    else
    {
        size_t len = strlen(ReadOptions->Filename) + 1;
        char* str = new char[len];
        strncpy(str, ReadOptions->Filename, len);
        rfci.Filename = str;
    }

    if (ReadOptions == nullptr || ReadOptions->Filename == nullptr || ReadOptions->ReadFileDataCallback == nullptr)
    {
        rfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        res->done = true;
    }
    else
    {
        size_t len = strlen(ReadOptions->Filename) + 1;
        char* str = new char[len];
        strncpy(str, ReadOptions->Filename, len);
        rfci.Filename = str;

        std::string file_path(FileManager::join(remote_directory, FileManager::clean_path(rfci.Filename)));
        if (FileManager::is_file(file_path))
        {
            APP_LOG(Log::LogLevel::INFO, "Start Reading file: %s", file_path.c_str());

            EOSSDK_PlayerDataStorageFileTransferRequest*& res_obj = _transferts[res];
            res_obj = new EOSSDK_PlayerDataStorageFileTransferRequest;
            res_obj->set_read_transfert(ReadOptions);

            func_result = reinterpret_cast<EOS_HPlayerDataStorageFileTransferRequest>(res_obj);
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
    GLOBAL_LOCK();

    if (CompletionCallback == nullptr)
    {
        return nullptr;
    }

    EOS_HPlayerDataStorageFileTransferRequest func_result = nullptr;
    pFrameResult_t res(new FrameResult);
    EOS_PlayerDataStorage_WriteFileCallbackInfo& wfci = res->CreateCallback<EOS_PlayerDataStorage_WriteFileCallbackInfo>((CallbackFunc)CompletionCallback);

    wfci.ClientData = ClientData;
    wfci.LocalUserId = GetEOS_Connect().get_myself()->first;

    if (WriteOptions == nullptr || WriteOptions->Filename == nullptr)
    {
        char* str = new char[1];
        *str = '\0';
        wfci.Filename = str;
    }
    else
    {
        size_t len = strlen(WriteOptions->Filename) + 1;
        char* str = new char[len];
        strncpy(str, WriteOptions->Filename, len);
        wfci.Filename = str;
    }

    if (WriteOptions == nullptr || WriteOptions->Filename == nullptr || WriteOptions->WriteFileDataCallback == nullptr)
    {
        wfci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        res->done = true;
    }
    else
    {
        EOSSDK_PlayerDataStorageFileTransferRequest*& res_obj = _transferts[res];
        res_obj = new EOSSDK_PlayerDataStorageFileTransferRequest;
        res_obj->set_write_transfert(WriteOptions);

        APP_LOG(Log::LogLevel::INFO, "Start Writing file: %s", res_obj->_file_name.c_str());

        func_result = reinterpret_cast<EOS_HPlayerDataStorageFileTransferRequest>(res_obj);
    }

    GetCB_Manager().add_callback(this, res);
    return func_result;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_PlayerDataStorage::CBRunFrame()
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

bool EOSSDK_PlayerDataStorage::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
    {
        case EOS_PlayerDataStorage_ReadFileCallbackInfo::k_iCallback:
        {
            EOS_PlayerDataStorage_ReadFileCallbackInfo& callback = res->GetCallback<EOS_PlayerDataStorage_ReadFileCallbackInfo>();
            EOSSDK_PlayerDataStorageFileTransferRequest& transfert = *_transferts[res];

            if (transfert.canceled())
            {
                callback.ResultCode = EOS_EResult::EOS_Canceled;
                transfert._done = true;
                res->done = true;
            }
            else
            {
                EOS_PlayerDataStorage_ReadFileDataCallbackInfo rfdci;
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
                    case EOS_PlayerDataStorage_EReadResult::EOS_RR_FailRequest:
                    {
                        callback.ResultCode = EOS_EResult::EOS_PlayerDataStorage_UserErrorFromDataCallback;
                        transfert._done = true;
                        res->done = true;
                    }
                    break;

                    case EOS_PlayerDataStorage_EReadResult::EOS_RR_CancelRequest:
                    {
                        callback.ResultCode = EOS_EResult::EOS_Canceled;
                        transfert._canceled = true;
                        transfert._done = true;
                        res->done = true;
                    }
                    break;

                    case EOS_PlayerDataStorage_EReadResult::EOS_RR_ContinueReading:
                    {
                    }
                    break;
                }
            }
        }
        break;

        case EOS_PlayerDataStorage_WriteFileCallbackInfo::k_iCallback:
        {
            EOS_PlayerDataStorage_WriteFileCallbackInfo& callback = res->GetCallback<EOS_PlayerDataStorage_WriteFileCallbackInfo>();
            EOSSDK_PlayerDataStorageFileTransferRequest& transfert = *_transferts[res];

            if (transfert._canceled)
            {
                callback.ResultCode = EOS_EResult::EOS_Canceled;
                transfert._done = true;
                res->done = true;
            }
            else
            {
                size_t offset = transfert._file_buffer.size();
                transfert._file_buffer.resize(offset + transfert._chunk_size * 2);

                EOS_PlayerDataStorage_WriteFileDataCallbackInfo wfdci;
                wfdci.ClientData = callback.ClientData;
                wfdci.Filename = callback.Filename;
                wfdci.LocalUserId = callback.LocalUserId;

                uint32_t buff_len = transfert._chunk_size * 2;
                wfdci.DataBufferLengthBytes = buff_len;

                switch (transfert._write_callback(&wfdci, &transfert._file_buffer[offset], &buff_len))
                {
                    case EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest:
                    {
                        callback.ResultCode = EOS_EResult::EOS_PlayerDataStorage_UserErrorFromDataCallback;
                        transfert._done = true;
                        res->done = true;
                    }
                    break;

                    case EOS_PlayerDataStorage_EWriteResult::EOS_WR_CancelRequest:
                    {
                        transfert._canceled = true;
                        transfert._done = true;
                        callback.ResultCode = EOS_EResult::EOS_Canceled;
                        res->done = true;
                    }
                    break;

                    case EOS_PlayerDataStorage_EWriteResult::EOS_WR_ContinueWriting:
                    {
                        transfert._file_buffer.resize(offset + buff_len);
                    }
                    break;

                    case EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest:
                    {
                        transfert._file_buffer.resize(offset + buff_len);
                        transfert._done = true;
                        std::ofstream out_file(FileManager::open_write(FileManager::join(remote_directory, FileManager::clean_path(transfert._file_name)), std::ios::binary | std::ios::trunc));
                        if (out_file)
                        {
                            out_file.write((const char*)transfert._file_buffer.data(), transfert._file_buffer.size());
                            out_file.close();
                            if(get_metadata(transfert._file_name))
                            {
                                callback.ResultCode = EOS_EResult::EOS_Success;
                            }
                            else
                            {
                                FileManager::delete_file(FileManager::join(remote_directory, transfert._file_name));
                                callback.ResultCode = EOS_EResult::EOS_UnexpectedError;
                            }
                        }
                        else
                        {
                            callback.ResultCode = EOS_EResult::EOS_UnexpectedError;
                        }
                        res->done = true;
                    }
                }
            }
        }
        break;
    }

    return res->done;
}

void EOSSDK_PlayerDataStorage::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
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

        case EOS_PlayerDataStorage_WriteFileCallbackInfo::k_iCallback:
        {
            EOS_PlayerDataStorage_WriteFileCallbackInfo& callback = res->GetCallback<EOS_PlayerDataStorage_WriteFileCallbackInfo>();
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