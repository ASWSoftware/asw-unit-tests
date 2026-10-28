/* **************************************************************************
ASWUnitTests_Win32GUI_System.cpp
Author: Anthony S. West - ASW Software

What the Win32 GUI runner needs from its process and system: text
conversion between the framework's UTF-8 and Windows' UTF-16, its
command-line arguments, where it keeps its settings, and file access
through wide paths.

Copyright 2026 ASW Software

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************** */

//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
// Module header
#include "ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// The most bytes one ReadFile() or WriteFile() call is asked to transfer.
DWORD const MaxFileChunk = 1u << 30;

// A buffer from CommandLineToArgvW() or FormatMessageW(), freed with LocalFree().
template <typename T>
using TLocalPtr = std::unique_ptr<T, HLOCAL(WINAPI*)(HLOCAL)>;

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFileHandle
//
// Closes a file handle when it goes out of scope.
/////////////////////////////////////////////////////////////////////////////
class TFileHandle
{
private:
    HANDLE const m_Handle;

public:
    explicit TFileHandle(HANDLE handle);
    ~TFileHandle();

    TFileHandle(TFileHandle const&) = delete;
    TFileHandle& operator=(TFileHandle const&) = delete;

    HANDLE Get() const;
    bool IsValid() const;
};

//---------------------------------------------------------------------------

std::runtime_error FileError(std::string const& action, std::string const& path, DWORD error);
std::string SystemErrorText(DWORD error);
int ToConversionLength(std::size_t length);
std::string WithExtension(std::string const& path, std::string const& extension);

//---------------------------------------------------------------------------
/*
    FileError

    The error to throw when a file operation fails, e.g. "Couldn't read C:\x.ini: Access is denied."
*/
std::runtime_error FileError(std::string const& action, std::string const& path, DWORD error)
{
    return std::runtime_error("Couldn't " + action + " " + path + ": " + SystemErrorText(error));
}
//---------------------------------------------------------------------------
/*
    SystemErrorText

    Windows' description of a GetLastError() code, e.g. "Access is denied.".
*/
std::string SystemErrorText(DWORD error)
{
    wchar_t* buffer = nullptr;
    DWORD const length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, 0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

    if (length == 0 || buffer == nullptr)
        return "error " + std::to_string(error);

    TLocalPtr<wchar_t> const owner(buffer, &LocalFree);
    std::wstring text(buffer, length);

    // FormatMessageW() ends the text with a line break.
    while (!text.empty() && (text.back() == L'\r' || text.back() == L'\n' || text.back() == L' '))
        text.pop_back();

    return WideToUTF8(text);
}
//---------------------------------------------------------------------------
/*
    ToConversionLength

    A length for MultiByteToWideChar() or WideCharToMultiByte(), which take an int.
*/
int ToConversionLength(std::size_t length)
{
    if (length > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
        throw std::length_error("text too long to convert");

    return static_cast<int>(length);
}
//---------------------------------------------------------------------------
/*
    WithExtension

    The path with its file name's extension replaced, or added if it has none, e.g. "a\b.exe" and ".ini" give
    "a\b.ini".
*/
std::string WithExtension(std::string const& path, std::string const& extension)
{
    std::size_t const separator = path.find_last_of("\\/:");
    std::size_t const dot = path.find_last_of('.');

    if (dot == std::string::npos || (separator != std::string::npos && dot < separator))
        return path + extension;

    return path.substr(0, dot) + extension;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFileHandle
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFileHandle::TFileHandle(HANDLE handle)
    : m_Handle(handle)
{
}
//---------------------------------------------------------------------------
TFileHandle::~TFileHandle()
{
    if (IsValid())
        CloseHandle(m_Handle);
}
//---------------------------------------------------------------------------
HANDLE TFileHandle::Get() const
{
    return m_Handle;
}
//---------------------------------------------------------------------------
bool TFileHandle::IsValid() const
{
    return m_Handle != INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
std::string AdjustLineBreaks(std::string const& text)
{
    std::string adjusted;
    adjusted.reserve(text.size() + text.size() / 16);

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '\r' || text[i] == '\n')
        {
            adjusted += "\r\n";

            // A CRLF pair is one line break.
            if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n')
                ++i;
        }
        else
        {
            adjusted += text[i];
        }
    }

    return adjusted;
}
//---------------------------------------------------------------------------
std::vector<std::string> CommandLineArguments(std::wstring const& commandLine)
{
    int count = 0;
    TLocalPtr<wchar_t*> const argv(CommandLineToArgvW(commandLine.c_str(), &count), &LocalFree);

    if (argv == nullptr)
        throw std::runtime_error("Couldn't split the command line: " + SystemErrorText(GetLastError()));

    std::vector<std::string> args;

    for (int i = 1; i < count; ++i)
        args.push_back(WideToUTF8(argv.get()[i]));

    return args;
}
//---------------------------------------------------------------------------
void CreateFolderPath(std::string const& folderPath)
{
    std::wstring path = UTF8ToWide(folderPath);
    std::replace(path.begin(), path.end(), L'/', L'\\');

    // Each folder from the top down. Creating a drive or a share ("C:", "\\server") fails, as does creating a
    // folder that exists, so only whether the whole path ends up a folder decides success.
    DWORD lastError = ERROR_SUCCESS;

    for (std::size_t end = path.find(L'\\'); ; end = path.find(L'\\', end + 1))
    {
        std::wstring const folder = path.substr(0, end);

        if (!folder.empty() && CreateDirectoryW(folder.c_str(), nullptr) == 0)
            lastError = GetLastError();

        if (end == std::wstring::npos)
            break;
    }

    DWORD const attributes = GetFileAttributesW(path.c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        throw FileError("create the folder", folderPath, lastError != ERROR_SUCCESS ? lastError : ERROR_PATH_NOT_FOUND);
}
//---------------------------------------------------------------------------
std::string ExeFileName()
{
    std::wstring path(MAX_PATH, L'\0');

    for (;;)
    {
        DWORD const length = GetModuleFileNameW(nullptr, &path[0], static_cast<DWORD>(path.size()));

        if (length == 0)
            throw std::runtime_error("Couldn't get the executable's path: " + SystemErrorText(GetLastError()));

        // A truncated path fills the whole buffer.
        if (length < path.size())
        {
            path.resize(length);
            break;
        }

        path.resize(path.size() * 2);
    }

    std::size_t const separator = path.find_last_of(L"\\/");

    return WideToUTF8(separator == std::wstring::npos ? path : path.substr(separator + 1));
}
//---------------------------------------------------------------------------
std::string ExpandFileName(std::string const& path)
{
    std::wstring const relative = UTF8ToWide(path);
    // The length GetFullPathNameW() needs, including the terminating null; 0 if the path isn't valid.
    DWORD const needed = GetFullPathNameW(relative.c_str(), 0, nullptr, nullptr);

    if (needed == 0)
        throw FileError("expand the path", path, GetLastError());

    std::wstring full(needed, L'\0');
    DWORD const length = GetFullPathNameW(relative.c_str(), needed, &full[0], nullptr);

    if (length == 0 || length >= needed)
        throw FileError("expand the path", path, length == 0 ? GetLastError() : ERROR_INSUFFICIENT_BUFFER);

    full.resize(length);
    return WideToUTF8(full);
}
//---------------------------------------------------------------------------
bool FileExists(std::string const& path)
{
    DWORD const attributes = GetFileAttributesW(UTF8ToWide(path).c_str());

    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
//---------------------------------------------------------------------------
std::string GUILayoutFilePath()
{
    return GUISettingsFolderPath() + "\\" + WithExtension(ExeFileName(), ".ini");
}
//---------------------------------------------------------------------------
std::string GUISelectionFilePath()
{
    return WithExtension(GUILayoutFilePath(), ".selection");
}
//---------------------------------------------------------------------------
std::string GUISettingsFolderPath()
{
    // The same folder the VCL GUI runner's System::Ioutils::TPath::GetHomePath() gives.
    wchar_t appData[MAX_PATH] = {};
    HRESULT const result = SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, appData);

    if (FAILED(result))
        throw std::runtime_error("Couldn't find the AppData folder: " + SystemErrorText(static_cast<DWORD>(result)));

    return WideToUTF8(appData) + "\\ASWUnitTests";
}
//---------------------------------------------------------------------------
std::vector<std::string> ProcessArguments()
{
    return CommandLineArguments(GetCommandLineW());
}
//---------------------------------------------------------------------------
std::string ReadFileBytes(std::string const& path)
{
    TFileHandle const file(CreateFileW(UTF8ToWide(path).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));

    if (!file.IsValid())
        throw FileError("open", path, GetLastError());

    LARGE_INTEGER size;

    if (GetFileSizeEx(file.Get(), &size) == 0)
        throw FileError("read", path, GetLastError());

    if (static_cast<unsigned long long>(size.QuadPart) > (std::numeric_limits<std::size_t>::max)() / 2)
        throw std::runtime_error("Couldn't read " + path + ": the file is too large");

    std::string bytes(static_cast<std::size_t>(size.QuadPart), '\0');
    std::size_t total = 0;

    while (total < bytes.size())
    {
        DWORD const chunk = static_cast<DWORD>((std::min)(bytes.size() - total, static_cast<std::size_t>(MaxFileChunk)));
        DWORD read = 0;

        if (ReadFile(file.Get(), &bytes[total], chunk, &read, nullptr) == 0)
            throw FileError("read", path, GetLastError());

        // The file got shorter since its size was read.
        if (read == 0)
            break;

        total += read;
    }

    bytes.resize(total);
    return bytes;
}
//---------------------------------------------------------------------------
bool RemoveFile(std::string const& path)
{
    return DeleteFileW(UTF8ToWide(path).c_str()) != 0;
}
//---------------------------------------------------------------------------
/*
    SetClipboardText

    Puts the text on the clipboard as CF_UNICODETEXT, which Windows also offers to programs that ask for
    CF_TEXT. Once SetClipboardData() succeeds, the clipboard owns the memory.
*/
void SetClipboardText(HWND owner, std::string const& text)
{
    std::wstring const wide = UTF8ToWide(text);
    HGLOBAL const memory = GlobalAlloc(GMEM_MOVEABLE, (wide.size() + 1) * sizeof(wchar_t));

    if (memory == nullptr)
        throw std::runtime_error("Couldn't copy to the clipboard: " + SystemErrorText(GetLastError()));

    wchar_t* const target = static_cast<wchar_t*>(GlobalLock(memory));

    if (target == nullptr)
    {
        DWORD const error = GetLastError();
        GlobalFree(memory);
        throw std::runtime_error("Couldn't copy to the clipboard: " + SystemErrorText(error));
    }

    std::copy(wide.c_str(), wide.c_str() + wide.size() + 1, target);
    GlobalUnlock(memory);

    if (OpenClipboard(owner) == 0)
    {
        DWORD const error = GetLastError();
        GlobalFree(memory);
        throw std::runtime_error("Couldn't open the clipboard: " + SystemErrorText(error));
    }

    EmptyClipboard();
    bool const placed = SetClipboardData(CF_UNICODETEXT, memory) != nullptr;
    DWORD const error = GetLastError();
    CloseClipboard();

    if (!placed)
    {
        GlobalFree(memory);
        throw std::runtime_error("Couldn't copy to the clipboard: " + SystemErrorText(error));
    }
}
//---------------------------------------------------------------------------
std::wstring UTF8ToWide(std::string const& text)
{
    if (text.empty())
        return std::wstring();

    int const length = ToConversionLength(text.size());
    int const wideLength = MultiByteToWideChar(CP_UTF8, 0, text.data(), length, nullptr, 0);

    if (wideLength <= 0)
        throw std::runtime_error("Couldn't convert UTF-8 text: " + SystemErrorText(GetLastError()));

    std::wstring wide(static_cast<std::size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), length, &wide[0], wideLength);
    return wide;
}
//---------------------------------------------------------------------------
std::string WindowText(HWND window)
{
    std::wstring text(static_cast<std::size_t>(GetWindowTextLengthW(window)) + 1, L'\0');
    text.resize(static_cast<std::size_t>(GetWindowTextW(window, &text[0], static_cast<int>(text.size()))));

    return WideToUTF8(text);
}
//---------------------------------------------------------------------------
void WriteFileBytes(std::string const& path, std::string const& bytes)
{
    TFileHandle const file(CreateFileW(UTF8ToWide(path).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr));

    if (!file.IsValid())
        throw FileError("create", path, GetLastError());

    std::size_t total = 0;

    while (total < bytes.size())
    {
        DWORD const chunk = static_cast<DWORD>((std::min)(bytes.size() - total, static_cast<std::size_t>(MaxFileChunk)));
        DWORD written = 0;

        if (WriteFile(file.Get(), bytes.data() + total, chunk, &written, nullptr) == 0)
            throw FileError("write", path, GetLastError());

        total += written;
    }
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
