/* **************************************************************************
ASWUnitTests_GUI_INIDocument.h
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
#ifndef ASWUnitTests_GUI_INIDocumentH
#define ASWUnitTests_GUI_INIDocumentH
//---------------------------------------------------------------------------
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIINIDocument
//
// The text of an INI file, such as a GUI runner's layout file, read and
// written the way the VCL's TMemIniFile does, so files saved by either one
// read the same with the other:
// - Section and key names are matched ignoring (ASCII) case.
// - Each line is trimmed; a key is the text before its line's first '=',
//   and its value the text after it, each trimmed.
// - Blank lines, comment lines (starting with ';'), lines before the first
//   section, and lines in a section with no '=' are skipped. So is a
//   section that appears again after its first occurrence, with its keys.
// - A key that appears twice in a section reads as its first value.
// - Text() writes each section as "[Name]", its keys as "Name=Value", then a
//   blank line, with CRLF line breaks and no byte order mark. Sections and
//   keys keep the order they were read or first written in, so unknown ones
//   are kept, but skipped lines aren't.
// Contains no UI code, so it's unit tested from the console runners (see
// gui/tests/Test_ASWUnitTests_GUI_INIDocument.cpp).
/////////////////////////////////////////////////////////////////////////////
class TGUIINIDocument
{
private:
    struct TKey
    {
        std::string Name;
        std::string Value;
    };

    struct TSection
    {
        std::vector<TKey> Keys;
        std::string Name;
    };

private:
    std::vector<TSection> m_Sections;

private:
    TSection const* FindSection(std::string const& section) const;
    TSection* FindSection(std::string const& section);

public:
    // An empty document.
    TGUIINIDocument();
    // Reads 'text', the file's content in UTF-8, past a UTF-8 byte order mark if it has one. Either CRLF or LF
    // line breaks.
    explicit TGUIINIDocument(std::string const& text);

    // Removes the key, if there is one. Its section stays, even if it's then empty.
    void DeleteKey(std::string const& section, std::string const& key);
    // The names of the section's keys, as written and in order, a repeated key's each time, as TMemIniFile's
    // ReadSection() gives them; empty if the section doesn't exist.
    std::vector<std::string> KeyNames(std::string const& section) const;
    // The key's value, or empty if the section or key doesn't exist.
    std::optional<std::string> ReadString(std::string const& section, std::string const& key) const;
    // The names of the sections, as written and in order.
    std::vector<std::string> SectionNames() const;
    // The document's text, as described above; empty for an empty document.
    std::string Text() const;
    // Sets the key's value, adding the key at the end of its section, and the section at the end of the
    // document, if they don't exist yet.
    void WriteString(std::string const& section, std::string const& key, std::string const& value);
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_INIDocumentH
