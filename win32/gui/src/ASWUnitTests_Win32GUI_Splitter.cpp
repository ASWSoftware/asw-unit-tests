/* **************************************************************************
ASWUnitTests_Win32GUI_Splitter.cpp
Author: Anthony S. West - ASW Software

The Win32 GUI runner's splitter bars.

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
#include "ASWUnitTests_Win32GUI_Splitter.h"
//---------------------------------------------------------------------------
#include <stdexcept>
#include <utility>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

wchar_t const SplitterClassName[] = L"ASWUnitTests_Win32GUI_Splitter";

} // namespace

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TWin32GUISplitter
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWin32GUISplitter::TWin32GUISplitter()
    : m_GrabOffset(0),
      m_Orientation(EOrientation::Vertical),
      m_Window(nullptr)
{
}
//---------------------------------------------------------------------------
void TWin32GUISplitter::Create(HWND parent, EOrientation orientation, TDragHandler onDrag)
{
    HINSTANCE const instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW windowClass = {};

    if (GetClassInfoExW(instance, SplitterClassName, &windowClass) == 0)
    {
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = &WindowProc;
        windowClass.hInstance = instance;
        windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        windowClass.lpszClassName = SplitterClassName;

        if (RegisterClassExW(&windowClass) == 0)
            throw std::runtime_error("Couldn't register the splitter window class");
    }

    m_Orientation = orientation;
    m_OnDrag = std::move(onDrag);

    if (CreateWindowExW(0, SplitterClassName, L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, nullptr, instance,
        this) == nullptr)
    {
        throw std::runtime_error("Couldn't create a splitter");
    }
}
//---------------------------------------------------------------------------
HWND TWin32GUISplitter::Handle() const
{
    return m_Window;
}
//---------------------------------------------------------------------------
LRESULT TWin32GUISplitter::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND const window = m_Window; // Still needed after WM_NCDESTROY clears m_Window.

    switch (message)
    {
        case WM_CAPTURECHANGED:
            m_GrabOffset = 0;
            return 0;

        case WM_LBUTTONDOWN:
            m_GrabOffset = m_Orientation == EOrientation::Vertical ? static_cast<short>(LOWORD(lParam)) :
                    static_cast<short>(HIWORD(lParam));
            SetCapture(m_Window);
            return 0;

        case WM_LBUTTONUP:
            if (GetCapture() == m_Window)
                ReleaseCapture();

            return 0;

        case WM_MOUSEMOVE:
            if (GetCapture() == m_Window && m_OnDrag)
            {
                // The mouse in the parent's client coordinates, less where the bar was grabbed.
                POINT point = { static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)) };
                MapWindowPoints(m_Window, GetParent(m_Window), &point, 1);
                m_OnDrag((m_Orientation == EOrientation::Vertical ? point.x : point.y) - m_GrabOffset);
            }

            return 0;

        case WM_NCDESTROY:
            m_Window = nullptr;
            break;

        case WM_SETCURSOR:
            // IDC_SIZEWE and IDC_SIZENS; the W versions of those macros need UNICODE.
            SetCursor(LoadCursorW(nullptr, MAKEINTRESOURCEW(m_Orientation == EOrientation::Vertical ? 32644 : 32645)));
            return TRUE;

        default:
            break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
//---------------------------------------------------------------------------
/*
    TWin32GUISplitter::WindowProc

    Passes each message to the splitter that created the window, whose address CreateWindowExW() received.
    No exception may leave a window procedure, since unwinding through Windows' own frames is undefined; the
    drag handler must not throw, and anything else that does is ignored, leaving the message to Windows.
*/
LRESULT CALLBACK TWin32GUISplitter::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        TWin32GUISplitter* const splitter = static_cast<TWin32GUISplitter*>(
            reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        splitter->m_Window = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(splitter));
    }

    TWin32GUISplitter* const splitter = reinterpret_cast<TWin32GUISplitter*>(GetWindowLongPtrW(window,
        GWLP_USERDATA));

    if (splitter != nullptr)
    {
        try
        {
            return splitter->HandleMessage(message, wParam, lParam);
        }
        catch (...)
        {
        }
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
