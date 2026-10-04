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
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// Converts wide text to UTF-8, reading it as UTF-16 where wchar_t is 16 bits (Windows) and as UTF-32 where it's
// 32 bits (Linux). Anything that isn't a valid code point, such as an unpaired surrogate, becomes U+FFFD, the
// Unicode replacement character.
std::string WideToUTF8(std::wstring const& text);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_UtilsH
