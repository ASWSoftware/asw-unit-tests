/* **************************************************************************
ASWUnitTests_Win32GUI_Windows.h
Author: Anthony S. West - ASW Software

The one place the Win32 GUI runner includes the Windows headers, after
setting the Windows version they target (Windows 10) and NOMINMAX.

Include it first in every Win32 GUI source file, before any other header:
the MinGW-w64 C++ library headers set _WIN32_WINNT to an older version
(Windows 7) when it isn't already set, which would hide the Windows 10
declarations the GUI uses. Including it later stops the build with an
error rather than failing in less obvious ways.

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
#ifndef ASWUnitTests_Win32GUI_WindowsH
#define ASWUnitTests_Win32GUI_WindowsH
//---------------------------------------------------------------------------

// Windows 10 (version 1703 or later at run time, for per-monitor v2 DPI awareness).
#if !defined(_WIN32_WINNT)
#  define _WIN32_WINNT 0x0A00
#elif _WIN32_WINNT < 0x0A00
#  error "_WIN32_WINNT is set below Windows 10: include ASWUnitTests_Win32GUI_Windows.h before any other header."
#endif

#if !defined(WINVER)
#  define WINVER 0x0A00
#elif WINVER < 0x0A00
#  error "WINVER is set below Windows 10: include ASWUnitTests_Win32GUI_Windows.h before any other header."
#endif

#if !defined(NOMINMAX)
#  define NOMINMAX
#endif

#if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
#endif

//---------------------------------------------------------------------------
#include <windows.h>
//---------------------------------------------------------------------------
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include <shellapi.h>
//---------------------------------------------------------------------------
#endif
