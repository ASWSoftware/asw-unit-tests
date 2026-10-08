/* **************************************************************************
ASWUnitTests_GUI_Shuffle.h
Author: Anthony S. West - ASW Software

Whether the VCL GUI runner shuffles its runs, and with which seed.

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
#ifndef ASWUnitTests_GUI_ShuffleH
#define ASWUnitTests_GUI_ShuffleH
//---------------------------------------------------------------------------
#include <functional>
#include <optional>
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIShuffle
//
// The state behind the GUI's Options > Run in Shuffled Order and Shuffle
// Seed..., which starts out from --shuffle and --shuffle-seed. With a fixed
// seed, every run uses it, so each repeats the same order. Without one, each
// run gets a new random seed, and the last one used is kept, so Shuffle
// Seed... can fix it to repeat that run's order.
// Contains no VCL code, so it's unit tested from the VCL console runner
// (see vcl/tests/Test_ASWUnitTests_GUI_Shuffle.cpp).
/////////////////////////////////////////////////////////////////////////////
class TGUIShuffle
{
public:
    // Returns a new random seed.
    typedef std::function<unsigned int ()> TSeedSource;

private:
    bool m_Enabled;
    std::optional<unsigned int> m_FixedSeed;
    std::optional<unsigned int> m_LastSeed;
    TSeedSource m_NewSeed;

public:
    // Starts with shuffling off, taking new seeds from std::random_device.
    TGUIShuffle();
    // Starts with shuffling off, taking new seeds from 'newSeed', e.g. a fixed sequence for a test.
    explicit TGUIShuffle(TSeedSource const& newSeed);

    // Applies --shuffle ('shuffle') and --shuffle-seed ('seed'), which, as on the command line, also turns
    // shuffling on.
    void ApplyCommandLine(bool shuffle, std::optional<unsigned int> seed);
    // Applies Shuffle Seed...'s choice and turns shuffling on: a new random seed for each run if
    // 'newSeedEachRun', otherwise 'seedText' (spaces around it allowed) for every run. Returns false, changing
    // nothing, with 'error' saying why, if 'seedText' is needed but isn't a valid seed.
    bool ApplySeedChoice(bool newSeedEachRun, std::string const& seedText, std::string& error);
    bool Enabled() const;
    // The seed every run uses, or std::nullopt for a new random seed each run.
    std::optional<unsigned int> FixedSeed() const;
    // The seed the latest shuffled run used, if any has run.
    std::optional<unsigned int> LastSeed() const;
    // The seed for the next run, or std::nullopt when shuffling is off: the fixed seed, or a new random one.
    // Either way, it becomes LastSeed().
    std::optional<unsigned int> NextRunSeed();
    // The seed Shuffle Seed... offers: the fixed seed, or else the latest run's, or else a new random one.
    unsigned int SeedToShow() const;
    void SetEnabled(bool enabled);
    // A short description for the status bar, e.g. "Shuffle: off", "Shuffle: seed 42", or
    // "Shuffle: random (last 42)".
    std::string StatusText() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_ShuffleH
