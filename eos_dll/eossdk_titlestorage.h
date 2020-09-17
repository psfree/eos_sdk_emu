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

namespace sdk
{
    class EOSSDK_TitleStorageFileTransferRequest
    {
        friend class EOSSDK_TitleStorage;

        std::mutex _local_mutex;

        std::string _file_name;

        bool _done;
        bool _canceled;
        bool _released;

        EOS_TitleStorage_OnReadFileDataCallback  _read_callback;
        EOS_TitleStorage_OnFileTransferProgressCallback _progress_callback;
        uint32_t _chunk_size;
        uint32_t _file_size;

        std::vector<uint8_t> _file_buffer;
        std::ifstream _input_file;

        void set_read_transfert(const EOS_TitleStorage_ReadFileOptions* ReadOptions);

    public:
        EOSSDK_TitleStorageFileTransferRequest();
        ~EOSSDK_TitleStorageFileTransferRequest();

        bool canceled();
        bool released();

        EOS_EResult GetFileRequestState();
        EOS_EResult GetFilename(uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength);
        EOS_EResult CancelRequest();
        void Release();
    };

    class EOSSDK_TitleStorage :
        public IRunFrame
    {
        struct file_metadata_t
        {
            std::string file_path;
            size_t file_size;
            std::string md5sum;
        };

        std::unordered_map<pFrameResult_t, EOSSDK_TitleStorageFileTransferRequest*> _transferts;
        nlohmann::fifo_map<std::string, file_metadata_t> _files_cache;

        bool get_metadata(std::string const& filename);

    public:
        EOSSDK_TitleStorage();
        ~EOSSDK_TitleStorage();

        static const std::string title_directory;

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void                                 QueryFile(const EOS_TitleStorage_QueryFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileCompleteCallback CompletionCallback);
        void                                 QueryFileList(const EOS_TitleStorage_QueryFileListOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileListCompleteCallback CompletionCallback);
        EOS_EResult                          CopyFileMetadataByFilename(const EOS_TitleStorage_CopyFileMetadataByFilenameOptions* Options, EOS_TitleStorage_FileMetadata** OutMetadata);
        uint32_t                             GetFileMetadataCount(const EOS_TitleStorage_GetFileMetadataCountOptions* Options);
        EOS_EResult                          CopyFileMetadataAtIndex(const EOS_TitleStorage_CopyFileMetadataAtIndexOptions* Options, EOS_TitleStorage_FileMetadata** OutMetadata);
        EOS_HTitleStorageFileTransferRequest ReadFile(const EOS_TitleStorage_ReadFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnReadFileCompleteCallback CompletionCallback);
        EOS_EResult                          DeleteCache(const EOS_TitleStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_TitleStorage_OnDeleteCacheCompleteCallback CompletionCallback);
    };
}
