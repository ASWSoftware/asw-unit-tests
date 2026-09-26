/* **************************************************************************
ASWUnitTests_CrashGuard.h
Author: Anthony S. West - ASW Software

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

#ifndef ASWUnitTests_CrashGuardH
#define ASWUnitTests_CrashGuardH
//---------------------------------------------------------------------------
#include <functional>
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TCrashGuardResult
//
// Outcome of TCrashGuard::Run(). 'Crashed' is false if the callback completed
// normally, including via an ordinary C++ exception, which propagates out of
// Run() completely unaffected; only a native fault (a Windows structured
// exception, or a POSIX fault signal) sets it. 'ShouldAbortRun' is set only
// for a fault severe enough that running any further tests in this process
// isn't considered safe; see TCrashGuard::Run() for exactly which faults
// that is and why the two platforms draw the line differently.
/////////////////////////////////////////////////////////////////////////////
struct TCrashGuardResult
{
    bool Crashed = false;
    bool ShouldAbortRun = false;
    std::string Description;
};


/////////////////////////////////////////////////////////////////////////////
// TCrashGuard
//
// Runs a callback with native-crash protection, backing
// TTestGroupBase::RunCatchingCrashes(). See ASWUnitTests_CrashGuard.cpp for
// the mechanism and its caveats: this cannot run any destructors for objects
// that were under construction on the callback's own stack at the moment of
// the fault, and it never attempts to handle SIGABRT.
/////////////////////////////////////////////////////////////////////////////
class TCrashGuard
{
public:
    static TCrashGuardResult Run(std::function<void()> const& body);
};

} // namespace ASWUnitTests

#endif // #ifndef ASWUnitTests_CrashGuardH
