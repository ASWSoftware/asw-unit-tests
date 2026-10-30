/* **************************************************************************
ASWUnitTests_Win32GUI_Splitter.h
Author: Anthony S. West - ASW Software

The Win32 GUI runner's splitter bars, the counterpart of the VCL's
TSplitter: a bar between two panes, dragged to resize them.

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
#ifndef ASWUnitTests_Win32GUI_SplitterH
#define ASWUnitTests_Win32GUI_SplitterH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <functional>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWin32GUISplitter
//
// A bar its parent window positions between two panes. Dragging it reports
// where the bar would go; the parent decides where it does go, keeping each
// pane at least its minimum size, and lays itself out again, as the VCL's
// TSplitter with ResizeStyle rsUpdate does.
/////////////////////////////////////////////////////////////////////////////
class TWin32GUISplitter
{
public:
    enum class EOrientation
    {
        // A tall bar between a left and a right pane, dragged left and right.
        Vertical,
        // A wide bar between a top and a bottom pane, dragged up and down.
        Horizontal,
    };

    // Called while the bar is dragged, with where its left (Vertical) or top (Horizontal) edge would be, in its
    // parent's client coordinates. Must not throw: it runs inside the splitter's window procedure.
    using TDragHandler = std::function<void (int position)>;

private:
    TDragHandler m_OnDrag;
    // Where in the bar it was grabbed, from its left or top edge, while it's being dragged.
    int m_GrabOffset;
    EOrientation m_Orientation;
    HWND m_Window;

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

public:
    TWin32GUISplitter();

    TWin32GUISplitter(TWin32GUISplitter const&) = delete;
    TWin32GUISplitter& operator=(TWin32GUISplitter const&) = delete;

    // Creates the bar as a child of 'parent', which positions it. Throws std::runtime_error if it can't be
    // created. The bar is destroyed with its parent, which must outlive neither this object nor its handler.
    void Create(HWND parent, EOrientation orientation, TDragHandler onDrag);
    HWND Handle() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_SplitterH
