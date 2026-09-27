/* **************************************************************************
ASWUnitTests_GUI_Strings.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 Anthony S. West

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
// Module header
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------
#include <cstddef>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
System::UnicodeString FromUTF8(std::string const& text)
{
    return System::UnicodeString(System::UTF8String(text.c_str(), static_cast<int>(text.size())));
}
//---------------------------------------------------------------------------
std::string ToUTF8(System::UnicodeString const& text)
{
    System::UTF8String const utf8(text);
    return std::string(utf8.c_str(), static_cast<std::size_t>(utf8.Length()));
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
