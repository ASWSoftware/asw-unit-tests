/* **************************************************************************
Test_ASWUnitTests_Version.cpp
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
#include "Test_ASWUnitTests_Version.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <string>
#include <string_view>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_Version.h"
//---------------------------------------------------------------------------

namespace
{

// A SemVer pre-release identifier: not empty, only [0-9A-Za-z-], and no leading zero if it's all digits.
bool IsValidPreReleaseIdentifier(std::string_view identifier)
{
    if (identifier.empty())
        return false;

    bool isNumeric = true;
    for (char const character : identifier)
    {
        bool const isDigit = character >= '0' && character <= '9';
        bool const isLetter = (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z');
        if (!isDigit && !isLetter && character != '-')
            return false;

        isNumeric = isNumeric && isDigit;
    }

    return !isNumeric || identifier.size() == 1 || identifier.front() != '0';
}

} // namespace

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Version
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Version::TTest_ASWUnitTests_Version()
    : inherited("ASWUnitTests_Version_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Version::Test_GetVersionStr_ReturnsVersion, "GetVersionStr_ReturnsVersion");
    RegisterTest(&TTest_ASWUnitTests_Version::Test_PreRelease_IsValidSemVer, "PreRelease_IsValidSemVer");
    RegisterTest(&TTest_ASWUnitTests_Version::Test_VersionString_MatchesParts, "VersionString_MatchesParts");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Version::~TTest_ASWUnitTests_Version()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::Test_GetVersionStr_ReturnsVersion()
{
    // Act
    std::string const version = TTestHandler::GetVersionStr();

    // Assert
    CheckEquals(std::string(ASWUNITTESTS_VERSION_STRING), version, __func__, __LINE__,
        "GetVersionStr() should return ASWUNITTESTS_VERSION_STRING");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::Test_PreRelease_IsValidSemVer()
{
    // Arrange: a pre-release is empty (a release) or dot-separated identifiers, e.g. "dev.1"
    std::string_view const preRelease = VersionPreRelease;

    // Act
    bool isValid = true;
    for (std::size_t start = 0; !preRelease.empty() && start <= preRelease.size();)
    {
        std::size_t const dot = preRelease.find('.', start);
        std::size_t const end = (dot == std::string_view::npos) ? preRelease.size() : dot;
        isValid = isValid && IsValidPreReleaseIdentifier(preRelease.substr(start, end - start));
        start = end + 1;
    }

    // Assert
    CheckTrue(isValid, __func__, __LINE__,
        "ASWUNITTESTS_VERSION_PRERELEASE should be empty or SemVer pre-release identifiers: " +
        std::string(preRelease));
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Version::Test_VersionString_MatchesParts()
{
    // Arrange
    std::string const preRelease = VersionPreRelease;
    std::string expected = std::to_string(VersionMajor) + "." + std::to_string(VersionMinor) + "." +
        std::to_string(VersionPatch);
    if (!preRelease.empty())
        expected += "-" + preRelease;

    // Act
    std::string const version = Version;

    // Assert
    CheckEquals(expected, version, __func__, __LINE__,
        "ASWUNITTESTS_VERSION_STRING should be MAJOR.MINOR.PATCH, plus -PRERELEASE if there is one");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Version)
