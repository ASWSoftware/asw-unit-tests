/* **************************************************************************
ASWUnitTests_GUI_Strings.h
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
#ifndef ASWUnitTests_GUI_StringsH
#define ASWUnitTests_GUI_StringsH
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// The framework's text is UTF-8 std::string; the VCL's is UTF-16 System::UnicodeString. These convert
// between the two without losing non-ASCII characters.

// Converts UTF-8 text to a UTF-16 VCL string.
System::UnicodeString FromUTF8(std::string const& text);
// Converts a UTF-16 VCL string to UTF-8.
std::string ToUTF8(System::UnicodeString const& text);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_StringsH
