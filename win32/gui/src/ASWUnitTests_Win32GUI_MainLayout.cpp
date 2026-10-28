/* **************************************************************************
ASWUnitTests_Win32GUI_MainLayout.cpp
Author: Anthony S. West - ASW Software

Where the Win32 GUI runner's main window puts its controls.

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
#include "ASWUnitTests_Win32GUI_MainLayout.h"
//---------------------------------------------------------------------------
#include <algorithm>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DPI.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

int ClampSplitPosition(int size, int minimum, int available);
int ResultsHeight(int clientHeight, UINT dpi);

//---------------------------------------------------------------------------
/*
    ClampSplitPosition

    'size' kept at least 'minimum' and at most what leaves 'minimum' of 'available' on the other side, with
    the minimum winning when 'available' is too small for both.
*/
int ClampSplitPosition(int size, int minimum, int available)
{
    return (std::max)(minimum, (std::min)(size, available - minimum));
}
//---------------------------------------------------------------------------
/*
    ResultsHeight

    The height between the toolbar and the progress bar, which the detail pane, its splitter and the results
    tabs share.
*/
int ResultsHeight(int clientHeight, UINT dpi)
{
    return clientHeight - ScaleForDPI(TWin32GUIMainSizes::StatusBarHeight, dpi) - ScaleForDPI(TWin32GUIMainSizes::ProgressBarHeight, dpi) -
        ScaleForDPI(TWin32GUIMainSizes::ToolbarHeight, dpi);
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
int ClampDetailHeight(int height, int clientHeight, UINT dpi)
{
    return ClampSplitPosition(height, ScaleForDPI(TWin32GUIMainSizes::DetailMinHeight, dpi),
        ResultsHeight(clientHeight, dpi) - ScaleForDPI(TWin32GUIMainSizes::SplitterSize, dpi));
}
//---------------------------------------------------------------------------
int ClampTestTreeWidth(int width, int clientWidth, UINT dpi)
{
    return ClampSplitPosition(width, ScaleForDPI(TWin32GUIMainSizes::TestTreeMinWidth, dpi),
        clientWidth - ScaleForDPI(TWin32GUIMainSizes::SplitterSize, dpi));
}
//---------------------------------------------------------------------------
TWin32GUIMainLayout LayOutMainWindow(int clientWidth, int clientHeight, int testTreeWidth, int detailHeight, UINT dpi)
{
    int const splitter = ScaleForDPI(TWin32GUIMainSizes::SplitterSize, dpi);
    int const statusTop = clientHeight - ScaleForDPI(TWin32GUIMainSizes::StatusBarHeight, dpi);
    int const progressTop = statusTop - ScaleForDPI(TWin32GUIMainSizes::ProgressBarHeight, dpi);
    int const top = ScaleForDPI(TWin32GUIMainSizes::ToolbarHeight, dpi);
    int const bottom = (std::max)(top, progressTop);
    int const resultsLeft = testTreeWidth + splitter;
    int const right = (std::max)(resultsLeft, clientWidth);
    int const detailBottom = top + detailHeight;

    TWin32GUIMainLayout layout;
    layout.TestTree = TGUIRect(0, top, testTreeWidth, bottom);
    layout.TestsSplitter = TGUIRect(testTreeWidth, top, resultsLeft, bottom);
    layout.Detail = TGUIRect(resultsLeft, top, right, detailBottom);
    layout.DetailSplitter = TGUIRect(resultsLeft, detailBottom, right, detailBottom + splitter);
    layout.Results = TGUIRect(resultsLeft, detailBottom + splitter, right, (std::max)(detailBottom + splitter, bottom));
    layout.ProgressBar = TGUIRect(0, progressTop, clientWidth, statusTop);
    layout.StatusBar = TGUIRect(0, statusTop, clientWidth, clientHeight);
    return layout;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
