/* **************************************************************************
Test_ASWUnitTests_GUI_Shuffle.cpp
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
// Module header
#include "Test_ASWUnitTests_GUI_Shuffle.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Shuffle.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

TGUIShuffle::TSeedSource SeedSequence(std::vector<unsigned int> const& seeds, size_t& callCount);

//---------------------------------------------------------------------------

// New seeds that come from 'seeds' in turn (repeating the last one once they run out), counting each call in
// 'callCount', which must outlive the returned source.
TGUIShuffle::TSeedSource SeedSequence(std::vector<unsigned int> const& seeds, size_t& callCount)
{
    callCount = 0;
    return [seeds, &callCount]()
        {
            unsigned int const seed = seeds[std::min(callCount, seeds.size() - 1)];
            ++callCount;
            return seed;
        };
}

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Shuffle
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Shuffle::TTest_ASWUnitTests_GUI_Shuffle()
    : inherited("ASWUnitTests_GUI_Shuffle_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_ApplyCommandLine_SeedTurnsShufflingOn,
        "ApplyCommandLine_SeedTurnsShufflingOn");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_ApplySeedChoice_FixedSeed, "ApplySeedChoice_FixedSeed");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_ApplySeedChoice_NewSeedEachRun,
        "ApplySeedChoice_NewSeedEachRun");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_ApplySeedChoice_RejectsInvalidSeed,
        "ApplySeedChoice_RejectsInvalidSeed");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_NextRunSeed_FixedSeedRepeats, "NextRunSeed_FixedSeedRepeats");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_NextRunSeed_NewSeedEachRun, "NextRunSeed_NewSeedEachRun");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_NextRunSeed_OffReturnsNothing, "NextRunSeed_OffReturnsNothing");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_SeedToShow_PrefersFixedThenLastSeed,
        "SeedToShow_PrefersFixedThenLastSeed");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_SetEnabled_KeepsSeeds, "SetEnabled_KeepsSeeds");
    RegisterTest(&TTest_ASWUnitTests_GUI_Shuffle::Test_StatusText_DescribesState, "StatusText_DescribesState");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Shuffle::~TTest_ASWUnitTests_GUI_Shuffle()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_ApplyCommandLine_SeedTurnsShufflingOn()
{
    // Arrange
    TGUIShuffle defaults;
    TGUIShuffle shuffleOnly;
    TGUIShuffle seedOnly;

    // Act
    shuffleOnly.ApplyCommandLine(true, std::nullopt);
    seedOnly.ApplyCommandLine(false, 42u);

    // Assert
    CheckFalse(defaults.Enabled(), __func__, __LINE__, "shuffling starts out off");
    CheckTrue(shuffleOnly.Enabled(), __func__, __LINE__, "--shuffle turns it on");
    CheckFalse(shuffleOnly.FixedSeed().has_value(), __func__, __LINE__, "with a new seed for each run");
    CheckTrue(seedOnly.Enabled(), __func__, __LINE__, "--shuffle-seed turns it on too");
    CheckTrue(seedOnly.FixedSeed() == 42u, __func__, __LINE__, "with that seed for every run");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_ApplySeedChoice_FixedSeed()
{
    // Arrange
    TGUIShuffle shuffle;
    std::string error;

    // Act
    bool const applied = shuffle.ApplySeedChoice(false, " 12345\t", error);

    // Assert
    CheckTrue(applied, __func__, __LINE__, "a valid seed, with spaces around it, is accepted");
    CheckEmpty(error, __func__, __LINE__, "with no error");
    CheckTrue(shuffle.Enabled(), __func__, __LINE__, "choosing a seed turns shuffling on");
    CheckTrue(shuffle.FixedSeed() == 12345u, __func__, __LINE__, "and every run uses that seed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_ApplySeedChoice_NewSeedEachRun()
{
    // Arrange
    TGUIShuffle shuffle;
    shuffle.ApplyCommandLine(false, 42u);
    shuffle.SetEnabled(false);
    std::string error;

    // Act
    bool const applied = shuffle.ApplySeedChoice(true, "not a seed", error);

    // Assert
    CheckTrue(applied, __func__, __LINE__, "the seed text isn't needed, so it isn't checked");
    CheckEmpty(error, __func__, __LINE__, "with no error");
    CheckTrue(shuffle.Enabled(), __func__, __LINE__, "shuffling is turned back on");
    CheckFalse(shuffle.FixedSeed().has_value(), __func__, __LINE__, "and the fixed seed is gone");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_ApplySeedChoice_RejectsInvalidSeed()
{
    // Arrange
    std::vector<std::string> const invalidSeeds = { "", "   ", "abc", "-1", "12 34", "4294967296" };

    for (std::string const& seedText : invalidSeeds)
    {
        TGUIShuffle shuffle;
        shuffle.ApplyCommandLine(false, 7u);
        shuffle.SetEnabled(false);
        std::string error;

        // Act
        bool const applied = shuffle.ApplySeedChoice(false, seedText, error);

        // Assert
        std::string const what = "\"" + seedText + "\"";
        CheckFalse(applied, __func__, __LINE__, what + " is rejected");
        CheckNotEmpty(error, __func__, __LINE__, what + " has an error saying why");
        CheckFalse(shuffle.Enabled(), __func__, __LINE__, what + " leaves shuffling off");
        CheckTrue(shuffle.FixedSeed() == 7u, __func__, __LINE__, what + " leaves the seed as it was");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_NextRunSeed_FixedSeedRepeats()
{
    // Arrange
    size_t newSeedCalls = 0;
    TGUIShuffle shuffle(SeedSequence({ 100u }, newSeedCalls));
    shuffle.ApplyCommandLine(true, 7u);

    // Act
    std::optional<unsigned int> const first = shuffle.NextRunSeed();
    std::optional<unsigned int> const second = shuffle.NextRunSeed();

    // Assert
    CheckTrue(first == 7u, __func__, __LINE__, "the first run uses the fixed seed");
    CheckTrue(second == 7u, __func__, __LINE__, "and so does the next");
    CheckTrue(shuffle.LastSeed() == 7u, __func__, __LINE__, "which is the last seed");
    CheckEquals(static_cast<size_t>(0), newSeedCalls, __func__, __LINE__, "no new seed was needed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_NextRunSeed_NewSeedEachRun()
{
    // Arrange
    size_t newSeedCalls = 0;
    TGUIShuffle shuffle(SeedSequence({ 10u, 20u }, newSeedCalls));
    shuffle.SetEnabled(true);

    // Act
    std::optional<unsigned int> const first = shuffle.NextRunSeed();
    std::optional<unsigned int> const second = shuffle.NextRunSeed();

    // Assert
    CheckTrue(first == 10u, __func__, __LINE__, "the first run gets a new seed");
    CheckTrue(second == 20u, __func__, __LINE__, "and the next run another");
    CheckTrue(shuffle.LastSeed() == 20u, __func__, __LINE__, "the latest is kept, to repeat that run's order");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_NextRunSeed_OffReturnsNothing()
{
    // Arrange
    size_t newSeedCalls = 0;
    TGUIShuffle shuffle(SeedSequence({ 10u, 20u }, newSeedCalls));
    shuffle.SetEnabled(true);
    shuffle.NextRunSeed();
    shuffle.SetEnabled(false);

    // Act
    std::optional<unsigned int> const seed = shuffle.NextRunSeed();

    // Assert
    CheckFalse(seed.has_value(), __func__, __LINE__, "an unshuffled run has no seed");
    CheckTrue(shuffle.LastSeed() == 10u, __func__, __LINE__, "and the last shuffled run's seed is kept");
    CheckEquals(static_cast<size_t>(1), newSeedCalls, __func__, __LINE__, "no new seed was taken for it");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_SeedToShow_PrefersFixedThenLastSeed()
{
    // Arrange
    size_t newSeedCalls = 0;
    TGUIShuffle shuffle(SeedSequence({ 10u, 20u }, newSeedCalls));

    // Act & Assert
    CheckEquals(10u, shuffle.SeedToShow(), __func__, __LINE__, "with no seed yet, a new one");

    shuffle.SetEnabled(true);
    shuffle.NextRunSeed(); // Takes 20.
    CheckEquals(20u, shuffle.SeedToShow(), __func__, __LINE__, "after a run, that run's seed");

    shuffle.ApplyCommandLine(true, 7u);
    CheckEquals(7u, shuffle.SeedToShow(), __func__, __LINE__, "with a fixed seed, that seed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_SetEnabled_KeepsSeeds()
{
    // Arrange
    TGUIShuffle shuffle;
    shuffle.ApplyCommandLine(true, 7u);
    shuffle.NextRunSeed();

    // Act
    shuffle.SetEnabled(false);
    shuffle.SetEnabled(true);

    // Assert
    CheckTrue(shuffle.FixedSeed() == 7u, __func__, __LINE__, "turning shuffling off and on keeps the fixed seed");
    CheckTrue(shuffle.LastSeed() == 7u, __func__, __LINE__, "and the last seed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Shuffle::Test_StatusText_DescribesState()
{
    // Arrange
    size_t newSeedCalls = 0;
    TGUIShuffle shuffle(SeedSequence({ 10u }, newSeedCalls));

    // Act & Assert
    CheckEquals(std::string("Shuffle: off"), shuffle.StatusText(), __func__, __LINE__, "off");

    shuffle.SetEnabled(true);
    CheckEquals(std::string("Shuffle: random"), shuffle.StatusText(), __func__, __LINE__,
        "on, before any shuffled run");

    shuffle.NextRunSeed();
    CheckEquals(std::string("Shuffle: random (last 10)"), shuffle.StatusText(), __func__, __LINE__,
        "on, after a shuffled run");

    shuffle.ApplyCommandLine(true, 4294967295u);
    CheckEquals(std::string("Shuffle: seed 4294967295"), shuffle.StatusText(), __func__, __LINE__,
        "with a fixed seed");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Shuffle)
