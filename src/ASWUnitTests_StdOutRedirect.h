/* **************************************************************************
ASWUnitTests_StdOutRedirect.h
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

#ifndef ASWUnitTests_StdOutRedirectH
#define ASWUnitTests_StdOutRedirectH
//---------------------------------------------------------------------------
#include <sstream>
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TStdOutRedirect
//
// Redirects std::cout into an in-memory buffer, for testing code that writes
// to the console (including this framework's own TCLIParser, whose parse
// errors and --help/--version text go straight to std::cout, and
// TTestHandler::Initialize()'s banner). Starts capturing immediately on
// construction. Call Restore() to hand std::cout back before this object is
// destroyed (its destructor calls Restore() automatically if that hasn't
// already happened), and Capture() to start a fresh capture again
// afterward - discarding whatever the previous capture collected, the same
// way constructing a new TStdOutRedirect would. Both methods are safe to
// call when already in the state they'd produce (e.g. Restore() while not
// currently capturing is a no-op), so a single object can be toggled back
// and forth as many times as a test needs.
/////////////////////////////////////////////////////////////////////////////
class TStdOutRedirect
{
private:
    std::ostringstream m_Captured;
    std::streambuf* m_OriginalBuffer; // Non-null while capturing; nullptr otherwise.

public:
    TStdOutRedirect();
    ~TStdOutRedirect();

    // Redirects std::cout into this object's buffer, discarding whatever a previous Capture()/Restore() cycle
    // collected. A no-op if already capturing.
    void Capture();

    // Restores std::cout to the stream buffer it had before capturing began. A no-op if not currently capturing.
    void Restore();

    // Everything written to std::cout during the current capture, or, if Restore() was since
    // called, the capture that just ended.
    std::string Str() const;
};

} // namespace ASWUnitTests

#endif // #ifndef ASWUnitTests_StdOutRedirectH
