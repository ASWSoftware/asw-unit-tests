/* **************************************************************************
Test_ASWUnitTests_CLI.cpp
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
#include "Test_ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_CLI
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_CLI::TTest_ASWUnitTests_CLI()
    : inherited("ASWUnitTests_CLI_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_BuildTestFilter_Filter, "BuildTestFilter_Filter");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_BuildTestFilter_Partition, "BuildTestFilter_Partition");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_Color, "ParseArguments_Color");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_Filter, "ParseArguments_Filter");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_HelpVersionList, "ParseArguments_HelpVersionList");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_InvalidOption, "ParseArguments_InvalidOption");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_PartitionValidation, "ParseArguments_PartitionValidation");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_Pause, "ParseArguments_Pause");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_ReportAndProjectName, "ParseArguments_ReportAndProjectName");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_Shuffle, "ParseArguments_Shuffle");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseArguments_TestTimeout, "ParseArguments_TestTimeout");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseColorMode, "ParseColorMode");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseUnsignedInt_Invalid, "ParseUnsignedInt_Invalid");
    RegisterTest(&TTest_ASWUnitTests_CLI::Test_ParseUnsignedInt_Valid, "ParseUnsignedInt_Valid");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_CLI::~TTest_ASWUnitTests_CLI()
{
}
//---------------------------------------------------------------------------
std::optional<int> TTest_ASWUnitTests_CLI::ParseArgs(std::vector<std::string> args, TCLIOptions& options)
{
    std::vector<char*> argv;
    argv.reserve(args.size());

    for (std::string& arg : args)
        argv.push_back(arg.data());

    // ParseArguments() writes its own parse errors, and --help/--version's text, directly to
    // std::cout (it has no TTestGroupBase to suppress); redirect so that doesn't print into the
    // real suite's console output and look like a genuine failure.
    TStdOutRedirect const suppressOutput;
    return TCLIParser::ParseArguments(static_cast<int>(argv.size()), argv.data(), options);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_BuildTestFilter_Filter()
{
    // Arrange
    TTestHandler tester;
    {
        TStdOutRedirect const suppressOutput; // Initialize() logs its own version/registration banner.
        tester.Initialize("Test_BuildTestFilter_Filter");
    }

    TCLIOptions options;
    options.HasFilter = true;
    options.FilterPattern = "*HexSingleToByte*";

    std::string filterDescription;

    // Act
    TestFilter const filter = TCLIParser::BuildTestFilter(tester, options, filterDescription);

    // Assert
    CheckTrue(filter != nullptr, __func__, __LINE__, "filter is constructed");
    CheckTrue(filter("ASWTools_String_Tests.HexSingleToByte[A]"), __func__, __LINE__, "matches a test fitting the pattern");
    CheckFalse(filter("ASWTools_String_Tests.Compare"), __func__, __LINE__, "does not match an unrelated test");
    CheckTrue(filterDescription.find("HexSingleToByte") != std::string::npos,
        __func__, __LINE__, "description mentions the pattern");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_BuildTestFilter_Partition()
{
    // Arrange
    TTestHandler tester;
    {
        TStdOutRedirect const suppressOutput; // Initialize() logs its own version/registration banner.
        tester.Initialize("Test_BuildTestFilter_Partition");
    }

    std::vector<std::string> const allNames = tester.GetAllTestFullNames();

    TCLIOptions options1;
    options1.HasPartitionCount = true;
    options1.PartitionIndex = 1;
    options1.PartitionCount = 2;

    TCLIOptions options2;
    options2.HasPartitionCount = true;
    options2.PartitionIndex = 2;
    options2.PartitionCount = 2;

    std::string description1;
    std::string description2;

    // Act
    TestFilter const filter1 = TCLIParser::BuildTestFilter(tester, options1, description1);
    TestFilter const filter2 = TCLIParser::BuildTestFilter(tester, options2, description2);

    size_t matchedByEither = 0;
    size_t matchedByBoth = 0;

    for (std::string const& name : allNames)
    {
        bool const inPartition1 = filter1(name);
        bool const inPartition2 = filter2(name);

        if (inPartition1 || inPartition2)
            ++matchedByEither;

        if (inPartition1 && inPartition2)
            ++matchedByBoth;
    }

    // Assert
    CheckEquals(static_cast<size_t>(0), matchedByBoth, __func__, __LINE__, "the two partitions do not overlap");
    CheckEquals(allNames.size(), matchedByEither, __func__, __LINE__, "the two partitions cover every test exactly once");
    CheckTrue(description1.find("partition 1 of 2") != std::string::npos, __func__, __LINE__, "partition 1 description");
    CheckTrue(description2.find("partition 2 of 2") != std::string::npos, __func__, __LINE__, "partition 2 description");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_Color()
{
    // Arrange
    TCLIOptions optionsMode;
    TCLIOptions optionsNoColor;
    TCLIOptions optionsColorPass;
    TCLIOptions optionsBadMode;

    // Act
    ParseArgs({ "ASWUnitTests", "--color", "always" }, optionsMode);
    ParseArgs({ "ASWUnitTests", "--no-color" }, optionsNoColor);
    ParseArgs({ "ASWUnitTests", "--color-pass", "bright-green" }, optionsColorPass);
    std::optional<int> const resultBadMode = ParseArgs({ "ASWUnitTests", "--color", "bogus" }, optionsBadMode);

    // Assert
    CheckTrue(optionsMode.ColorMode == TColorMode::Always, __func__, __LINE__, "--color always");
    CheckTrue(optionsNoColor.ColorMode == TColorMode::Never, __func__, __LINE__, "--no-color");
    CheckTrue(optionsColorPass.ColorPass == TConsoleColor::BrightGreen, __func__, __LINE__, "--color-pass bright-green");
    CheckTrue(resultBadMode.has_value() && *resultBadMode == ExitCode_InvalidArguments,
        __func__, __LINE__, "--color with an unrecognized mode is rejected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_Filter()
{
    // Arrange
    TCLIOptions optionsSpaceForm;
    TCLIOptions optionsEqualsForm;
    TCLIOptions optionsIgnoreCase;

    // Act
    std::optional<int> const resultSpaceForm = ParseArgs({ "ASWUnitTests", "--filter", "*.cpp" }, optionsSpaceForm);
    std::optional<int> const resultEqualsForm = ParseArgs({ "ASWUnitTests", "--filter=*.cpp" }, optionsEqualsForm);
    ParseArgs({ "ASWUnitTests", "--filter", "*.cpp", "--filter-ignore-case" }, optionsIgnoreCase);

    // Assert
    CheckFalse(resultSpaceForm.has_value(), __func__, __LINE__, "--filter <pattern> continues (no early exit)");
    CheckTrue(optionsSpaceForm.HasFilter, __func__, __LINE__, "--filter <pattern> sets HasFilter");
    CheckEquals(std::string("*.cpp"), optionsSpaceForm.FilterPattern, __func__, __LINE__, "--filter <pattern> value");
    CheckFalse(optionsSpaceForm.FilterIgnoreCase, __func__, __LINE__, "--filter-ignore-case not set by default");

    CheckFalse(resultEqualsForm.has_value(), __func__, __LINE__, "--filter=<pattern> continues");
    CheckTrue(optionsEqualsForm.HasFilter, __func__, __LINE__, "--filter=<pattern> sets HasFilter");
    CheckEquals(std::string("*.cpp"), optionsEqualsForm.FilterPattern, __func__, __LINE__, "--filter=<pattern> value");

    CheckTrue(optionsIgnoreCase.FilterIgnoreCase, __func__, __LINE__, "--filter-ignore-case sets FilterIgnoreCase");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_HelpVersionList()
{
    // Arrange
    TCLIOptions optionsHelp;
    TCLIOptions optionsVersion;
    TCLIOptions optionsList;

    // Act
    std::optional<int> const resultHelp = ParseArgs({ "ASWUnitTests", "--help" }, optionsHelp);
    std::optional<int> const resultVersion = ParseArgs({ "ASWUnitTests", "--version" }, optionsVersion);
    std::optional<int> const resultList = ParseArgs({ "ASWUnitTests", "--list" }, optionsList);

    // Assert
    CheckTrue(resultHelp.has_value() && *resultHelp == ExitCode_Success, __func__, __LINE__, "--help exits with success");
    CheckTrue(resultVersion.has_value() && *resultVersion == ExitCode_Success,
        __func__, __LINE__, "--version exits with success");
    CheckFalse(resultList.has_value(), __func__, __LINE__, "--list continues (does not exit early)");
    CheckTrue(optionsList.ListOnly, __func__, __LINE__, "--list sets ListOnly");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_InvalidOption()
{
    // Arrange
    TCLIOptions options;

    // Act
    std::optional<int> const result = ParseArgs({ "ASWUnitTests", "--this-flag-does-not-exist" }, options);

    // Assert
    CheckTrue(result.has_value() && *result == ExitCode_InvalidArguments, __func__, __LINE__, "unrecognized option is rejected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_PartitionValidation()
{
    // Arrange
    TCLIOptions optionsMissingCount;
    TCLIOptions optionsZeroCount;
    TCLIOptions optionsIndexTooHigh;
    TCLIOptions optionsValid;

    // Act
    std::optional<int> const resultMissingCount = ParseArgs(
        { "ASWUnitTests", "--partition-index", "1" }, optionsMissingCount);
    std::optional<int> const resultZeroCount = ParseArgs(
        { "ASWUnitTests", "--partition-index", "1", "--partition-count", "0" }, optionsZeroCount);
    std::optional<int> const resultIndexTooHigh = ParseArgs(
        { "ASWUnitTests", "--partition-index", "5", "--partition-count", "3" }, optionsIndexTooHigh);
    std::optional<int> const resultValid = ParseArgs(
        { "ASWUnitTests", "--partition-index", "2", "--partition-count", "3" }, optionsValid);

    // Assert
    CheckTrue(resultMissingCount.has_value() && *resultMissingCount == ExitCode_InvalidArguments,
        __func__, __LINE__, "--partition-index without --partition-count is rejected");
    CheckTrue(resultZeroCount.has_value() && *resultZeroCount == ExitCode_InvalidArguments,
        __func__, __LINE__, "--partition-count 0 is rejected");
    CheckTrue(resultIndexTooHigh.has_value() && *resultIndexTooHigh == ExitCode_InvalidArguments,
        __func__, __LINE__, "--partition-index greater than --partition-count is rejected");

    CheckFalse(resultValid.has_value(), __func__, __LINE__, "a valid partition pair continues");
    CheckEquals(2u, optionsValid.PartitionIndex, __func__, __LINE__, "PartitionIndex parsed");
    CheckEquals(3u, optionsValid.PartitionCount, __func__, __LINE__, "PartitionCount parsed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_Pause()
{
    // Arrange
    TCLIOptions optionsDefault;
    TCLIOptions optionsPause;

    // Act
    ParseArgs({ "ASWUnitTests" }, optionsDefault);
    ParseArgs({ "ASWUnitTests", "--pause" }, optionsPause);

    // Assert
    CheckFalse(optionsDefault.PauseOnExit, __func__, __LINE__, "PauseOnExit defaults to false");
    CheckTrue(optionsPause.PauseOnExit, __func__, __LINE__, "--pause sets PauseOnExit");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_ReportAndProjectName()
{
    // Arrange
    TCLIOptions options;

    // Act
    ParseArgs({ "ASWUnitTests", "--report-junit", "results.xml", "--project-name", "MyProject" }, options);

    // Assert
    CheckEquals(std::string("results.xml"), options.JunitReportPath, __func__, __LINE__, "--report-junit path");
    CheckEquals(std::string("MyProject"), options.ProjectName, __func__, __LINE__, "--project-name value");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_Shuffle()
{
    // Arrange
    TCLIOptions optionsBare;
    TCLIOptions optionsSeedSpace;
    TCLIOptions optionsSeedEquals;
    TCLIOptions optionsBadSeed;

    // Act
    ParseArgs({ "ASWUnitTests", "--shuffle" }, optionsBare);
    ParseArgs({ "ASWUnitTests", "--shuffle-seed", "42" }, optionsSeedSpace);
    ParseArgs({ "ASWUnitTests", "--shuffle-seed=42" }, optionsSeedEquals);
    std::optional<int> const resultBadSeed = ParseArgs({ "ASWUnitTests", "--shuffle-seed", "abc" }, optionsBadSeed);

    // Assert
    CheckTrue(optionsBare.Shuffle, __func__, __LINE__, "--shuffle sets Shuffle");
    CheckFalse(optionsBare.ShuffleSeed.has_value(), __func__, __LINE__, "--shuffle alone leaves no explicit seed");

    CheckTrue(optionsSeedSpace.Shuffle, __func__, __LINE__, "--shuffle-seed implies Shuffle");
    CheckTrue(optionsSeedSpace.ShuffleSeed == 42u, __func__, __LINE__, "--shuffle-seed <N> value");

    CheckTrue(optionsSeedEquals.ShuffleSeed == 42u, __func__, __LINE__, "--shuffle-seed=<N> value");

    CheckTrue(resultBadSeed.has_value() && *resultBadSeed == ExitCode_InvalidArguments,
        __func__, __LINE__, "--shuffle-seed with a non-numeric argument is rejected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseArguments_TestTimeout()
{
    // Arrange
    TCLIOptions optionsSpace;
    TCLIOptions optionsEquals;
    TCLIOptions optionsBare;
    TCLIOptions optionsZero;
    TCLIOptions optionsNonNumeric;

    // Act
    ParseArgs({ "ASWUnitTests", "--test-timeout-seconds", "5" }, optionsSpace);
    ParseArgs({ "ASWUnitTests", "--test-timeout-seconds=5" }, optionsEquals);
    std::optional<int> const resultZero = ParseArgs({ "ASWUnitTests", "--test-timeout-seconds", "0" }, optionsZero);
    std::optional<int> const resultNonNumeric =
        ParseArgs({ "ASWUnitTests", "--test-timeout-seconds", "abc" }, optionsNonNumeric);

    // Assert
    CheckFalse(optionsBare.TestTimeoutSeconds.has_value(), __func__, __LINE__, "no default when the flag is absent");
    CheckTrue(optionsSpace.TestTimeoutSeconds == 5u, __func__, __LINE__, "--test-timeout-seconds <N> value");
    CheckTrue(optionsEquals.TestTimeoutSeconds == 5u, __func__, __LINE__, "--test-timeout-seconds=<N> value");

    CheckTrue(resultZero.has_value() && *resultZero == ExitCode_InvalidArguments,
        __func__, __LINE__, "--test-timeout-seconds 0 is rejected, since it can't ever let a test finish");
    CheckTrue(resultNonNumeric.has_value() && *resultNonNumeric == ExitCode_InvalidArguments,
        __func__, __LINE__, "--test-timeout-seconds with a non-numeric argument is rejected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseColorMode()
{
    // Act & Assert
    CheckTrue(TCLIParser::ParseColorMode("auto") == TColorMode::Auto, __func__, __LINE__, "auto");
    CheckTrue(TCLIParser::ParseColorMode("always") == TColorMode::Always, __func__, __LINE__, "always");
    CheckTrue(TCLIParser::ParseColorMode("never") == TColorMode::Never, __func__, __LINE__, "never");
    CheckFalse(TCLIParser::ParseColorMode("bogus").has_value(), __func__, __LINE__, "unrecognized mode");
    CheckFalse(TCLIParser::ParseColorMode("").has_value(), __func__, __LINE__, "empty string");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseUnsignedInt_Invalid()
{
    // Act & Assert
    CheckFalse(TCLIParser::ParseUnsignedInt("").has_value(), __func__, __LINE__, "empty string");
    CheckFalse(TCLIParser::ParseUnsignedInt("abc").has_value(), __func__, __LINE__, "non-numeric");
    CheckFalse(TCLIParser::ParseUnsignedInt("123abc").has_value(), __func__, __LINE__, "trailing garbage after digits");
    CheckFalse(TCLIParser::ParseUnsignedInt("-5").has_value(),
        __func__, __LINE__, "negative number rejected (wraps past unsigned int max)");
    CheckFalse(TCLIParser::ParseUnsignedInt("99999999999").has_value(), __func__, __LINE__, "overflows unsigned int");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CLI::Test_ParseUnsignedInt_Valid()
{
    // Act & Assert
    CheckTrue(TCLIParser::ParseUnsignedInt("0") == 0u, __func__, __LINE__, "zero");
    CheckTrue(TCLIParser::ParseUnsignedInt("123") == 123u, __func__, __LINE__, "ordinary value");
    CheckTrue(TCLIParser::ParseUnsignedInt("4294967295") == 4294967295u, __func__, __LINE__, "exactly UINT_MAX");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_CLI)
