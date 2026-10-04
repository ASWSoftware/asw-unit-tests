/* **************************************************************************
ASWUnitTests_Version.h
Author: Anthony S. West - ASW Software

Single source for the ASWUnitTests semantic version (https://semver.org).

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

#ifndef ASWUnitTests_VersionH
#define ASWUnitTests_VersionH
//---------------------------------------------------------------------------

// The version, as macros so code can check it with #if (e.g. to leave out a call to a method an older version doesn't
// have). A release has an empty pre-release; between releases, the develop branch has the next planned version with a
// pre-release such as "dev.1", which comes before that release in version order (e.g. 1.1.0-dev.1 < 1.1.0).
// ASWUNITTESTS_VERSION_STRING is "MAJOR.MINOR.PATCH", followed by "-PRERELEASE" if the pre-release isn't empty; change
// them together (a unit test checks that they match).
#define ASWUNITTESTS_VERSION_MAJOR 1
#define ASWUNITTESTS_VERSION_MINOR 2
#define ASWUNITTESTS_VERSION_PATCH 0
#define ASWUNITTESTS_VERSION_PRERELEASE "dev.1"
#define ASWUNITTESTS_VERSION_STRING "1.2.0-dev.1"

namespace ASWUnitTests
{

// The same version, for C++ code
inline constexpr unsigned int VersionMajor = ASWUNITTESTS_VERSION_MAJOR;
inline constexpr unsigned int VersionMinor = ASWUNITTESTS_VERSION_MINOR;
inline constexpr unsigned int VersionPatch = ASWUNITTESTS_VERSION_PATCH;
inline constexpr char VersionPreRelease[] = ASWUNITTESTS_VERSION_PRERELEASE;
inline constexpr char Version[] = ASWUNITTESTS_VERSION_STRING;

} // namespace ASWUnitTests

#endif // #ifndef ASWUnitTests_VersionH
