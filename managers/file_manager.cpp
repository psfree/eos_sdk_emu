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

#include "file_manager.h"

constexpr decltype(FileManager::separator) FileManager::separator;

FileManager::FileManager()
{}

FileManager::~FileManager()
{}

inline FileManager& FileManager::Inst()
{
    static FileManager inst;
    return inst;
}

std::string FileManager::filename(std::string const& path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
        return path.substr(pos);

    return path;
}

std::string FileManager::dirname(std::string const& path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
        return std::string();

    return std::string(path.begin(), path.begin() + pos + 1);
}

std::string const& FileManager::root_dir()
{
    auto& self = Inst();
    return self._root_directory;
}

std::string FileManager::canonical_path(std::string const& _path)
{
    auto& self = Inst();
    return join(self._root_directory, clean_path(_path));
}

size_t FileManager::file_size(std::string const& _path)
{
    std::string path(canonical_path(_path));
    std::ifstream in_file(path, std::ios::in | std::ios::binary);
    if (in_file)
    {
        in_file.seekg(0, std::ios::end);
        return in_file.tellg();
    }

    return 0;
}

time_t FileManager::file_atime(std::string const& _path)
{
    std::string path(canonical_path(_path));
    struct stat file_stat = {};
    if (stat(path.c_str(), &file_stat) != 0)
        return 0;

    return file_stat.st_atime;
}

time_t FileManager::file_mtime(std::string const& _path)
{
    std::string path(canonical_path(_path));
    struct stat file_stat = {};
    if (stat(path.c_str(), &file_stat) != 0)
        return 0;

    return file_stat.st_mtime;
}

time_t FileManager::file_ctime(std::string const& _path)
{
    std::string path(canonical_path(_path));
    struct stat file_stat = {};
    if (stat(path.c_str(), &file_stat) != 0)
        return 0;

    return file_stat.st_ctime;
}

std::ifstream FileManager::open_read(std::string const& path, std::ios::openmode open_mode)
{
    return std::ifstream(canonical_path(path), open_mode | std::ios::in);
}

std::ofstream FileManager::open_write(std::string const& _path, std::ios::openmode open_mode)
{
    std::string path(canonical_path(_path));
    create_directory(dirname(_path));
    return std::ofstream(path, open_mode | std::ios::out);
}

std::fstream  FileManager::open_readwrite(std::string const& _path, std::ios::openmode open_mode)
{
    std::string path(canonical_path(_path));
    create_directory(dirname(_path));
    return std::fstream(path, open_mode | std::ios::out | std::ios::in);
}

#ifdef __WINDOWS__

bool FileManager::is_absolute(std::string const& path)
{
    return path.length() >= 2 && (((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) && path[1] == ':');
}

bool FileManager::set_root_dir(std::string const& _directory)
{
    auto& self = Inst();
    std::string directory;

    if (!is_absolute(_directory))
    {
        DWORD size = GetCurrentDirectoryW(0, nullptr);
        if (size == 0)
            return false;

        std::wstring wdirectory;
        ++size;
        wdirectory.resize(size);
        wdirectory.resize(GetCurrentDirectoryW(size, &wdirectory[0]));
        wdirectory += L'\\';

        std::string tmp;
        utf8::utf16to8(wdirectory.begin(), wdirectory.end(), std::back_inserter(tmp));
        directory = std::move(tmp + _directory);
    }
    else
    {
        directory = _directory;
    }

    self._root_directory = std::move(clean_path(directory));

    if (self._root_directory.empty())
        self._root_directory.assign(directory.begin(), directory.begin() + 3);

    APP_LOG(Log::LogLevel::INFO, "New root directory: %s", self._root_directory.c_str());

    return true;
}

std::string FileManager::clean_path(std::string const& path)
{
    std::string cleaned_path(path);
    size_t pos;
    size_t size;

    std::replace(cleaned_path.begin(), cleaned_path.end(), '/', '\\');

    while ((pos = cleaned_path.find("\\\\")) != std::string::npos)
        cleaned_path.replace(pos, 2, "\\");

    pos = 0;
    while ((pos = cleaned_path.find("\\.", pos)) != std::string::npos)
    {
        if (cleaned_path[pos + 2] == '\\' || (pos + 2) >= cleaned_path.length())
        {
            cleaned_path.replace(pos, 3, "\\");
        }
        else
        {
            ++pos;
        }
    }

    pos = 0;
    while ((pos = cleaned_path.find("\\..", pos)) != std::string::npos )
    {
        if (cleaned_path[pos + 3] == '\\' || (pos+3) >= cleaned_path.length())
        {
            if (pos == 0)
                size = 3;
            else
            {
                size_t parent_pos = cleaned_path.rfind("\\", pos - 1);
                if (parent_pos == std::string::npos)
                {
                    size = pos + 3;
                    pos = 0;
                }
                else
                {
                    size = 3 + pos - parent_pos;
                    pos = parent_pos;
                }
            }

            cleaned_path.replace(pos, size, "");
        }
        else
        {
            ++pos;
        }
    }

    return cleaned_path;
}

bool FileManager::is_dir(std::string const& _path)
{
    std::string path(canonical_path(_path));
    std::wstring wpath;
    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));

    DWORD attrs = GetFileAttributesW(wpath.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && attrs & FILE_ATTRIBUTE_DIRECTORY;
}

bool FileManager::is_file(std::string const& _path)
{
    std::string path(canonical_path(_path));
    std::wstring wpath;
    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));

    DWORD attrs = GetFileAttributesW(wpath.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && ((attrs & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY);
}

bool FileManager::exists(std::string const& _path)
{
    std::string path(canonical_path(_path));
    std::wstring wpath;
    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));

    DWORD attrs = GetFileAttributesW(wpath.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES;
}

bool FileManager::create_directory(std::string const& _directory, bool recursive)
{
    size_t pos = 0;
    struct _stat sb;

    std::string directory(canonical_path(_directory));
    std::wstring sub_dir;
    std::wstring wdirectory;
    
    utf8::utf8to16(directory.begin(), directory.end(), std::back_inserter(wdirectory));
    if (wdirectory.empty())
        return false;

    if (recursive)
    {
        pos = 3;

        do
        {
            pos = wdirectory.find_first_of(L"\\/", pos + 1);
            sub_dir = std::move(wdirectory.substr(0, pos));
            if (_wstat(sub_dir.c_str(), &sb) == 0)
            {
                if (!(sb.st_mode & _S_IFDIR))
                {// A subpath in the target is not a directory
                    return false;
                }
                // Folder exists
            }
            else if (CreateDirectoryW(wdirectory.substr(0, pos).c_str(), NULL) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS)
            {// Failed to create directory
                return false;
            }
        }
        while (pos != std::string::npos);

        return true;
    }
    
    return (CreateDirectoryW(wdirectory.c_str(), NULL) != FALSE || GetLastError() == ERROR_ALREADY_EXISTS);
}

bool FileManager::delete_file(std::string const& _path)
{
    std::string path(canonical_path(_path));
    std::wstring wpath;

    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));

    return DeleteFileW(wpath.c_str()) == TRUE || GetLastError() == ERROR_FILE_NOT_FOUND;
}

static std::vector<std::wstring> list_files(std::wstring const& path, bool recursive)
{
    std::vector<std::wstring> files;
    WIN32_FIND_DATAW hfind_data;
    HANDLE hfind = INVALID_HANDLE_VALUE;

    std::wstring search_path = path;

    if (*path.rbegin() != L'\\')
        search_path += L'\\';

    search_path += L'*';

    // Start iterating over the files in the path directory.
    hfind = FindFirstFileW(search_path.c_str(), &hfind_data);
    if (hfind != INVALID_HANDLE_VALUE)
    {
        search_path.pop_back();
        do // Managed to locate and create an handle to that folder.
        {
            if (wcscmp(L".", hfind_data.cFileName) == 0
                || wcscmp(L"..", hfind_data.cFileName) == 0)
                continue;

            if (hfind_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (recursive)
                {
                    std::wstring dir_name = hfind_data.cFileName;

                    std::vector<std::wstring> sub_files = std::move(list_files(search_path + dir_name, true));
                    std::transform(sub_files.begin(), sub_files.end(), std::back_inserter(files), [&dir_name](std::wstring& file_name)
                        {
                            return dir_name + L'\\' + file_name;
                        });
                }
            }
            else
            {
                files.emplace_back(hfind_data.cFileName);
            }
        } while (FindNextFileW(hfind, &hfind_data) == TRUE);
        FindClose(hfind);
    }

    return files;
}

std::vector<std::string> FileManager::list_files(std::string const& _path, bool recursive)
{
    std::vector<std::string> files;
    std::string path(canonical_path(_path));
    std::wstring wpath;
    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));
    std::vector<std::wstring> wfiles(std::move(::list_files(wpath, recursive)));

    files.reserve(wfiles.size());
    std::transform(wfiles.begin(), wfiles.end(), std::back_inserter(files), [](std::wstring const& wfile_name)
    {
        std::string file_name;
        utf8::utf16to8(wfile_name.begin(), wfile_name.end(), std::back_inserter(file_name));
        return file_name;
    });

    return files;
}

#else

bool FileManager::is_absolute(std::string const& path)
{
    return path[0] == '/';
}

bool FileManager::set_root_dir(std::string const& _directory)
{
    auto& self = Inst();
    std::string directory;

    if (!is_absolute(_directory))
    {
        char buff[4096];
        getcwd(buff, 4096);
        std::string tmp(buff);

        tmp += '/';
        directory = std::move(tmp + _directory);
    }
    else
        directory = _directory;

    self._root_directory = std::move(clean_path(directory));

    if (self._root_directory.empty())
        self._root_directory.assign(directory.begin(), directory.begin() + 1);

    APP_LOG(Log::LogLevel::INFO, "New root directory: %s", self._root_directory.c_str());

    return true;
}

std::string FileManager::clean_path(std::string const& path)
{
    std::string cleaned_path(path);
    size_t pos;
    size_t size;

    std::replace(cleaned_path.begin(), cleaned_path.end(), '\\', '/');

    while ((pos = cleaned_path.find("//")) != std::string::npos)
        cleaned_path.replace(pos, 2, "/");

    pos = 0;
    while ((pos = cleaned_path.find("/.", pos)) != std::string::npos)
    {
        if (cleaned_path[pos + 2] == '/' || (pos + 2) >= cleaned_path.length())
        {
            cleaned_path.replace(pos, 3, "/");
        }
        else
        {
            ++pos;
        }
    }

    pos = 0;
    while ((pos = cleaned_path.find("/..", pos)) != std::string::npos)
    {
        if (cleaned_path[pos + 3] == '/' || (pos + 3) >= cleaned_path.length())
        {
            if (pos == 0)
                size = 3;
            else
            {
                size_t parent_pos = cleaned_path.rfind("/", pos - 1);
                if (parent_pos == std::string::npos)
                {
                    size = pos + 3;
                    pos = 0;
                }
                else
                {
                    size = 3 + pos - parent_pos;
                    pos = parent_pos;
                }
            }

            cleaned_path.replace(pos, size, "");
        }
        else
        {
            ++pos;
        }
    }

    if (cleaned_path.empty())
        cleaned_path = '/';

    return cleaned_path;
}

bool FileManager::is_dir(std::string const& _path)
{
    std::string path(canonical_path(_path));
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0)
    {
        return S_ISDIR(sb.st_mode);
    }

    return false;
}

bool FileManager::is_file(std::string const& _path)
{
    std::string path(canonical_path(_path));
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0)
    {
        return S_ISREG(sb.st_mode);
    }

    return false;
}

bool FileManager::exists(std::string const& _path)
{
    std::string path(canonical_path(_path));
    struct stat sb;
    return stat(path.c_str(), &sb) == 0;
}

bool FileManager::create_directory(std::string const& _directory, bool recursive)
{
    size_t pos = 0;
    struct stat sb;

    std::string sub_dir;
    std::string directory(canonical_path(_directory));

    do
    {
        pos = directory.find_first_of("\\/", pos + 1);
        sub_dir = std::move(directory.substr(0, pos));
        if (stat(sub_dir.c_str(), &sb) == 0)
        {
            if (!S_ISDIR(sb.st_mode))
            {// A subpath in the target is not a directory
                return false;
            }
            // Folder exists
        }
        else if (mkdir(sub_dir.c_str(), 0755) < 0 && errno != EEXIST)
        {// Failed to create directory (no permission?)
            return false;
        }
    }
    while (pos != std::string::npos);

    return true;
}

bool FileManager::delete_file(std::string const& _path)
{
    std::string path(canonical_path(_path));
    return unlink(path.c_str()) == 0;
}

std::vector<std::string> FileManager::list_files(std::string const& path, bool recursive)
{
    std::vector<std::string> files;

    std::string search_path = path;

    if (*path.rbegin() != PATH_SEPARATOR)
        search_path += PATH_SEPARATOR;

    DIR* dir = opendir(search_path.c_str());
    struct dirent* entry;

    if (dir == nullptr)
        return files;

    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".") == 0
            || strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR)
        {
            if (recursive)
            {
                std::string dir_name = entry->d_name;
                std::vector<std::string> sub_files = std::move(list_files(search_path + dir_name, true));
                std::transform(sub_files.begin(), sub_files.end(), std::back_inserter(files), [&dir_name](std::string& file_name)
                    {
                        return dir_name + PATH_SEPARATOR + file_name;
                    });
            }
        }
        else if (entry->d_type == DT_REG)
        {
            files.emplace_back(entry->d_name);
        }
    }

    closedir(dir);

    return files;
}

#endif