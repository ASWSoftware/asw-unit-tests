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
#include <utility>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
#include <Vcl.Styles.hpp> // Links the unit that registers the engine loading VCL style resources, e.g. Carbon.
#include <Vcl.Themes.hpp>
#include <windows.h>
#include <shellapi.h>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_INIDocument.h"
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
/*
    ApplyGUITheme

    TrySetStyle() leaves the style as it is if the dark style's resource isn't linked in (the GUI project's
    Custom_Styles), as in the VCL console runner, or the Vcl.Styles unit isn't, which loads it.
*/
bool ApplyGUITheme(EGUITheme theme, System::UnicodeString const& darkStyle)
{
    bool const dark = GUIThemeIsDark(theme, WindowsAppsUseDarkTheme(), WindowsHighContrast());
    System::UnicodeString const styleName = dark ? darkStyle : System::UnicodeString("Windows");

    if (System::Sysutils::SameText(Vcl::Themes::TStyleManager::ActiveStyle->Name, styleName))
        return false;

    return Vcl::Themes::TStyleManager::TrySetStyle(styleName, false);
}
//---------------------------------------------------------------------------
std::vector<std::string> BuiltInGUIDarkThemeNames()
{
    std::vector<std::string> names = { GUIDefaultDarkThemeName() };
    System::DynamicArray<System::UnicodeString> const styles = Vcl::Themes::TStyleManager::StyleNames;

    for (std::string const& name : GUIVCLStyleDarkThemeNames())
    {
        for (int i = 0; i < styles.Length; ++i)
        {
            if (System::Sysutils::SameText(styles[i], FromUTF8(name)))
            {
                names.push_back(name);
                break;
            }
        }
    }

    return names;
}
//---------------------------------------------------------------------------
std::vector<std::string> FileNamesInFolder(System::UnicodeString const& folderPath)
{
    std::vector<std::string> names;

    try
    {
        if (!System::Sysutils::DirectoryExists(folderPath))
            return names;

        System::DynamicArray<System::UnicodeString> const files = System::Ioutils::TDirectory::GetFiles(folderPath);

        for (int i = 0; i < files.Length; ++i)
            names.push_back(ToUTF8(System::Sysutils::ExtractFileName(files[i])));
    }
    catch (...)
    {
        names.clear();
    }

    return names;
}
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
System::UnicodeString GUIThemesFolderPath()
{
    // GetHomePath() is %APPDATA% on Windows, as for GUILayoutFilePath().
    return System::Ioutils::TPath::Combine(System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetHomePath(),
        "ASWUnitTests"), "Themes");
}
//---------------------------------------------------------------------------
/*
    LoadGUIDarkStyle

    A style, once loaded, stays registered for the process, under the name the file gives it, and can't be
    loaded again, so a file loaded before gives the style it loaded, and a change to it shows after a restart.
    A file whose style has the name of one already registered, e.g. Carbon's own file renamed, can't be loaded
    either. Every error gives Carbon, with a line in 'problems'. A built-in style that isn't linked in is looked
    for as a file.
*/
System::UnicodeString LoadGUIDarkStyle(System::UnicodeString const& themesFolder, std::string const& darkTheme,
    std::vector<std::string>& problems)
{
    // Each file loaded so far, by its path in lowercase, and the name of its style.
    static std::vector<std::pair<System::UnicodeString, System::UnicodeString> > loaded;

    System::UnicodeString const carbon = "Carbon";

    for (std::string const& name : BuiltInGUIDarkThemeNames())
    {
        if (System::Sysutils::SameText(FromUTF8(name), FromUTF8(darkTheme)))
            return FromUTF8(name);
    }

    std::string problem;

    try
    {
        System::UnicodeString const path = System::Ioutils::TPath::Combine(themesFolder, FromUTF8(darkTheme) + ".vsf");
        System::UnicodeString const key = System::Sysutils::AnsiLowerCase(path);

        for (std::pair<System::UnicodeString, System::UnicodeString> const& entry : loaded)
        {
            if (entry.first == key)
                return entry.second;
        }

        Vcl::Themes::TStyleInfo info;

        if (!System::Sysutils::FileExists(path))
        {
            problem = "There's no style file " + ToUTF8(path) + ", so " + GUIDefaultDarkThemeName() +
                " is used instead.";
        }
        else if (!Vcl::Themes::TStyleManager::IsValidStyle(path, info))
        {
            problem = "Its file isn't a VCL style file, so " + GUIDefaultDarkThemeName() + " is used instead.";
        }
        else
        {
            System::DynamicArray<System::UnicodeString> const names = Vcl::Themes::TStyleManager::StyleNames;

            for (int i = 0; i < names.Length && problem.empty(); ++i)
            {
                if (System::Sysutils::SameText(names[i], info.Name))
                {
                    problem = "Its style is named " + ToUTF8(info.Name) + ", as a style already loaded is, so " +
                        GUIDefaultDarkThemeName() + " is used instead. Give it a name of its own in RAD Studio's " +
                        "Bitmap Style Designer.";
                }
            }

            if (problem.empty())
            {
                Vcl::Themes::TStyleManager::LoadFromFile(path);
                loaded.emplace_back(key, info.Name);
                return info.Name;
            }
        }
    }
    catch (System::Sysutils::Exception const& exception)
    {
        problem = "Its style file couldn't be loaded, so " + GUIDefaultDarkThemeName() + " is used instead: " +
            ToUTF8(exception.Message);
    }

    problems.push_back(GUIThemeProblemLine(darkTheme, problem));
    return carbon;
}
//---------------------------------------------------------------------------
std::string LoadGUIDarkThemeFromLayoutFile(System::UnicodeString const& layoutPath)
{
    try
    {
        if (!System::Sysutils::FileExists(layoutPath))
            return GUIDefaultDarkThemeName();

        return LoadGUIDarkTheme(TGUIINIDocument(ReadFileBytes(layoutPath)));
    }
    catch (...)
    {
        return GUIDefaultDarkThemeName();
    }
}
//---------------------------------------------------------------------------
EGUITheme LoadGUIThemeFromLayoutFile(System::UnicodeString const& layoutPath)
{
    try
    {
        if (!System::Sysutils::FileExists(layoutPath))
            return EGUITheme::Windows;

        return LoadGUITheme(TGUIINIDocument(ReadFileBytes(layoutPath)));
    }
    catch (...)
    {
        return EGUITheme::Windows;
    }
}
//---------------------------------------------------------------------------
/*
    OpenFolderInExplorer

    ShellExecuteW() returns a value above 32 on success.
*/
void OpenFolderInExplorer(System::UnicodeString const& folderPath)
{
    HINSTANCE const result = ::ShellExecuteW(nullptr, L"open", folderPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

    if (reinterpret_cast<INT_PTR>(result) <= 32)
        throw System::Sysutils::Exception(System::UnicodeString("Couldn't open the folder ") + folderPath);
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
void SaveGUIDarkThemeToLayoutFile(System::UnicodeString const& layoutPath, std::string const& name)
{
    try
    {
        System::Sysutils::ForceDirectories(System::Sysutils::ExtractFileDir(layoutPath));

        // Starting from the file's current text keeps the layout and any other values in it.
        TGUIINIDocument ini(System::Sysutils::FileExists(layoutPath) ? ReadFileBytes(layoutPath) : std::string());
        SaveGUIDarkTheme(ini, name);
        WriteFileBytes(layoutPath, ini.Text());
    }
    catch (...)
    {
        // Not saving it (e.g. a read-only profile) only means the next start uses the default dark theme.
    }
}
//---------------------------------------------------------------------------
void SaveGUIThemeToLayoutFile(System::UnicodeString const& layoutPath, EGUITheme theme)
{
    try
    {
        System::Sysutils::ForceDirectories(System::Sysutils::ExtractFileDir(layoutPath));

        // Starting from the file's current text keeps the layout and any other values in it.
        TGUIINIDocument ini(System::Sysutils::FileExists(layoutPath) ? ReadFileBytes(layoutPath) : std::string());
        SaveGUITheme(ini, theme);
        WriteFileBytes(layoutPath, ini.Text());
    }
    catch (...)
    {
        // Not saving it (e.g. a read-only profile) only means the next start uses the default theme.
    }
}
//---------------------------------------------------------------------------
bool WindowsAppsUseDarkTheme()
{
    DWORD value = 1;
    DWORD size = sizeof(value);

    LSTATUS const status = ::RegGetValueW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme",
        RRF_RT_REG_DWORD, nullptr, &value, &size);

    return status == ERROR_SUCCESS && value == 0;
}
//---------------------------------------------------------------------------
bool WindowsHighContrast()
{
    HIGHCONTRASTW highContrast{};
    highContrast.cbSize = sizeof(highContrast);

    return ::SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, 0) &&
        (highContrast.dwFlags & HCF_HIGHCONTRASTON) != 0;
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
