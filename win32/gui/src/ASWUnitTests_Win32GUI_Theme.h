/* **************************************************************************
ASWUnitTests_Win32GUI_Theme.h
Author: Anthony S. West - ASW Software

The Win32 GUI runner's light and dark looks: Windows' standard one, and a
dark one painted in Carbon's colors (the VCL GUI runner's dark style),
drawing what Windows can't make dark itself.

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
#ifndef ASWUnitTests_Win32GUI_ThemeH
#define ASWUnitTests_Win32GUI_ThemeH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <memory>
#include <string>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DPI.h"
#include "ASWUnitTests_Win32GUI_Palette.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EWin32GUIThemedControl
//
// The kinds of child control TWin32GUITheme::Attach() themes, each its own
// way. Buttons, radio buttons, check boxes and labels need no attaching:
// their window does it (see TWin32GUITheme::AttachWindow()).
/////////////////////////////////////////////////////////////////////////////
enum class EWin32GUIThemedControl
{
    // An EDIT control, single-line or multiline, read-only or not.
    Edit,
    // A WC_LISTVIEWW control in report view, with its header.
    ListView,
    // A PROGRESS_CLASSW control.
    ProgressBar,
    // An MSFTEDIT_CLASS rich edit control. Its text keeps the colors it's given (see TWin32GUITheme's
    // WindowTextColor()).
    RichEdit,
    // A window that only fills itself with its parent's background, e.g. a splitter.
    Splitter,
    // A STATUSCLASSNAMEW control.
    StatusBar,
    // A WC_TABCONTROLW control, whose pages are separate windows above it.
    TabControl,
    // A TOOLTIPS_CLASSW window.
    Tooltip,
    // A WC_TREEVIEWW control, with or without TVS_CHECKBOXES (and TVS_EX_PARTIALCHECKBOXES).
    TreeView,
    // An UPDOWN_CLASSW control with UDS_HORZ, such as a tab control's scroll arrows, which attaching the tab
    // control attaches as it creates them.
    UpDown,
};


/////////////////////////////////////////////////////////////////////////////
// TWin32GUITheme
//
// Whether the GUI is light (Windows' standard look, in the system colors) or
// dark (painted in CarbonPalette()'s colors), applied to the windows and
// controls attached to it, and switched live by SetDark(). Dark has:
//   - the title bar Windows draws dark (DWMWA_USE_IMMERSIVE_DARK_MODE);
//   - owner-drawn menus, and the menus' frames painted through a hook on
//     this thread's menu windows;
//   - buttons, radio buttons and check boxes custom-drawn by their window;
//   - the tab control, status bar and progress bar painted whole, through
//     a buffer, so they don't flicker;
//   - the tree's and list's items and the list's header drawn by us, and
//     their scroll bars Windows' own dark ones ("DarkMode_Explorer");
//   - the edit boxes', tree's, list's and log's borders painted over.
// Make one per process, on the GUI's thread, before any window, and destroy
// it after the windows; dialogs attach themselves while they're open.
/////////////////////////////////////////////////////////////////////////////
class TWin32GUITheme
{
private:
    // An attached control and its kind.
    struct TAttachedControl
    {
        HWND Control;
        EWin32GUIThemedControl Kind;
    };

    // An item of an attached window's menu bar, or of one of its menus, while they're owner-drawn: the item's
    // address is its MENUITEMINFOW::dwItemData.
    struct TMenuItem
    {
        HMENU Menu;
        UINT Position;
        // Its caption, with any "\t" and shortcut.
        std::wstring Text;
        bool Separator;
        bool Submenu;
        bool TopLevel;
    };

private:
    // The theme whose hook paints the menus' frames; see MenuHookProc().
    static TWin32GUITheme* s_Instance;

private:
    std::vector<TAttachedControl> m_Controls;
    bool m_Dark;
    // The dark tree check boxes, one image list per DPI they've been needed at, shared by the attached trees
    // while dark. Kept until this object is destroyed, so a tree never shows a destroyed list.
    std::vector<std::pair<UINT, HIMAGELIST> > m_DarkCheckImages;
    // The menu bar whose items, and its menus', are owner-drawn while dark, and the window it belongs to.
    HMENU m_MenuBar;
    HWND m_MenuBarWindow;
    HBRUSH m_MenuBrush;
    // The font for the menus' items, for m_MenuFontDPI.
    TWin32GUIFont m_MenuFont;
    UINT m_MenuFontDPI;
    HHOOK m_MenuHook;
    std::vector<std::unique_ptr<TMenuItem> > m_MenuItems;
    TWin32GUIPalette m_Palette;
    HBRUSH m_PanelBrush;
    // Each attached tree's own check box images, put back when it turns light.
    std::vector<std::pair<HWND, HIMAGELIST> > m_ThemedCheckImages;
    HBRUSH m_WindowBrush;
    // The windows attached by AttachWindow().
    std::vector<HWND> m_Windows;

private:
    static LRESULT CALLBACK ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
        DWORD_PTR theme);
    static LRESULT CALLBACK MenuFrameProc(HWND menu, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
        DWORD_PTR theme);
    static LRESULT CALLBACK MenuHookProc(int code, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
        DWORD_PTR theme);

private:
    // Gives a bordered control (an edit box, the tree, the list, the log) its own border while dark, in place
    // of WS_EX_CLIENTEDGE's, or WS_EX_CLIENTEDGE back while light; see the definition.
    void ApplyToBorder(HWND control) const;
    // Applies the current look to one attached control.
    void ApplyToControl(TAttachedControl const& attached);
    // Makes 'menu''s items, and its submenus', owner-drawn while dark, adding each to 'items', or standard while
    // light.
    void ApplyToMenu(HMENU menu, std::vector<std::unique_ptr<TMenuItem> >& items);
    // Makes the menu bar's items and its menus' owner-drawn while dark, or standard while light.
    void ApplyToMenus();
    void ApplyToTitleBar(HWND window) const;
    void ApplyToTree(HWND tree);
    // Attaches the scroll arrows a tab control creates when its tabs don't fit, if it has created them and they
    // aren't attached yet.
    void AttachUpDown(HWND tabControl);
    // The dark tree check boxes for 'dpi', created if needed.
    HIMAGELIST DarkCheckImages(UINT dpi);
    void DrawButton(NMCUSTOMDRAW const& draw) const;
    void DrawCheckButton(NMCUSTOMDRAW const& draw, bool radio) const;
    // OnListCustomDraw()'s item prepaint while dark.
    void DrawListRow(NMLVCUSTOMDRAW const& draw, COLORREF textColor) const;
    void DrawMenuItem(DRAWITEMSTRUCT const& draw);
    // The owner-drawn menu item whose MENUITEMINFOW::dwItemData is 'itemData', or nullptr if it isn't one.
    TMenuItem const* FindMenuItem(ULONG_PTR itemData) const;
    LRESULT HandleControlMessage(HWND control, EWin32GUIThemedControl kind, UINT message, WPARAM wParam,
        LPARAM lParam);
    LRESULT HandleWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    void MeasureMenuItem(HWND window, MEASUREITEMSTRUCT& measure);
    // The font for the menus at 'dpi', created if needed.
    HFONT MenuFont(UINT dpi);
    // The custom draw of a button, radio button or check box, by its window; see AttachWindow().
    LRESULT OnButtonCustomDraw(NMCUSTOMDRAW const& draw) const;
    // A WM_CTLCOLOR* message to an attached window; false leaves it to the window.
    bool OnCtlColor(UINT message, HDC context, HWND control, LRESULT& result) const;
    LRESULT OnHeaderCustomDraw(NMCUSTOMDRAW const& draw) const;
    LRESULT OnTreeCustomDraw(NMTVCUSTOMDRAW& draw) const;
    void PaintBorder(HWND control, HDC context) const;
    void PaintMenuBarLine(HWND window) const;
    void PaintMenuFrame(HWND menu, HDC given) const;
    // A bordered control's WM_NCPAINT while dark, of 'update', its region: its scroll bars and border.
    void PaintNonClient(HWND control, HRGN update) const;
    void PaintProgressBar(HWND progressBar, HDC context) const;
    // What the dark scroll bar theme leaves light on 'control''s scroll bars (see the definition).
    void PaintScrollBars(HWND control, HDC given) const;
    void PaintStatusBar(HWND statusBar, HDC context) const;
    void PaintTabControl(HWND tabControl, HDC context) const;
    void PaintUpDown(HWND upDown, HDC context) const;
    // Forgets a control or window that's being destroyed.
    void Unattach(HWND window);

public:
    // Light, with nothing attached. Hooks this thread's menu windows.
    TWin32GUITheme();
    ~TWin32GUITheme();

    TWin32GUITheme(TWin32GUITheme const&) = delete;
    TWin32GUITheme& operator=(TWin32GUITheme const&) = delete;

    // Themes 'control', of 'kind', a child of an attached window, until it's destroyed.
    void Attach(HWND control, EWin32GUIThemedControl kind);
    // Themes 'window', a top-level window or dialog, until it's destroyed: its title bar and background, its
    // labels, edit boxes' and read-only edit boxes' colors (WM_CTLCOLOR*), its buttons, radio buttons and check
    // boxes (their NM_CUSTOMDRAW), and 'menuBar', its menu bar, if it has one.
    void AttachWindow(HWND window, HMENU menuBar = nullptr);
    // A failed test's text color on the window color (see WindowColor()).
    COLORREF FailureTextColor() const;
    bool IsDark() const;
    // An attached list view's NM_CUSTOMDRAW, for its parent to return: each row drawn whole while dark, or in its
    // default look while light, with its text in 'rowTextColor', which the item prepaint stage uses.
    LRESULT OnListCustomDraw(NMLVCUSTOMDRAW& draw, COLORREF rowTextColor) const;
    // The dark palette's colors.
    TWin32GUIPalette const& Palette() const;
    // Applies the current look again to everything attached, e.g. after a DPI change or a change of the system
    // colors.
    void Refresh();
    // Switches to the dark look or back to the light one, and redraws everything attached.
    void SetDark(bool dark);
    // A skipped test's text color on the window color (see WindowColor()).
    COLORREF SkipTextColor() const;
    // The background of edit boxes, lists and the log.
    COLORREF WindowColor() const;
    // The text color of edit boxes, lists and the log.
    COLORREF WindowTextColor() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_ThemeH
