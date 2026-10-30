/* **************************************************************************
ASWUnitTests_GUI_INIDocument.cpp
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
#include "ASWUnitTests_GUI_INIDocument.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

bool SameName(std::string const& a, std::string const& b);
char ToLowerASCII(char c);
std::string Trim(std::string const& text);

//---------------------------------------------------------------------------

// Whether two section or key names are the same, ignoring ASCII case, as TMemIniFile matches them.
bool SameName(std::string const& a, std::string const& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y)
        {
            return ToLowerASCII(x) == ToLowerASCII(y);
        });
}
//---------------------------------------------------------------------------
char ToLowerASCII(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}
//---------------------------------------------------------------------------
// 'text' without the spaces, tabs and other control characters at either end, as Delphi's Trim() removes.
std::string Trim(std::string const& text)
{
    auto const isTrimmed = [](char c)
        {
            return static_cast<unsigned char>(c) <= ' ';
        };

    size_t start = 0;
    while (start < text.size() && isTrimmed(text[start]))
        ++start;

    size_t end = text.size();
    while (end > start && isTrimmed(text[end - 1]))
        --end;

    return text.substr(start, end - start);
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIINIDocument
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TGUIINIDocument::TGUIINIDocument()
{
}
//---------------------------------------------------------------------------
TGUIINIDocument::TGUIINIDocument(std::string const& text)
{
    // Past a UTF-8 byte order mark, in case the file was edited with something that adds one.
    size_t start = (text.compare(0, 3, "\xEF\xBB\xBF") == 0) ? 3 : 0;

    // The section the lines being read belong to; nullptr before the first one, and in a section that
    // appeared before, whose lines are skipped.
    TSection* section = nullptr;

    while (start < text.size())
    {
        size_t end = text.find('\n', start);
        if (end == std::string::npos)
            end = text.size();

        std::string const line = Trim(text.substr(start, end - start));
        start = end + 1;

        if (line.empty() || line[0] == ';')
            continue;

        if (line[0] == '[' && line.back() == ']')
        {
            std::string const name = Trim(line.substr(1, line.size() - 2));

            if (FindSection(name) != nullptr)
            {
                section = nullptr;
            }
            else
            {
                m_Sections.push_back(TSection{ std::vector<TKey>(), name });
                section = &m_Sections.back();
            }

            continue;
        }

        size_t const equals = line.find('=');
        if (section == nullptr || equals == std::string::npos)
            continue;

        section->Keys.push_back(TKey{ Trim(line.substr(0, equals)), Trim(line.substr(equals + 1)) });
    }
}
//---------------------------------------------------------------------------
void TGUIINIDocument::DeleteKey(std::string const& section, std::string const& key)
{
    TSection* const found = FindSection(section);
    if (found == nullptr)
        return;

    auto const keyFound = std::find_if(found->Keys.begin(), found->Keys.end(), [&key](TKey const& candidate)
        {
            return SameName(candidate.Name, key);
        });

    if (keyFound != found->Keys.end())
        found->Keys.erase(keyFound);
}
//---------------------------------------------------------------------------
TGUIINIDocument::TSection const* TGUIINIDocument::FindSection(std::string const& section) const
{
    for (TSection const& candidate : m_Sections)
    {
        if (SameName(candidate.Name, section))
            return &candidate;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
TGUIINIDocument::TSection* TGUIINIDocument::FindSection(std::string const& section)
{
    return const_cast<TSection*>(static_cast<TGUIINIDocument const*>(this)->FindSection(section));
}
//---------------------------------------------------------------------------
std::vector<std::string> TGUIINIDocument::KeyNames(std::string const& section) const
{
    std::vector<std::string> names;
    TSection const* const found = FindSection(section);

    if (found != nullptr)
    {
        for (TKey const& key : found->Keys)
            names.push_back(key.Name);
    }

    return names;
}
//---------------------------------------------------------------------------
std::optional<std::string> TGUIINIDocument::ReadString(std::string const& section, std::string const& key) const
{
    TSection const* const found = FindSection(section);
    if (found == nullptr)
        return std::nullopt;

    for (TKey const& candidate : found->Keys)
    {
        if (SameName(candidate.Name, key))
            return candidate.Value;
    }

    return std::nullopt;
}
//---------------------------------------------------------------------------
std::vector<std::string> TGUIINIDocument::SectionNames() const
{
    std::vector<std::string> names;

    for (TSection const& section : m_Sections)
        names.push_back(section.Name);

    return names;
}
//---------------------------------------------------------------------------
std::string TGUIINIDocument::Text() const
{
    std::string text;

    for (TSection const& section : m_Sections)
    {
        text += "[" + section.Name + "]\r\n";

        for (TKey const& key : section.Keys)
            text += key.Name + "=" + key.Value + "\r\n";

        text += "\r\n";
    }

    return text;
}
//---------------------------------------------------------------------------
void TGUIINIDocument::WriteString(std::string const& section, std::string const& key, std::string const& value)
{
    TSection* found = FindSection(section);
    if (found == nullptr)
    {
        m_Sections.push_back(TSection{ std::vector<TKey>(), section });
        found = &m_Sections.back();
    }

    for (TKey& candidate : found->Keys)
    {
        if (SameName(candidate.Name, key))
        {
            candidate.Value = value;
            return;
        }
    }

    found->Keys.push_back(TKey{ key, value });
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
