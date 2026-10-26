/* **************************************************************************
ASWUnitTests_GUI_VCLSystem.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "ASWUnitTests_GUI_VCLSystem.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <memory>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
System::UnicodeString GUILayoutFilePath()
{
    // GetHomePath() is %APPDATA% on Windows.
    System::UnicodeString const folder = System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetHomePath(),
        "ASWUnitTests");
    System::UnicodeString const fileName = System::Sysutils::ChangeFileExt(
        System::Sysutils::ExtractFileName(System::ParamStr(0)), ".ini");

    return System::Ioutils::TPath::Combine(folder, fileName);
}
//---------------------------------------------------------------------------
System::UnicodeString GUISelectionFilePath()
{
    return System::Sysutils::ChangeFileExt(GUILayoutFilePath(), ".selection");
}
//---------------------------------------------------------------------------
std::vector<std::string> ProcessArguments()
{
    std::vector<std::string> args;

    for (int i = 1; i <= System::ParamCount(); ++i)
        args.push_back(ToUTF8(System::ParamStr(i)));

    return args;
}
//---------------------------------------------------------------------------
std::string ReadFileBytes(System::UnicodeString const& path)
{
    System::Sysutils::TBytes const bytes = System::Ioutils::TFile::ReadAllBytes(path);

    if (bytes.Length == 0)
        return std::string();

    return std::string(reinterpret_cast<char const*>(&bytes[0]), static_cast<std::size_t>(bytes.Length));
}
//---------------------------------------------------------------------------
void WriteFileBytes(System::UnicodeString const& path, std::string const& bytes)
{
    std::unique_ptr<System::Classes::TFileStream> const file(new System::Classes::TFileStream(path, fmCreate));

    if (!bytes.empty())
        file->WriteBuffer(bytes.data(), static_cast<NativeInt>(bytes.size()));
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
