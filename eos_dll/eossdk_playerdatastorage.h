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

#ifdef DeleteFile
#undef DeleteFile
#endif

namespace sdk
{
    class EOSSDK_PlayerDataStorageFileTransferRequest
    {
        friend class EOSSDK_PlayerDataStorage;

        std::mutex _local_mutex;

        bool        _released;
        std::string _file_path;
        std::string _file_name;

    public:
        EOSSDK_PlayerDataStorageFileTransferRequest();
        ~EOSSDK_PlayerDataStorageFileTransferRequest();

        bool released();

        EOS_EResult GetFileRequestState();
        EOS_EResult GetFilename(uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength);
        EOS_EResult CancelRequest();
        void Release();
    };

    class EOSSDK_PlayerDataStorage :
        public IRunFrame
    {
        static constexpr const char remote_folder[] = "remote";
        std::string _game_save_folder;

        std::set<EOSSDK_PlayerDataStorageFileTransferRequest*> _transfer_requests;

    public:
        EOSSDK_PlayerDataStorage();
        ~EOSSDK_PlayerDataStorage();

        std::string build_path_string(std::string const& base_folder, std::string file);
        bool file_exists(std::string const& base_folder, std::string const& file);

        // RunFrame is always called when running callbacks
        virtual bool CBRunFrame();
        // RunNetwork is run if you register to a network message and we received that message
        virtual bool RunNetwork(Network_Message_pb const& msg);
        // RunCallbacks is run when you sent a callback
        // True  = FrameResult_t has been filled with a result
        // False = FrameResult_t is not changed
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void QueryFile(const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback);
        void QueryFileList(const EOS_PlayerDataStorage_QueryFileListOptions* QueryFileListOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileListCompleteCallback CompletionCallback);
        EOS_EResult CopyFileMetadataByFilename(const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata);
        EOS_EResult GetFileMetadataCount(const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount);
        EOS_EResult CopyFileMetadataAtIndex(const EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata);
        void DuplicateFile(const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback);
        void DeleteFile(const EOS_PlayerDataStorage_DeleteFileOptions* DeleteOptions, void* ClientData, const EOS_PlayerDataStorage_OnDeleteFileCompleteCallback CompletionCallback);
        EOS_HPlayerDataStorageFileTransferRequest ReadFile(const EOS_PlayerDataStorage_ReadFileOptions* ReadOptions, void* ClientData, const EOS_PlayerDataStorage_OnReadFileCompleteCallback CompletionCallback);
        EOS_HPlayerDataStorageFileTransferRequest WriteFile(const EOS_PlayerDataStorage_WriteFileOptions* WriteOptions, void* ClientData, const EOS_PlayerDataStorage_OnWriteFileCompleteCallback CompletionCallback);
    };
}