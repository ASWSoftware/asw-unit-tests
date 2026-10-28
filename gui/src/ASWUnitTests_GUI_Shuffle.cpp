/* **************************************************************************
ASWUnitTests_GUI_Shuffle.cpp
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
#include "ASWUnitTests_GUI_Shuffle.h"
//---------------------------------------------------------------------------
#include <random>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIShuffle
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TGUIShuffle::TGUIShuffle()
    : TGUIShuffle([]()
    {
        return std::random_device{}();
    })
{
}
//---------------------------------------------------------------------------
TGUIShuffle::TGUIShuffle(TSeedSource const& newSeed)
    : m_Enabled(false),
      m_NewSeed(newSeed)
{
}
//---------------------------------------------------------------------------
void TGUIShuffle::ApplyCommandLine(bool shuffle, std::optional<unsigned int> seed)
{
    m_Enabled = shuffle || seed.has_value();
    m_FixedSeed = seed;
}
//---------------------------------------------------------------------------
bool TGUIShuffle::ApplySeedChoice(bool newSeedEachRun, std::string const& seedText, std::string& error)
{
    error.clear();
    std::optional<unsigned int> seed;

    if (!newSeedEachRun)
    {
        std::string::size_type const first = seedText.find_first_not_of(" \t");
        std::string::size_type const last = seedText.find_last_not_of(" \t");
        if (first != std::string::npos)
            seed = TCLIParser::ParseUnsignedInt(seedText.substr(first, last - first + 1));

        if (!seed.has_value())
        {
            error = "The seed must be a whole number from 0 to 4294967295.";
            return false;
        }
    }

    m_Enabled = true;
    m_FixedSeed = seed;

    return true;
}
//---------------------------------------------------------------------------
bool TGUIShuffle::Enabled() const
{
    return m_Enabled;
}
//---------------------------------------------------------------------------
std::optional<unsigned int> TGUIShuffle::FixedSeed() const
{
    return m_FixedSeed;
}
//---------------------------------------------------------------------------
std::optional<unsigned int> TGUIShuffle::LastSeed() const
{
    return m_LastSeed;
}
//---------------------------------------------------------------------------
std::optional<unsigned int> TGUIShuffle::NextRunSeed()
{
    if (!m_Enabled)
        return std::nullopt;

    m_LastSeed = m_FixedSeed.has_value() ? *m_FixedSeed : m_NewSeed();

    return m_LastSeed;
}
//---------------------------------------------------------------------------
unsigned int TGUIShuffle::SeedToShow() const
{
    if (m_FixedSeed.has_value())
        return *m_FixedSeed;

    if (m_LastSeed.has_value())
        return *m_LastSeed;

    return m_NewSeed();
}
//---------------------------------------------------------------------------
void TGUIShuffle::SetEnabled(bool enabled)
{
    m_Enabled = enabled;
}
//---------------------------------------------------------------------------
std::string TGUIShuffle::StatusText() const
{
    if (!m_Enabled)
        return "Shuffle: off";

    if (m_FixedSeed.has_value())
        return "Shuffle: seed " + std::to_string(*m_FixedSeed);

    if (m_LastSeed.has_value())
        return "Shuffle: random (last " + std::to_string(*m_LastSeed) + ")";

    return "Shuffle: random";
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
