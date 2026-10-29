/* **************************************************************************
ASWUnitTests_Utils.h
Author: Anthony S. West - ASW Software

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
#ifndef ASWUnitTests_UtilsH
#define ASWUnitTests_UtilsH
//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

#if defined(_WIN32)
// Returns the arguments in a Windows command line such as GetCommandLineW()'s, without the program name, converted
// to UTF-8. Split by CommandLineToArgvW(), whose quoting rules are those of the C runtime's argv. Throws
// std::runtime_error if the command line can't be split.
std::vector<std::string> CommandLineArguments(std::wstring const& commandLine);
#endif

// Returns the 32-bit FNV-1a hash of 'text''s bytes. Unlike std::hash, the result is the same with every compiler
// and standard library.
uint32_t FNV1aHash(std::string const& text);

// Returns the indices 0 to 'count' - 1 in a random order determined entirely by 'seed'. Unlike std::shuffle, a
// given seed gives the same order with every compiler and standard library. 'count' must fit in 32 bits.
std::vector<std::size_t> ShuffledIndices(std::size_t count, unsigned int seed);

#if defined(_WIN32)
// Returns Windows' description of a GetLastError() code, e.g. "Access is denied.", or "error <code>" when it has
// none.
std::string SystemErrorText(unsigned long error);
#endif

// Converts UTF-8 text to wide text, UTF-16 where wchar_t is 16 bits (Windows) and UTF-32 where it's 32 bits
// (Linux). Each maximal ill-formed sequence (Unicode's term: a byte that can't start a sequence, or as much of a
// sequence as is valid before it breaks off) becomes one U+FFFD, the Unicode replacement character, as Unicode
// recommends. WideToUTF8() converts back.
std::wstring UTF8ToWide(std::string const& text);

// Converts wide text to UTF-8, reading it as UTF-16 where wchar_t is 16 bits (Windows) and as UTF-32 where it's
// 32 bits (Linux). Anything that isn't a valid code point, such as an unpaired surrogate, becomes U+FFFD, the
// Unicode replacement character.
std::string WideToUTF8(std::wstring const& text);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_UtilsH
