/* **************************************************************************
ASWUnitTests_Win32GUI_Theme.cpp
Author: Anthony S. West - ASW Software

The Win32 GUI runner's light and dark looks.

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
#include "ASWUnitTests_Win32GUI_Theme.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <vsstyle.h>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Glyphs.h"
#include "ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------

// RAD Studio's Win32 (bcc32c) links its own Windows import libraries, which don't include SetWindowTheme()'s.
// CMake links uxtheme for the other compilers, and RAD Studio's Win64x links it already.
#if defined(__BORLANDC__) && !defined(_WIN64)
#  pragma comment(lib, "uxtheme.lib")
#endif

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// The theme Windows' own dark controls (scroll bars, the tree's expand arrows) are drawn from. Not documented,
// but used widely; where it's missing, the controls keep their light theme.
wchar_t const DarkExplorerTheme[] = L"DarkMode_Explorer";
// DwmSetWindowAttribute()'s attribute for a dark title bar: 20 from Windows 10 version 20H1 on, 19 before.
DWORD const DarkTitleBarAttribute = 20;
DWORD const DarkTitleBarAttributeBefore20H1 = 19;
// The theme the tree and the list have while light, as File Explorer draws them, as the VCL draws its own.
wchar_t const ExplorerTheme[] = L"Explorer";
// The window property that holds which tab is under the mouse, plus one (0 for none), while dark.
wchar_t const HotTabProperty[] = L"ASWUnitTests_Win32GUI_HotTab";
// The window property set on a control that had WS_EX_CLIENTEDGE, which it has in place of it while dark; see
// TWin32GUITheme::ApplyToBorder().
wchar_t const OwnBorderProperty[] = L"ASWUnitTests_Win32GUI_OwnBorder";
// The window property set on the tree or the list while its size changes, between WM_WINDOWPOSCHANGING and
// WM_WINDOWPOSCHANGED; see TWin32GUITheme::HandleControlMessage().
wchar_t const ResizingProperty[] = L"ASWUnitTests_Win32GUI_Resizing";
// The menu windows' class, documented as a system class.
wchar_t const MenuClassName[] = L"#32768";
// The subclass IDs: an attached control's is ControlSubclassBase plus its kind.
UINT_PTR const ControlSubclassBase = 100;
UINT_PTR const MenuFrameSubclassID = 2;
UINT_PTR const WindowSubclassID = 1;

/////////////////////////////////////////////////////////////////////////////
// TGlyphCanvas
//
// A small image built pixel by pixel, e.g. a check box from its border, its
// inside and its mark, then drawn or made into a bitmap.
/////////////////////////////////////////////////////////////////////////////
class TGlyphCanvas
{
private:
    int const m_Height;
    // Top-down 32-bit pixels, 0xAARRGGBB, fully opaque.
    std::vector<std::uint32_t> m_Pixels;
    int const m_Width;

private:
    static std::uint32_t ToPixel(COLORREF color);

private:
    BITMAPINFO BitmapInfo() const;

public:
    TGlyphCanvas(int width, int height, COLORREF background);

    // Mixes 'color' into the pixels 'glyph' covers in the square at 'left', 'top' of side 'size'.
    void Blend(EWin32GUIGlyph glyph, int left, int top, int size, COLORREF color);
    // Takes the pixels at 'left', 'top' of 'context' as the canvas's own, e.g. a gradient a glyph is then drawn
    // over. Leaves the canvas as it was if they can't be read.
    void CopyFrom(HDC context, int left, int top);
    // A bitmap of the canvas, which the caller deletes; nullptr if it can't be created.
    HBITMAP CreateBitmap() const;
    void Draw(HDC context, int left, int top) const;
    // Mixes every pixel toward 'toward' in the proportion 'numerator' / 'denominator' (see BlendColors()).
    void Fade(COLORREF toward, int numerator, int denominator);
    void FillRect(RECT const& rect, COLORREF color);
    // The pixel at 'x', 'y', or black outside the canvas.
    COLORREF PixelAt(int x, int y) const;
};

/////////////////////////////////////////////////////////////////////////////
// TPaintBuffer
//
// Paints through a bitmap the size of a rectangle, copied to the target in
// one step when this object is destroyed, so nothing painted in between is
// seen, and so nothing flickers. If the bitmap can't be created, painting
// goes straight to the target.
/////////////////////////////////////////////////////////////////////////////
class TPaintBuffer
{
private:
    HBITMAP m_Bitmap;
    HDC m_Context;
    HGDIOBJ m_OldBitmap;
    RECT const m_Rect;
    HDC const m_Target;

public:
    TPaintBuffer(HDC target, RECT const& rect);
    ~TPaintBuffer();

    TPaintBuffer(TPaintBuffer const&) = delete;
    TPaintBuffer& operator=(TPaintBuffer const&) = delete;

    // The context to paint into, in the target's coordinates.
    HDC Get() const;
};

//---------------------------------------------------------------------------

COLORREF BackgroundColor(HWND control, EWin32GUIThemedControl kind);
int BorderWidth(HWND control);
bool ClassNameIs(HWND window, wchar_t const* className);
void ExcludeRect(HRGN region, RECT const& rect);
void FillGradient(HDC context, RECT const& rect, COLORREF top, COLORREF bottom);
void FillSolid(HDC context, RECT const& rect, COLORREF color);
void FrameSolid(HDC context, RECT const& rect, COLORREF color);
bool HasThemedBorder(HWND control);
bool HidesAccelerators(HWND control);
bool HidesFocus(HWND control);
RECT Inset(RECT rect, int by);
bool IsOverDisabledScrollBar(HWND control, WPARAM hitTest);
bool MayRedrawScrollBars(UINT message);
void SetImmersiveDarkMode(HWND window, bool dark);
void SplitShortcut(std::wstring const& text, std::wstring& caption, std::wstring& shortcut);
SIZE TextSize(HDC context, std::wstring const& text, UINT format);
std::wstring WideWindowText(HWND window);

//---------------------------------------------------------------------------
/*
    BackgroundColor

    The background the tree or the list paints, by asking it: the system's window color if it has none of its
    own.
*/
COLORREF BackgroundColor(HWND control, EWin32GUIThemedControl kind)
{
    // The tree's -1 for none is CLR_NONE too, as a COLORREF.
    COLORREF const color = static_cast<COLORREF>(kind == EWin32GUIThemedControl::TreeView ?
            SendMessageW(control, TVM_GETBKCOLOR, 0, 0) : SendMessageW(control, LVM_GETBKCOLOR, 0, 0));

    return color == CLR_NONE ? GetSysColor(COLOR_WINDOW) : color;
}
//---------------------------------------------------------------------------
/*
    BorderWidth

    WS_EX_CLIENTEDGE's width at the control's DPI, which the dark look's own border has too.
*/
int BorderWidth(HWND control)
{
    return (std::max)(1, GetSystemMetricsForDpi(SM_CXEDGE, WindowDPI(control)));
}
//---------------------------------------------------------------------------
bool ClassNameIs(HWND window, wchar_t const* className)
{
    wchar_t name[64] = {};
    GetClassNameW(window, name, 64);

    return lstrcmpiW(name, className) == 0;
}
//---------------------------------------------------------------------------
void ExcludeRect(HRGN region, RECT const& rect)
{
    HRGN const excluded = CreateRectRgnIndirect(&rect);

    if (excluded == nullptr)
        return;

    CombineRgn(region, region, excluded, RGN_DIFF);
    DeleteObject(excluded);
}
//---------------------------------------------------------------------------
/*
    FillGradient

    A vertical gradient, one row at a time: 'top' at the first row and 'bottom' at the last.
*/
void FillGradient(HDC context, RECT const& rect, COLORREF top, COLORREF bottom)
{
    int const height = rect.bottom - rect.top;

    for (int y = 0; y < height; ++y)
    {
        RECT const row = { rect.left, rect.top + y, rect.right, rect.top + y + 1 };
        FillSolid(context, row, BlendColors(top, bottom, y, height - 1));
    }
}
//---------------------------------------------------------------------------
void FillSolid(HDC context, RECT const& rect, COLORREF color)
{
    COLORREF const old = SetDCBrushColor(context, color);
    FillRect(context, &rect, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    SetDCBrushColor(context, old);
}
//---------------------------------------------------------------------------
void FrameSolid(HDC context, RECT const& rect, COLORREF color)
{
    COLORREF const old = SetDCBrushColor(context, color);
    FrameRect(context, &rect, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    SetDCBrushColor(context, old);
}
//---------------------------------------------------------------------------
/*
    HasThemedBorder

    Whether the dark look paints the control's border (see TWin32GUITheme::PaintBorder()): it has one of its own
    in place of WS_EX_CLIENTEDGE, or still has WS_EX_CLIENTEDGE.
*/
bool HasThemedBorder(HWND control)
{
    return GetPropW(control, OwnBorderProperty) != nullptr ||
        (GetWindowLongPtrW(control, GWL_EXSTYLE) & WS_EX_CLIENTEDGE) != 0;
}
//---------------------------------------------------------------------------
bool HidesAccelerators(HWND control)
{
    return (SendMessageW(control, WM_QUERYUISTATE, 0, 0) & UISF_HIDEACCEL) != 0;
}
//---------------------------------------------------------------------------
bool HidesFocus(HWND control)
{
    return (SendMessageW(control, WM_QUERYUISTATE, 0, 0) & UISF_HIDEFOCUS) != 0;
}
//---------------------------------------------------------------------------
RECT Inset(RECT rect, int by)
{
    InflateRect(&rect, -by, -by);
    return rect;
}
//---------------------------------------------------------------------------
/*
    IsOverDisabledScrollBar

    Whether 'hitTest', a WM_NCHITTEST result, is the control's vertical or horizontal scroll bar, and that bar is
    disabled.
*/
bool IsOverDisabledScrollBar(HWND control, WPARAM hitTest)
{
    if (hitTest != HTVSCROLL && hitTest != HTHSCROLL)
        return false;

    SCROLLBARINFO info = {};
    info.cbSize = sizeof(info);

    return GetScrollBarInfo(control, hitTest == HTVSCROLL ? OBJID_VSCROLL : OBJID_HSCROLL, &info) != 0 &&
        (info.rgstate[0] & STATE_SYSTEM_UNAVAILABLE) != 0;
}
//---------------------------------------------------------------------------
/*
    MayRedrawScrollBars

    Whether a control with scroll bars (an edit box, the tree, the list, the log) may draw them straight away,
    rather than in WM_NCPAINT, while it handles 'message': as its text changes (an edit box enables or disables
    them), as it's resized (they may appear or go, and with them the corner between them), and as the mouse
    enters or leaves it or its scroll bars (a themed scroll bar's arrows show whether the mouse is over the
    window).
*/
bool MayRedrawScrollBars(UINT message)
{
    switch (message)
    {
        case EM_REPLACESEL:
        case WM_CAPTURECHANGED:
        case WM_ENABLE:
        case WM_KILLFOCUS:
        case WM_MOUSELEAVE:
        case WM_MOUSEMOVE:
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCMOUSELEAVE:
        case WM_NCMOUSEMOVE:
        case WM_SETFOCUS:
        case WM_SETTEXT:
        case WM_SIZE:
        case WM_WINDOWPOSCHANGED:
            return true;

        default:
            return false;
    }
}
//---------------------------------------------------------------------------
/*
    SetImmersiveDarkMode

    DwmSetWindowAttribute() is loaded from dwmapi.dll as it's needed, rather than linked, so the GUI needs no
    import library of it (RAD Studio's Win32 and Win64x have different ones, and CMake would need it listed for
    every project that builds the GUI). Windows before 10 version 1809 has neither attribute, and keeps the
    title bar light.
*/
void SetImmersiveDarkMode(HWND window, bool dark)
{
    using TDwmSetWindowAttribute = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);

    static HMODULE const library = LoadLibraryW(L"dwmapi.dll");
    static TDwmSetWindowAttribute const setAttribute = library == nullptr ? nullptr :
            reinterpret_cast<TDwmSetWindowAttribute>(reinterpret_cast<void*>(GetProcAddress(library,
                "DwmSetWindowAttribute")));

    if (setAttribute == nullptr)
        return;

    BOOL const value = dark ? TRUE : FALSE;

    if (FAILED(setAttribute(window, DarkTitleBarAttribute, &value, sizeof(value))))
        setAttribute(window, DarkTitleBarAttributeBefore20H1, &value, sizeof(value));
}
//---------------------------------------------------------------------------
/*
    SplitShortcut

    A menu item's text, e.g. "&Run Selected\tF9", as its caption and its shortcut, which is empty if it has
    none.
*/
void SplitShortcut(std::wstring const& text, std::wstring& caption, std::wstring& shortcut)
{
    std::size_t const tab = text.find(L'\t');
    caption = text.substr(0, tab);
    shortcut = tab == std::wstring::npos ? std::wstring() : text.substr(tab + 1);
}
//---------------------------------------------------------------------------
SIZE TextSize(HDC context, std::wstring const& text, UINT format)
{
    RECT bounds = { 0, 0, 0, 0 };
    DrawTextW(context, text.c_str(), static_cast<int>(text.size()), &bounds, format | DT_CALCRECT | DT_SINGLELINE);

    return { bounds.right - bounds.left, bounds.bottom - bounds.top };
}
//---------------------------------------------------------------------------
std::wstring WideWindowText(HWND window)
{
    std::wstring text(static_cast<std::size_t>(GetWindowTextLengthW(window)) + 1, L'\0');
    text.resize(static_cast<std::size_t>(GetWindowTextW(window, &text[0], static_cast<int>(text.size()))));

    return text;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TGlyphCanvas
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TGlyphCanvas::TGlyphCanvas(int width, int height, COLORREF background)
    : m_Height((std::max)(height, 1)),
      m_Pixels(static_cast<std::size_t>((std::max)(width, 1)) * static_cast<std::size_t>((std::max)(height, 1)),
          ToPixel(background)),
      m_Width((std::max)(width, 1))
{
}
//---------------------------------------------------------------------------
BITMAPINFO TGlyphCanvas::BitmapInfo() const
{
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = m_Width;
    info.bmiHeader.biHeight = -m_Height; // Top-down.
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    return info;
}
//---------------------------------------------------------------------------
void TGlyphCanvas::Blend(EWin32GUIGlyph glyph, int left, int top, int size, COLORREF color)
{
    std::vector<std::uint8_t> const coverage = RenderWin32GUIGlyph(glyph, size);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int const canvasX = left + x;
            int const canvasY = top + y;
            std::uint8_t const amount = coverage[static_cast<std::size_t>(y) * static_cast<std::size_t>(size) +
                                                 static_cast<std::size_t>(x)];

            if (amount == 0 || canvasX < 0 || canvasY < 0 || canvasX >= m_Width || canvasY >= m_Height)
                continue;

            std::uint32_t& pixel = m_Pixels[static_cast<std::size_t>(canvasY) * static_cast<std::size_t>(m_Width) +
                                            static_cast<std::size_t>(canvasX)];
            COLORREF const under = RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
            pixel = ToPixel(BlendColors(under, color, amount, 255));
        }
    }
}
//---------------------------------------------------------------------------
void TGlyphCanvas::CopyFrom(HDC context, int left, int top)
{
    BITMAPINFO const info = BitmapInfo();
    void* bits = nullptr;
    HBITMAP const bitmap = CreateDIBSection(context, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    HDC const copy = CreateCompatibleDC(context);

    if (bitmap != nullptr && bits != nullptr && copy != nullptr)
    {
        HGDIOBJ const old = SelectObject(copy, bitmap);

        if (BitBlt(copy, 0, 0, m_Width, m_Height, context, left, top, SRCCOPY) != 0)
        {
            GdiFlush();
            std::uint32_t const* const pixels = static_cast<std::uint32_t const*>(bits);

            for (std::size_t i = 0; i < m_Pixels.size(); ++i)
                m_Pixels[i] = 0xFF000000u | pixels[i];
        }

        SelectObject(copy, old);
    }

    if (copy != nullptr)
        DeleteDC(copy);

    if (bitmap != nullptr)
        DeleteObject(bitmap);
}
//---------------------------------------------------------------------------
HBITMAP TGlyphCanvas::CreateBitmap() const
{
    BITMAPINFO const info = BitmapInfo();
    void* bits = nullptr;
    HBITMAP const bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);

    if (bitmap != nullptr && bits != nullptr)
        std::copy(m_Pixels.begin(), m_Pixels.end(), static_cast<std::uint32_t*>(bits));

    return bitmap;
}
//---------------------------------------------------------------------------
void TGlyphCanvas::Draw(HDC context, int left, int top) const
{
    BITMAPINFO const info = BitmapInfo();
    SetDIBitsToDevice(context, left, top, static_cast<DWORD>(m_Width), static_cast<DWORD>(m_Height), 0, 0, 0,
        static_cast<UINT>(m_Height), m_Pixels.data(), &info, DIB_RGB_COLORS);
}
//---------------------------------------------------------------------------
void TGlyphCanvas::Fade(COLORREF toward, int numerator, int denominator)
{
    for (std::uint32_t& pixel : m_Pixels)
    {
        COLORREF const color = RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
        pixel = ToPixel(BlendColors(color, toward, numerator, denominator));
    }
}
//---------------------------------------------------------------------------
void TGlyphCanvas::FillRect(RECT const& rect, COLORREF color)
{
    std::uint32_t const pixel = ToPixel(color);

    for (int y = (std::max)(0, static_cast<int>(rect.top)); y < (std::min)(m_Height, static_cast<int>(rect.bottom));
         ++y)
    {
        for (int x = (std::max)(0, static_cast<int>(rect.left)); x < (std::min)(m_Width, static_cast<int>(rect.right));
             ++x)
        {
            m_Pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_Width) + static_cast<std::size_t>(x)] =
                pixel;
        }
    }
}
//---------------------------------------------------------------------------
COLORREF TGlyphCanvas::PixelAt(int x, int y) const
{
    if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
        return RGB(0, 0, 0);

    std::size_t const row = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_Width);
    std::uint32_t const pixel = m_Pixels[row + static_cast<std::size_t>(x)];
    return RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
}
//---------------------------------------------------------------------------
std::uint32_t TGlyphCanvas::ToPixel(COLORREF color)
{
    return 0xFF000000u | (static_cast<std::uint32_t>(GetRValue(color)) << 16) |
        (static_cast<std::uint32_t>(GetGValue(color)) << 8) | static_cast<std::uint32_t>(GetBValue(color));
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TPaintBuffer
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TPaintBuffer::TPaintBuffer(HDC target, RECT const& rect)
    : m_Bitmap(nullptr),
      m_Context(nullptr),
      m_OldBitmap(nullptr),
      m_Rect(rect),
      m_Target(target)
{
    int const width = rect.right - rect.left;
    int const height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0)
        return;

    m_Context = CreateCompatibleDC(target);
    m_Bitmap = m_Context != nullptr ? CreateCompatibleBitmap(target, width, height) : nullptr;

    if (m_Bitmap == nullptr)
    {
        if (m_Context != nullptr)
            DeleteDC(m_Context);

        m_Context = nullptr;
        return;
    }

    m_OldBitmap = SelectObject(m_Context, m_Bitmap);
    SetViewportOrgEx(m_Context, -rect.left, -rect.top, nullptr);
}
//---------------------------------------------------------------------------
TPaintBuffer::~TPaintBuffer()
{
    if (m_Context == nullptr)
        return;

    SetViewportOrgEx(m_Context, 0, 0, nullptr);
    BitBlt(m_Target, m_Rect.left, m_Rect.top, m_Rect.right - m_Rect.left, m_Rect.bottom - m_Rect.top, m_Context, 0, 0,
        SRCCOPY);
    SelectObject(m_Context, m_OldBitmap);
    DeleteObject(m_Bitmap);
    DeleteDC(m_Context);
}
//---------------------------------------------------------------------------
HDC TPaintBuffer::Get() const
{
    return m_Context != nullptr ? m_Context : m_Target;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWin32GUITheme
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWin32GUITheme* TWin32GUITheme::s_Instance = nullptr;

//---------------------------------------------------------------------------
TWin32GUITheme::TWin32GUITheme()
    : m_Dark(false),
      m_MenuBar(nullptr),
      m_MenuBarWindow(nullptr),
      m_MenuBrush(nullptr),
      m_MenuFontDPI(0),
      m_MenuHook(nullptr),
      m_Palette(CarbonPalette()),
      m_PanelBrush(nullptr),
      m_WindowBrush(nullptr)
{
    m_MenuBrush = CreateSolidBrush(m_Palette.MenuBackground);
    m_PanelBrush = CreateSolidBrush(m_Palette.Panel);
    m_WindowBrush = CreateSolidBrush(m_Palette.Window);

    // Without the hook, the menus' frames stay light.
    s_Instance = this;
    m_MenuHook = SetWindowsHookExW(WH_CALLWNDPROC, &MenuHookProc, nullptr, GetCurrentThreadId());
}
//---------------------------------------------------------------------------
TWin32GUITheme::~TWin32GUITheme()
{
    if (m_MenuHook != nullptr)
        UnhookWindowsHookEx(m_MenuHook);

    if (s_Instance == this)
        s_Instance = nullptr;

    for (std::pair<HWND, HIMAGELIST> const& themed : m_ThemedCheckImages)
        ImageList_Destroy(themed.second);

    for (std::pair<UINT, HIMAGELIST> const& dark : m_DarkCheckImages)
        ImageList_Destroy(dark.second);

    for (HBRUSH const brush : { m_MenuBrush, m_PanelBrush, m_WindowBrush })
    {
        if (brush != nullptr)
            DeleteObject(brush);
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::ApplyToBorder

    A control with WS_EX_CLIENTEDGE paints that border itself, in a light color, whatever its theme, in each
    WM_NCPAINT; painting a dark one over it afterwards shows the light one for a moment, e.g. while a splitter
    is dragged. So while dark it has no WS_EX_CLIENTEDGE: WM_NCCALCSIZE keeps the same width for a border of
    its own, around its scroll bars, which WM_NCPAINT paints (see HandleControlMessage()). Light puts
    WS_EX_CLIENTEDGE back.
*/
void TWin32GUITheme::ApplyToBorder(HWND control) const
{
    LONG_PTR const exStyle = GetWindowLongPtrW(control, GWL_EXSTYLE);
    bool const ownBorder = GetPropW(control, OwnBorderProperty) != nullptr;

    if (m_Dark && (exStyle & WS_EX_CLIENTEDGE) != 0)
    {
        SetPropW(control, OwnBorderProperty, reinterpret_cast<HANDLE>(static_cast<INT_PTR>(1)));
        SetWindowLongPtrW(control, GWL_EXSTYLE, exStyle & ~static_cast<LONG_PTR>(WS_EX_CLIENTEDGE));
    }
    else if (!m_Dark && ownBorder)
    {
        RemovePropW(control, OwnBorderProperty);
        SetWindowLongPtrW(control, GWL_EXSTYLE, exStyle | WS_EX_CLIENTEDGE);
    }
    else if (!ownBorder)
    {
        return;
    }

    // Its non-client area is measured again, also for a new DPI while dark.
    SetWindowPos(control, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
        SWP_FRAMECHANGED);
}
//---------------------------------------------------------------------------
void TWin32GUITheme::ApplyToControl(TAttachedControl const& attached)
{
    HWND const control = attached.Control;
    wchar_t const* const darkOrDefault = m_Dark ? DarkExplorerTheme : nullptr;

    switch (attached.Kind)
    {
        case EWin32GUIThemedControl::Edit:
            // Its scroll bars; its colors come from WM_CTLCOLOREDIT and WM_CTLCOLORSTATIC.
            SetWindowTheme(control, darkOrDefault, nullptr);
            break;

        case EWin32GUIThemedControl::ListView:
        {
            SetWindowTheme(control, m_Dark ? DarkExplorerTheme : ExplorerTheme, nullptr);
            HWND const header = reinterpret_cast<HWND>(SendMessageW(control, LVM_GETHEADER, 0, 0));

            if (header != nullptr)
            {
                SetWindowTheme(header, m_Dark ? L"DarkMode_ItemsView" : nullptr, nullptr);
            }

            COLORREF const background = WindowColor();
            SendMessageW(control, LVM_SETBKCOLOR, 0, static_cast<LPARAM>(background));
            SendMessageW(control, LVM_SETTEXTBKCOLOR, 0, static_cast<LPARAM>(background));
            SendMessageW(control, LVM_SETTEXTCOLOR, 0, static_cast<LPARAM>(WindowTextColor()));
            break;
        }

        case EWin32GUIThemedControl::RichEdit:
            SetWindowTheme(control, darkOrDefault, nullptr);
            SendMessageW(control, EM_SETBKGNDCOLOR, m_Dark ? FALSE : TRUE, static_cast<LPARAM>(m_Palette.Window));
            break;

        case EWin32GUIThemedControl::Tooltip:
            if (m_Dark)
            {
                // Without visual styles, so it takes the colors given.
                SetWindowTheme(control, L" ", L" ");
                SendMessageW(control, TTM_SETTIPBKCOLOR, static_cast<WPARAM>(m_Palette.TooltipBackground), 0);
                SendMessageW(control, TTM_SETTIPTEXTCOLOR, static_cast<WPARAM>(m_Palette.TooltipText), 0);
            }
            else
            {
                SetWindowTheme(control, nullptr, nullptr);
                SendMessageW(control, TTM_SETTIPBKCOLOR, static_cast<WPARAM>(GetSysColor(COLOR_INFOBK)), 0);
                SendMessageW(control, TTM_SETTIPTEXTCOLOR, static_cast<WPARAM>(GetSysColor(COLOR_INFOTEXT)), 0);
            }

            return; // A tooltip is drawn when it next shows.

        case EWin32GUIThemedControl::TreeView:
            ApplyToTree(control);
            break;

        case EWin32GUIThemedControl::ProgressBar:
        case EWin32GUIThemedControl::Splitter:
        case EWin32GUIThemedControl::StatusBar:
        case EWin32GUIThemedControl::TabControl:
        case EWin32GUIThemedControl::UpDown:
            break; // Painted by ControlProc() while dark.
    }

    if (attached.Kind == EWin32GUIThemedControl::Edit || attached.Kind == EWin32GUIThemedControl::ListView ||
        attached.Kind == EWin32GUIThemedControl::RichEdit || attached.Kind == EWin32GUIThemedControl::TreeView)
    {
        ApplyToBorder(control);
    }

    RedrawWindow(control, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}
//---------------------------------------------------------------------------
void TWin32GUITheme::ApplyToMenu(HMENU menu, std::vector<std::unique_ptr<TMenuItem> >& items)
{
    int const count = GetMenuItemCount(menu);

    for (int position = 0; position < count; ++position)
    {
        wchar_t text[256] = {};
        MENUITEMINFOW info = {};
        info.cbSize = sizeof(info);
        info.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
        info.dwTypeData = text;
        info.cch = 256;

        if (GetMenuItemInfoW(menu, static_cast<UINT>(position), TRUE, &info) == 0)
            continue;

        // An owner-drawn item's string can't be read back reliably, so it's taken from its item.
        TMenuItem const* const current = FindMenuItem(info.dwItemData);

        if (current != nullptr)
            text[current->Text.copy(text, 255)] = L'\0';

        // Its string is set again too, which an owner-drawn item may have lost.
        MENUITEMINFOW change = {};
        change.cbSize = sizeof(change);
        change.fMask = MIIM_FTYPE | MIIM_DATA | ((info.fType & MFT_SEPARATOR) != 0 ? 0 : MIIM_STRING);
        change.fType = info.fType & ~static_cast<UINT>(MFT_OWNERDRAW);
        change.dwTypeData = text;

        if (m_Dark)
        {
            std::unique_ptr<TMenuItem> item = std::make_unique<TMenuItem>();
            item->Menu = menu;
            item->Position = static_cast<UINT>(position);
            item->Text = text;
            item->Separator = (info.fType & MFT_SEPARATOR) != 0;
            item->Submenu = info.hSubMenu != nullptr;
            item->TopLevel = menu == m_MenuBar;

            // Its string stays, so Windows still finds its & letter.
            change.fType |= MFT_OWNERDRAW;
            change.dwItemData = reinterpret_cast<ULONG_PTR>(item.get());
            items.push_back(std::move(item));
        }

        if (menu == m_MenuBar)
        {
            // A menu bar item keeps the width it was first measured at, whatever changes, e.g. an owner-drawn
            // width, too narrow for the standard look, so it's replaced by a new one. A menu measures its items
            // each time it opens.
            change.fMask |= MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
            change.wID = info.wID;
            change.fState = info.fState;
            change.hSubMenu = info.hSubMenu;
            RemoveMenu(menu, static_cast<UINT>(position), MF_BYPOSITION);
            InsertMenuItemW(menu, static_cast<UINT>(position), TRUE, &change);
        }
        else
        {
            SetMenuItemInfoW(menu, static_cast<UINT>(position), TRUE, &change);
        }

        if (info.hSubMenu != nullptr)
        {
            MENUINFO background = {};
            background.cbSize = sizeof(background);
            background.fMask = MIM_BACKGROUND;
            background.hbrBack = m_Dark ? m_MenuBrush : nullptr;
            SetMenuInfo(info.hSubMenu, &background);

            ApplyToMenu(info.hSubMenu, items);
        }
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::ApplyToMenus

    The new items replace the old ones only once every menu item points to its new one.
*/
void TWin32GUITheme::ApplyToMenus()
{
    if (m_MenuBar == nullptr)
        return;

    std::vector<std::unique_ptr<TMenuItem> > items;
    ApplyToMenu(m_MenuBar, items);
    m_MenuItems.swap(items);

    // The menu bar's empty part, right of its items.
    MENUINFO background = {};
    background.cbSize = sizeof(background);
    background.fMask = MIM_BACKGROUND;
    background.hbrBack = m_Dark ? m_PanelBrush : nullptr;
    SetMenuInfo(m_MenuBar, &background);

    DrawMenuBar(m_MenuBarWindow);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::ApplyToTitleBar

    A visible window's frame is redrawn, so a live switch shows on its title bar too.
*/
void TWin32GUITheme::ApplyToTitleBar(HWND window) const
{
    SetImmersiveDarkMode(window, m_Dark);

    if (IsWindowVisible(window) != FALSE)
    {
        SetWindowPos(window, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
            SWP_FRAMECHANGED);
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::ApplyToTree

    A tree with check boxes shows Carbon's while dark, keeping its own to give back when it turns light. Its own
    are given back before its theme changes, and replaced after, since a theme change redraws the images of the
    list it has.
*/
void TWin32GUITheme::ApplyToTree(HWND tree)
{
    bool const checkBoxes = (GetWindowLongPtrW(tree, GWL_STYLE) & TVS_CHECKBOXES) != 0;
    std::vector<std::pair<HWND, HIMAGELIST> >::iterator const themed = std::find_if(m_ThemedCheckImages.begin(),
        m_ThemedCheckImages.end(), [tree](std::pair<HWND, HIMAGELIST> const& entry)
        {
            return entry.first == tree;
        });

    if (!m_Dark && themed != m_ThemedCheckImages.end())
    {
        SendMessageW(tree, TVM_SETIMAGELIST, TVSIL_STATE, reinterpret_cast<LPARAM>(themed->second));
        m_ThemedCheckImages.erase(themed);
    }

    SetWindowTheme(tree, m_Dark ? DarkExplorerTheme : ExplorerTheme, nullptr);
    SendMessageW(tree, TVM_SETBKCOLOR, 0, m_Dark ? static_cast<LPARAM>(m_Palette.Window) : -1);
    SendMessageW(tree, TVM_SETTEXTCOLOR, 0, m_Dark ? static_cast<LPARAM>(m_Palette.WindowText) : -1);
    SendMessageW(tree, TVM_SETLINECOLOR, 0, m_Dark ? static_cast<LPARAM>(m_Palette.TreeLine) :
            static_cast<LPARAM>(CLR_DEFAULT));

    if (m_Dark && checkBoxes)
    {
        HIMAGELIST const previous = reinterpret_cast<HIMAGELIST>(SendMessageW(tree, TVM_SETIMAGELIST, TVSIL_STATE,
            reinterpret_cast<LPARAM>(DarkCheckImages(WindowDPI(tree)))));
        bool const previousIsDark = std::any_of(m_DarkCheckImages.begin(), m_DarkCheckImages.end(),
            [previous](std::pair<UINT, HIMAGELIST> const& entry)
            {
                return entry.second == previous;
            });

        if (previous != nullptr && !previousIsDark)
            m_ThemedCheckImages.emplace_back(tree, previous);
    }
}
//---------------------------------------------------------------------------
void TWin32GUITheme::Attach(HWND control, EWin32GUIThemedControl kind)
{
    if (control == nullptr)
        return;

    m_Controls.push_back({ control, kind });
    SetWindowSubclass(control, &ControlProc, ControlSubclassBase + static_cast<UINT_PTR>(kind),
        reinterpret_cast<DWORD_PTR>(this));
    ApplyToControl(m_Controls.back());

    if (kind == EWin32GUIThemedControl::TabControl)
        AttachUpDown(control);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::AttachUpDown

    A tab control creates its scroll arrows, a child up-down control, the first time its tabs don't fit, and
    keeps them, hidden, once they fit again. It's looked for when the tab control is attached, created
    (WM_PARENTNOTIFY) and resized, in case it doesn't notify.
*/
void TWin32GUITheme::AttachUpDown(HWND tabControl)
{
    HWND const upDown = FindWindowExW(tabControl, nullptr, UPDOWN_CLASSW, nullptr);

    if (upDown == nullptr)
        return;

    bool const attached = std::any_of(m_Controls.begin(), m_Controls.end(), [upDown](TAttachedControl const& entry)
        {
            return entry.Control == upDown;
        });

    if (!attached)
        Attach(upDown, EWin32GUIThemedControl::UpDown);
}
//---------------------------------------------------------------------------
void TWin32GUITheme::AttachWindow(HWND window, HMENU menuBar)
{
    if (window == nullptr)
        return;

    m_Windows.push_back(window);
    SetWindowSubclass(window, &WindowProc, WindowSubclassID, reinterpret_cast<DWORD_PTR>(this));

    if (menuBar != nullptr)
    {
        m_MenuBar = menuBar;
        m_MenuBarWindow = window;
        ApplyToMenus();
    }

    ApplyToTitleBar(window);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::ControlProc

    An attached control's subclass. No exception may leave a window procedure; one that would leaves the
    message to the control.
*/
LRESULT CALLBACK TWin32GUITheme::ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
    DWORD_PTR theme)
{
    try
    {
        EWin32GUIThemedControl const kind = static_cast<EWin32GUIThemedControl>(id - ControlSubclassBase);

        if (message == WM_NCDESTROY)
        {
            reinterpret_cast<TWin32GUITheme*>(theme)->Unattach(control);
            RemovePropW(control, HotTabProperty);
            RemovePropW(control, OwnBorderProperty);
            RemovePropW(control, ResizingProperty);
            RemoveWindowSubclass(control, &ControlProc, id);
            return DefSubclassProc(control, message, wParam, lParam);
        }

        return reinterpret_cast<TWin32GUITheme*>(theme)->HandleControlMessage(control, kind, message, wParam, lParam);
    }
    catch (...)
    {
        return DefSubclassProc(control, message, wParam, lParam);
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::CreateDarkCheckImage

    Carbon's check box, 13 pixels at 96 DPI, in the middle of the tree's 16-pixel state image.
*/
HBITMAP TWin32GUITheme::CreateDarkCheckImage(int state, UINT dpi) const
{
    int const cell = ScaleForDPI(16, dpi);
    int const box = ScaleForDPI(13, dpi);
    int const offset = (cell - box) / 2;
    int const border = (std::max)(1, ScaleForDPI(1, dpi));

    TGlyphCanvas canvas(cell, cell, m_Palette.Window);

    if (state > 0)
    {
        RECT const outside = { offset, offset, offset + box, offset + box };
        canvas.FillRect(outside, m_Palette.CheckBoxBorder);
        canvas.FillRect(Inset(outside, border), m_Palette.CheckBoxFill);

        if (state == 2)
            canvas.Blend(EWin32GUIGlyph::Cross, offset, offset, box, m_Palette.CheckMark);
        else if (state == 3)
            canvas.Blend(EWin32GUIGlyph::Square, offset, offset, box, m_Palette.CheckMark);
    }

    return canvas.CreateBitmap();
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::DarkCheckImages

    In the order TVS_EX_PARTIALCHECKBOXES numbers them: none, unchecked, checked (a cross), partial (a square).
*/
HIMAGELIST TWin32GUITheme::DarkCheckImages(UINT dpi)
{
    for (std::pair<UINT, HIMAGELIST> const& entry : m_DarkCheckImages)
    {
        if (entry.first == dpi)
            return entry.second;
    }

    int const cell = ScaleForDPI(16, dpi);
    HIMAGELIST const images = ImageList_Create(cell, cell, ILC_COLOR32, 4, 0);

    if (images == nullptr)
        return nullptr;

    for (int state = 0; state < 4; ++state)
    {
        HBITMAP const bitmap = CreateDarkCheckImage(state, dpi);

        if (bitmap != nullptr)
        {
            ImageList_Add(images, bitmap, nullptr);
            DeleteObject(bitmap);
        }
    }

    m_DarkCheckImages.emplace_back(dpi, images);
    return images;
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::DrawButton

    As Carbon draws a push button: an outline, a border, and a face shaded from top to bottom, its border lighter
    under the mouse, and with a line inside it for the default button or the focused one, which is how the
    focus shows, as in the VCL GUI runner.
*/
void TWin32GUITheme::DrawButton(NMCUSTOMDRAW const& draw) const
{
    HWND const button = draw.hdr.hwndFrom;
    HDC const context = draw.hdc;
    UINT const dpi = WindowDPI(button);
    int const line = (std::max)(1, ScaleForDPI(1, dpi));

    bool const disabled = (draw.uItemState & CDIS_DISABLED) != 0;
    bool const hot = (draw.uItemState & CDIS_HOT) != 0;
    bool const pressed = (draw.uItemState & CDIS_SELECTED) != 0;
    bool const emphasized = !disabled && ((draw.uItemState & (CDIS_DEFAULT | CDIS_FOCUS)) != 0 ||
        (GetWindowLongPtrW(button, GWL_STYLE) & BS_TYPEMASK) == BS_DEFPUSHBUTTON);

    RECT const outline = draw.rc;
    RECT const border = Inset(outline, line);
    RECT const face = Inset(border, line);

    FillSolid(context, outline, disabled ? m_Palette.ButtonDisabledOutline : m_Palette.ButtonOutline);
    FillSolid(context, border, disabled ? m_Palette.ButtonDisabledBorder : hot || emphasized ?
                m_Palette.ButtonHotBorder : m_Palette.ButtonBorder);

    if (disabled)
        FillGradient(context, face, m_Palette.ButtonDisabledFaceTop, m_Palette.ButtonDisabledFaceBottom);
    else if (pressed)
        FillGradient(context, face, m_Palette.ButtonPressedFaceTop, m_Palette.ButtonPressedFaceBottom);
    else if (hot)
        FillGradient(context, face, m_Palette.ButtonHotFaceTop, m_Palette.ButtonHotFaceBottom);
    else
        FillGradient(context, face, m_Palette.ButtonFaceTop, m_Palette.ButtonFaceBottom);

    if (emphasized)
        FrameSolid(context, face, m_Palette.ButtonDefaultFrame);

    std::wstring const text = WideWindowText(button);
    RECT textRect = face;
    int const oldMode = SetBkMode(context, TRANSPARENT);
    COLORREF const oldColor = SetTextColor(context, disabled ? m_Palette.DisabledText : hot || pressed ?
                m_Palette.HotText : m_Palette.ControlText);
    DrawTextW(context, text.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE |
        (HidesAccelerators(button) ? DT_HIDEPREFIX : 0));
    SetTextColor(context, oldColor);
    SetBkMode(context, oldMode);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::DrawCheckButton

    As Carbon draws a radio button or a check box: its 13-pixel circle or box at the left, middle, and its text
    after it, with a focus rectangle around the text when it has the focus and the keyboard is being used.
*/
void TWin32GUITheme::DrawCheckButton(NMCUSTOMDRAW const& draw, bool radio) const
{
    HWND const button = draw.hdr.hwndFrom;
    HDC const context = draw.hdc;
    UINT const dpi = WindowDPI(button);
    RECT const rect = draw.rc;

    bool const disabled = (draw.uItemState & CDIS_DISABLED) != 0;
    bool const hot = (draw.uItemState & CDIS_HOT) != 0 || (draw.uItemState & CDIS_SELECTED) != 0;
    LRESULT const check = SendMessageW(button, BM_GETCHECK, 0, 0);

    FillSolid(context, rect, m_Palette.Panel);

    int const box = ScaleForDPI(13, dpi);
    int const top = (rect.top + rect.bottom - box) / 2;
    COLORREF const borderColor = hot && !disabled ? m_Palette.CheckBoxHotBorder : m_Palette.CheckBoxBorder;
    COLORREF const markColor = disabled ? m_Palette.CheckMarkDisabled : m_Palette.CheckMark;
    TGlyphCanvas canvas(box, box, m_Palette.Panel);

    if (radio)
    {
        canvas.Blend(EWin32GUIGlyph::Circle, 0, 0, box, borderColor);
        canvas.Blend(EWin32GUIGlyph::InnerCircle, 0, 0, box, m_Palette.CheckBoxFill);

        if (check == BST_CHECKED)
            canvas.Blend(EWin32GUIGlyph::Dot, 0, 0, box, markColor);
    }
    else
    {
        RECT const outside = { 0, 0, box, box };
        canvas.FillRect(outside, borderColor);
        canvas.FillRect(Inset(outside, (std::max)(1, ScaleForDPI(1, dpi))), m_Palette.CheckBoxFill);

        if (check == BST_CHECKED)
            canvas.Blend(EWin32GUIGlyph::Cross, 0, 0, box, markColor);
        else if (check == BST_INDETERMINATE)
            canvas.Blend(EWin32GUIGlyph::Square, 0, 0, box, markColor);
    }

    canvas.Draw(context, rect.left, top);

    std::wstring const text = WideWindowText(button);
    UINT const format = DT_LEFT | DT_VCENTER | DT_SINGLELINE | (HidesAccelerators(button) ? DT_HIDEPREFIX : 0);
    RECT textRect = { rect.left + box + ScaleForDPI(4, dpi), rect.top, rect.right, rect.bottom };
    int const oldMode = SetBkMode(context, TRANSPARENT);
    COLORREF const oldColor = SetTextColor(context, disabled ? m_Palette.DisabledText : m_Palette.ControlText);
    DrawTextW(context, text.c_str(), -1, &textRect, format);
    SetTextColor(context, oldColor);
    SetBkMode(context, oldMode);

    if ((draw.uItemState & CDIS_FOCUS) != 0 && !HidesFocus(button))
    {
        SIZE const size = TextSize(context, text, format & ~static_cast<UINT>(DT_VCENTER));
        RECT focus = { textRect.left - 1, (rect.top + rect.bottom - size.cy) / 2 - 1, textRect.left + size.cx + 1,
                       (rect.top + rect.bottom + size.cy) / 2 + 1 };
        DrawFocusRect(context, &focus);
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::DrawListRow

    The row's background, its selection as the tree shows one, then each column's text, clipped with an
    ellipsis, and a focus rectangle around the focused row while the list has the focus and the keyboard is
    being used.
*/
void TWin32GUITheme::DrawListRow(NMLVCUSTOMDRAW const& draw, COLORREF textColor) const
{
    HWND const list = draw.nmcd.hdr.hwndFrom;
    HDC const context = draw.nmcd.hdc;
    UINT const dpi = WindowDPI(list);
    WPARAM const index = static_cast<WPARAM>(draw.nmcd.dwItemSpec);
    UINT const state = static_cast<UINT>(SendMessageW(list, LVM_GETITEMSTATE, index, LVIS_SELECTED | LVIS_FOCUSED));

    RECT row = {};
    row.left = LVIR_BOUNDS;
    SendMessageW(list, LVM_GETITEMRECT, index, reinterpret_cast<LPARAM>(&row));

    if ((state & LVIS_SELECTED) != 0)
    {
        FillSolid(context, row, m_Palette.Selection);
        FrameSolid(context, row, m_Palette.SelectionFrame);
    }
    else
    {
        FillSolid(context, row, m_Palette.Window);
    }

    int const oldMode = SetBkMode(context, TRANSPARENT);
    COLORREF const oldColor = SetTextColor(context, textColor);
    HWND const header = reinterpret_cast<HWND>(SendMessageW(list, LVM_GETHEADER, 0, 0));
    int const columns = header != nullptr ? static_cast<int>(SendMessageW(header, HDM_GETITEMCOUNT, 0, 0)) : 1;

    for (int column = 0; column < columns; ++column)
    {
        RECT cell = {};

        if (column == 0)
        {
            cell.left = LVIR_LABEL;
            SendMessageW(list, LVM_GETITEMRECT, index, reinterpret_cast<LPARAM>(&cell));
        }
        else
        {
            cell.top = column;
            cell.left = LVIR_BOUNDS;
            SendMessageW(list, LVM_GETSUBITEMRECT, index, reinterpret_cast<LPARAM>(&cell));
        }

        std::wstring text(1024, L'\0');
        LVITEMW item = {};
        item.iSubItem = column;
        item.pszText = &text[0];
        item.cchTextMax = static_cast<int>(text.size());
        text.resize(static_cast<std::size_t>(SendMessageW(list, LVM_GETITEMTEXTW, index,
            reinterpret_cast<LPARAM>(&item))));

        // As the list's own text is placed: 2 pixels into the first column's label, 6 into the others.
        cell.left += ScaleForDPI(column == 0 ? 2 : 6, dpi);
        cell.right -= ScaleForDPI(6, dpi);
        DrawTextW(context, text.c_str(), static_cast<int>(text.size()), &cell, DT_LEFT | DT_VCENTER | DT_SINGLELINE |
            DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    SetTextColor(context, oldColor);
    SetBkMode(context, oldMode);

    if ((state & LVIS_FOCUSED) != 0 && GetFocus() == list && !HidesFocus(list))
    {
        RECT focus = Inset(row, 1);
        DrawFocusRect(context, &focus);
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::DrawMenuItem

    As Carbon draws the menus: the item under the mouse or the keyboard in the highlight, with a frame, unless
    it's disabled; a separator as a line; a menu item's caption after a gutter for its check mark (a tick, or a
    dot for a radio item), its shortcut at the right, and a submenu's arrow. The & letters are underlined only
    once the keyboard has been used (ODS_NOACCEL). Windows draws its own submenu arrow after WM_DRAWITEM,
    unless the item is clipped out of the context.
*/
void TWin32GUITheme::DrawMenuItem(DRAWITEMSTRUCT const& draw)
{
    TMenuItem const* const item = FindMenuItem(draw.itemData);

    if (item == nullptr)
        return;

    HDC const context = draw.hDC;
    RECT const rect = draw.rcItem;
    UINT const dpi = m_MenuBarWindow != nullptr ? WindowDPI(m_MenuBarWindow) : USER_DEFAULT_SCREEN_DPI;
    bool const disabled = (draw.itemState & (ODS_DISABLED | ODS_GRAYED)) != 0;
    // As Carbon: a disabled item under the mouse or the keyboard looks as it does otherwise.
    bool const selected = !disabled && (draw.itemState & (ODS_SELECTED | ODS_HOTLIGHT)) != 0;
    COLORREF const background = selected ? m_Palette.Highlight : item->TopLevel ? m_Palette.Panel :
                m_Palette.MenuBackground;

    FillSolid(context, rect, background);

    if (selected)
        FrameSolid(context, rect, m_Palette.HighlightFrame);

    if (item->Separator)
    {
        int const middle = (rect.top + rect.bottom) / 2;
        RECT const separator = { rect.left + ScaleForDPI(4, dpi), middle, rect.right - ScaleForDPI(4, dpi), middle +
                                 (std::max)(1, ScaleForDPI(1, dpi)) };
        FillSolid(context, separator, m_Palette.MenuSeparator);
        return;
    }

    COLORREF const textColor = disabled ? (item->TopLevel ? m_Palette.MenuBarDisabledText :
                    m_Palette.MenuDisabledText) : selected ? m_Palette.HotText : m_Palette.ControlText;
    UINT const format = DT_SINGLELINE | DT_VCENTER | ((draw.itemState & ODS_NOACCEL) != 0 ? DT_HIDEPREFIX : 0);
    std::wstring caption;
    std::wstring shortcut;
    SplitShortcut(item->Text, caption, shortcut);

    HGDIOBJ const oldFont = SelectObject(context, MenuFont(dpi));
    int const oldMode = SetBkMode(context, TRANSPARENT);
    COLORREF const oldColor = SetTextColor(context, textColor);

    if (item->TopLevel)
    {
        RECT textRect = rect;
        DrawTextW(context, caption.c_str(), -1, &textRect, format | DT_CENTER);
    }
    else
    {
        int const gutter = ScaleForDPI(28, dpi);
        int const arrowWidth = ScaleForDPI(22, dpi);
        RECT textRect = { rect.left + gutter, rect.top, rect.right - arrowWidth, rect.bottom };
        DrawTextW(context, caption.c_str(), -1, &textRect, format | DT_LEFT);

        if (!shortcut.empty())
            DrawTextW(context, shortcut.c_str(), -1, &textRect, format | DT_RIGHT);

        int const glyph = ScaleForDPI(16, dpi);
        int const glyphTop = (rect.top + rect.bottom - glyph) / 2;

        if ((draw.itemState & ODS_CHECKED) != 0)
        {
            MENUITEMINFOW info = {};
            info.cbSize = sizeof(info);
            info.fMask = MIIM_FTYPE;
            GetMenuItemInfoW(item->Menu, item->Position, TRUE, &info);

            TGlyphCanvas canvas(glyph, glyph, background);
            canvas.Blend((info.fType & MFT_RADIOCHECK) != 0 ? EWin32GUIGlyph::Dot : EWin32GUIGlyph::Tick, 0, 0, glyph,
                textColor);
            canvas.Draw(context, rect.left + (gutter - glyph) / 2, glyphTop);
        }

        if (item->Submenu)
        {
            TGlyphCanvas canvas(glyph, glyph, background);
            canvas.Blend(EWin32GUIGlyph::ArrowRight, 0, 0, glyph, textColor);
            canvas.Draw(context, rect.right - arrowWidth + (arrowWidth - glyph) / 2, glyphTop);
            ExcludeClipRect(context, rect.left, rect.top, rect.right, rect.bottom);
        }
    }

    SetTextColor(context, oldColor);
    SetBkMode(context, oldMode);
    SelectObject(context, oldFont);
}
//---------------------------------------------------------------------------
COLORREF TWin32GUITheme::FailureTextColor() const
{
    return m_Dark ? m_Palette.FailureText : FailureTextColorOn(GetSysColor(COLOR_WINDOW));
}
//---------------------------------------------------------------------------
TWin32GUITheme::TMenuItem const* TWin32GUITheme::FindMenuItem(ULONG_PTR itemData) const
{
    for (std::unique_ptr<TMenuItem> const& item : m_MenuItems)
    {
        if (reinterpret_cast<ULONG_PTR>(item.get()) == itemData)
            return item.get();
    }

    return nullptr;
}
//---------------------------------------------------------------------------
LRESULT TWin32GUITheme::HandleControlMessage(HWND control, EWin32GUIThemedControl kind, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    bool const painted = kind == EWin32GUIThemedControl::ProgressBar || kind == EWin32GUIThemedControl::StatusBar ||
        kind == EWin32GUIThemedControl::TabControl || kind == EWin32GUIThemedControl::UpDown;

    bool const mayCreateArrows = message == WM_SIZE || (message == WM_PARENTNOTIFY && LOWORD(wParam) == WM_CREATE);

    if (kind == EWin32GUIThemedControl::TabControl && mayCreateArrows)
    {
        // Its scroll arrows, which it creates when it's too narrow for its tabs.
        LRESULT const result = DefSubclassProc(control, message, wParam, lParam);
        AttachUpDown(control);

        if (m_Dark && message == WM_SIZE)
            InvalidateRect(control, nullptr, FALSE);

        return result;
    }

    bool const doubleBuffered = kind == EWin32GUIThemedControl::ListView || kind == EWin32GUIThemedControl::TreeView;

    if (doubleBuffered && message == WM_WINDOWPOSCHANGING &&
        (reinterpret_cast<WINDOWPOS const*>(lParam)->flags & SWP_NOSIZE) == 0)
    {
        SetPropW(control, ResizingProperty, reinterpret_cast<HANDLE>(static_cast<INT_PTR>(1)));
    }
    else if (doubleBuffered && (message == WM_WINDOWPOSCHANGED || message == WM_PAINT))
    {
        RemovePropW(control, ResizingProperty);
    }
    else if (doubleBuffered && message == WM_ERASEBKGND && GetPropW(control, ResizingProperty) != nullptr)
    {
        // As the control grows, e.g. by a splitter, Windows erases the part it newly shows straight away, and the
        // control paints it later. Being double-buffered, the tree and the list skip the erase, so that part
        // showed what the screen had there before, e.g. the scroll bar at its old place, until they painted it.
        // Filled with their background here, and only while resizing, so their other repaints don't flicker.
        RECT client;
        GetClientRect(control, &client);
        FillSolid(reinterpret_cast<HDC>(wParam), client, BackgroundColor(control, kind));
        return 1;
    }

    if (!m_Dark)
    {
        // The control paints itself, through a buffer too, so it doesn't flicker while the window is resized:
        // erasing and painting it straight onto the screen would show its background in between. Not the status
        // bar, whose WM_PRINTCLIENT leaves out its background and text.
        bool const buffered = kind == EWin32GUIThemedControl::ProgressBar || kind == EWin32GUIThemedControl::TabControl;

        if (buffered && message == WM_ERASEBKGND)
            return 1;

        if (buffered && message == WM_PAINT)
        {
            PAINTSTRUCT paint = {};
            HDC const target = BeginPaint(control, &paint);

            {
                TPaintBuffer const buffer(target, paint.rcPaint);
                DefSubclassProc(control, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(buffer.Get()), PRF_CLIENT |
                    PRF_ERASEBKGND);
            }

            EndPaint(control, &paint);
            return 0;
        }

        return DefSubclassProc(control, message, wParam, lParam);
    }

    bool const bordered = kind == EWin32GUIThemedControl::Edit || kind == EWin32GUIThemedControl::ListView ||
        kind == EWin32GUIThemedControl::RichEdit || kind == EWin32GUIThemedControl::TreeView;

    if (bordered && message == WM_NCMOUSEMOVE && IsOverDisabledScrollBar(control, wParam))
    {
        // Not passed on: over a scroll bar, Windows fades its arrows as the mouse comes and goes, painting them
        // from a timer of its own, after PaintScrollBars() has, so a disabled bar's would end up light. A
        // disabled bar does nothing with the mouse anyway.
        return 0;
    }

    if (bordered && MayRedrawScrollBars(message))
    {
        LRESULT const result = DefSubclassProc(control, message, wParam, lParam);
        PaintScrollBars(control, nullptr);
        return result;
    }

    switch (message)
    {
        case WM_ERASEBKGND:
            if (kind == EWin32GUIThemedControl::Splitter)
            {
                RECT client;
                GetClientRect(control, &client);
                FillSolid(reinterpret_cast<HDC>(wParam), client, m_Palette.Panel);
                return 1;
            }

            if (painted)
                return 1; // WM_PAINT paints it all.

            break;

        case WM_MOUSELEAVE:
        case WM_MOUSEMOVE:
            if (kind == EWin32GUIThemedControl::TabControl)
            {
                // The tab under the mouse is drawn lighter.
                TCHITTESTINFO hit = {};

                if (message == WM_MOUSEMOVE)
                {
                    hit.pt.x = static_cast<short>(LOWORD(lParam));
                    hit.pt.y = static_cast<short>(HIWORD(lParam));
                }
                else
                {
                    hit.pt.x = -1;
                    hit.pt.y = -1;
                }

                int const hot = static_cast<int>(SendMessageW(control, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&hit))) +
                    1;

                if (reinterpret_cast<INT_PTR>(GetPropW(control, HotTabProperty)) != hot)
                {
                    SetPropW(control, HotTabProperty, reinterpret_cast<HANDLE>(static_cast<INT_PTR>(hot)));
                    InvalidateRect(control, nullptr, FALSE);
                }

                if (message == WM_MOUSEMOVE)
                {
                    TRACKMOUSEEVENT track = {};
                    track.cbSize = sizeof(track);
                    track.dwFlags = TME_LEAVE;
                    track.hwndTrack = control;
                    TrackMouseEvent(&track);
                }
            }

            if (kind == EWin32GUIThemedControl::UpDown)
            {
                // The arrow under the mouse is drawn lighter.
                InvalidateRect(control, nullptr, FALSE);

                if (message == WM_MOUSEMOVE)
                {
                    TRACKMOUSEEVENT track = {};
                    track.cbSize = sizeof(track);
                    track.dwFlags = TME_LEAVE;
                    track.hwndTrack = control;
                    TrackMouseEvent(&track);
                }
            }

            break;

        case WM_CAPTURECHANGED:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            if (kind == EWin32GUIThemedControl::UpDown)
            {
                // Pressed, or not any more.
                LRESULT const result = DefSubclassProc(control, message, wParam, lParam);
                InvalidateRect(control, nullptr, FALSE);
                return result;
            }

            break;

        case WM_NCCALCSIZE:
            if (bordered && GetPropW(control, OwnBorderProperty) != nullptr)
            {
                // The border's width taken off the window's rectangle first, so the control puts its scroll bars
                // and client area inside it, as with WS_EX_CLIENTEDGE.
                RECT* const proposed = wParam != FALSE ? &reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam)->rgrc[0] :
                        reinterpret_cast<RECT*>(lParam);
                int const edge = BorderWidth(control);
                InflateRect(proposed, -edge, -edge);
                return DefSubclassProc(control, message, wParam, lParam);
            }

            break;

        case WM_NCPAINT:
            if (bordered)
            {
                PaintNonClient(control, reinterpret_cast<HRGN>(wParam));
                return 0;
            }

            break;

        case WM_NOTIFY:
            if (kind == EWin32GUIThemedControl::ListView)
            {
                NMHDR const* const header = reinterpret_cast<NMHDR const*>(lParam);

                if (header->code == NM_CUSTOMDRAW &&
                    header->hwndFrom == reinterpret_cast<HWND>(SendMessageW(control, LVM_GETHEADER, 0, 0)))
                {
                    return OnHeaderCustomDraw(*reinterpret_cast<NMCUSTOMDRAW const*>(lParam));
                }
            }

            break;

        case WM_PAINT:
        case WM_PRINTCLIENT:
            if (painted)
            {
                PAINTSTRUCT paint = {};
                HDC const target = message == WM_PAINT ? BeginPaint(control, &paint) : reinterpret_cast<HDC>(wParam);
                RECT client;
                GetClientRect(control, &client);

                {
                    TPaintBuffer const buffer(target, client);

                    if (kind == EWin32GUIThemedControl::ProgressBar)
                        PaintProgressBar(control, buffer.Get());
                    else if (kind == EWin32GUIThemedControl::StatusBar)
                        PaintStatusBar(control, buffer.Get());
                    else if (kind == EWin32GUIThemedControl::TabControl)
                        PaintTabControl(control, buffer.Get());
                    else
                        PaintUpDown(control, buffer.Get());
                }

                if (message == WM_PAINT)
                    EndPaint(control, &paint);

                return 0;
            }

            break;

        case WM_SIZE:
            if (painted)
                InvalidateRect(control, nullptr, FALSE); // e.g. the status bar's size grip, at its right.

            break;

        case PBM_DELTAPOS:
        case PBM_SETPOS:
        case PBM_SETRANGE:
        case PBM_SETRANGE32:
        case PBM_SETSTATE:
        case PBM_STEPIT:
            if (kind == EWin32GUIThemedControl::ProgressBar)
            {
                // The themed bar animates toward a new position by itself; this one shows it at once. Not on the
                // themed bar's WM_TIMER, which only steps that animation, drawing nothing while dark.
                LRESULT const result = DefSubclassProc(control, message, wParam, lParam);
                InvalidateRect(control, nullptr, FALSE);
                return result;
            }

            break;

        default:
            break;
    }

    return DefSubclassProc(control, message, wParam, lParam);
}
//---------------------------------------------------------------------------
LRESULT TWin32GUITheme::HandleWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
        {
            LRESULT result = 0;

            if (OnCtlColor(message, reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam), result))
                return result;

            break;
        }

        case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT const* const draw = reinterpret_cast<DRAWITEMSTRUCT const*>(lParam);

            if (draw->CtlType == ODT_MENU && FindMenuItem(draw->itemData) != nullptr)
            {
                DrawMenuItem(*draw);
                return TRUE;
            }

            break;
        }

        case WM_ERASEBKGND:
            if (m_Dark)
            {
                // Only between the children, which a window with WS_CLIPCHILDREN leaves out of the context.
                RECT client;
                GetClientRect(window, &client);
                FillSolid(reinterpret_cast<HDC>(wParam), client, m_Palette.Panel);
                return 1;
            }

            break;

        case WM_MEASUREITEM:
        {
            MEASUREITEMSTRUCT* const measure = reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);

            if (measure->CtlType == ODT_MENU && FindMenuItem(measure->itemData) != nullptr)
            {
                MeasureMenuItem(window, *measure);
                return TRUE;
            }

            break;
        }

        case WM_NCACTIVATE:
        case WM_NCPAINT:
            if (m_Dark && window == m_MenuBarWindow)
            {
                LRESULT const result = DefSubclassProc(window, message, wParam, lParam);
                PaintMenuBarLine(window);
                return result;
            }

            break;

        case WM_NOTIFY:
            if (m_Dark)
            {
                NMHDR const* const header = reinterpret_cast<NMHDR const*>(lParam);

                if (header->code == NM_CUSTOMDRAW && ClassNameIs(header->hwndFrom, WC_BUTTONW))
                    return OnButtonCustomDraw(*reinterpret_cast<NMCUSTOMDRAW const*>(lParam));

                if (header->code == NM_CUSTOMDRAW && ClassNameIs(header->hwndFrom, WC_TREEVIEWW))
                    return OnTreeCustomDraw(*reinterpret_cast<NMTVCUSTOMDRAW*>(lParam));
            }

            break;

        default:
            break;
    }

    return DefSubclassProc(window, message, wParam, lParam);
}
//---------------------------------------------------------------------------
bool TWin32GUITheme::IsDark() const
{
    return m_Dark;
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::MeasureMenuItem

    Carbon's sizes: a menu item 21 pixels high at 96 DPI and a separator 9, with room for the check mark
    gutter, the caption, the shortcut and the submenu arrow; a menu bar item its caption plus 6 pixels each
    side, as tall as the menu bar. Windows adds the width of a check mark to a menu bar item's, which is taken
    off here.
*/
void TWin32GUITheme::MeasureMenuItem(HWND window, MEASUREITEMSTRUCT& measure)
{
    TMenuItem const* const item = FindMenuItem(measure.itemData);
    UINT const dpi = WindowDPI(window);

    if (item->Separator)
    {
        measure.itemWidth = static_cast<UINT>(ScaleForDPI(10, dpi));
        measure.itemHeight = static_cast<UINT>(ScaleForDPI(9, dpi));
        return;
    }

    std::wstring caption;
    std::wstring shortcut;
    SplitShortcut(item->Text, caption, shortcut);

    HDC const context = GetDC(window);
    HGDIOBJ const oldFont = SelectObject(context, MenuFont(dpi));
    SIZE const captionSize = TextSize(context, caption, 0);
    SIZE const shortcutSize = shortcut.empty() ? SIZE { 0, 0 } : TextSize(context, shortcut, 0);
    SelectObject(context, oldFont);
    ReleaseDC(window, context);

    if (item->TopLevel)
    {
        int const captionWidth = static_cast<int>(captionSize.cx);
        int const added = GetSystemMetricsForDpi(SM_CXMENUCHECK, dpi) - 1;
        measure.itemWidth = static_cast<UINT>((std::max)(captionWidth, captionWidth + ScaleForDPI(12, dpi) - added));
        measure.itemHeight = static_cast<UINT>(GetSystemMetricsForDpi(SM_CYMENU, dpi));
    }
    else
    {
        measure.itemWidth = static_cast<UINT>(ScaleForDPI(28, dpi) + captionSize.cx + (shortcut.empty() ? 0 :
                ScaleForDPI(32, dpi) + shortcutSize.cx) + ScaleForDPI(22, dpi));
        measure.itemHeight = static_cast<UINT>(ScaleForDPI(21, dpi));
    }
}
//---------------------------------------------------------------------------
HFONT TWin32GUITheme::MenuFont(UINT dpi)
{
    if (m_MenuFont.Get() == nullptr || m_MenuFontDPI != dpi)
    {
        NONCLIENTMETRICSW metrics = {};
        metrics.cbSize = sizeof(metrics);

        if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, dpi) != 0)
        {
            m_MenuFont = TWin32GUIFont(CreateFontIndirectW(&metrics.lfMenuFont));
            m_MenuFontDPI = dpi;
        }
    }

    return m_MenuFont.Get() != nullptr ? m_MenuFont.Get() : static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::MenuFrameProc

    A menu window's subclass while it exists: its frame (its non-client area) painted in Carbon's colors while
    dark, also into the context WM_PRINT gives it for the menu's fade animation.
*/
LRESULT CALLBACK TWin32GUITheme::MenuFrameProc(HWND menu, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
    DWORD_PTR theme)
{
    try
    {
        TWin32GUITheme const* const owner = reinterpret_cast<TWin32GUITheme const*>(theme);

        switch (message)
        {
            case WM_NCDESTROY:
                RemoveWindowSubclass(menu, &MenuFrameProc, id);
                break;

            case WM_NCPAINT:
                if (owner->m_Dark)
                {
                    owner->PaintMenuFrame(menu, nullptr);
                    return 0;
                }

                break;

            case WM_PRINT:
                if (owner->m_Dark && (lParam & PRF_NONCLIENT) != 0)
                {
                    LRESULT const result = DefSubclassProc(menu, message, wParam, lParam);
                    owner->PaintMenuFrame(menu, reinterpret_cast<HDC>(wParam));
                    return result;
                }

                break;

            default:
                break;
        }
    }
    catch (...)
    {
    }

    return DefSubclassProc(menu, message, wParam, lParam);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::MenuHookProc

    This thread's WH_CALLWNDPROC hook: subclasses each menu window as it's created (see MenuFrameProc()).
*/
LRESULT CALLBACK TWin32GUITheme::MenuHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    TWin32GUITheme* const theme = s_Instance;

    if (code == HC_ACTION && theme != nullptr)
    {
        CWPSTRUCT const* const call = reinterpret_cast<CWPSTRUCT const*>(lParam);

        if (call->message == WM_CREATE && ClassNameIs(call->hwnd, MenuClassName))
            SetWindowSubclass(call->hwnd, &MenuFrameProc, MenuFrameSubclassID, reinterpret_cast<DWORD_PTR>(theme));
    }

    return CallNextHookEx(theme != nullptr ? theme->m_MenuHook : nullptr, code, wParam, lParam);
}
//---------------------------------------------------------------------------
LRESULT TWin32GUITheme::OnButtonCustomDraw(NMCUSTOMDRAW const& draw) const
{
    if (draw.dwDrawStage != CDDS_PREPAINT)
        return CDRF_DODEFAULT;

    switch (GetWindowLongPtrW(draw.hdr.hwndFrom, GWL_STYLE) & BS_TYPEMASK)
    {
        case BS_DEFPUSHBUTTON:
        case BS_PUSHBUTTON:
            DrawButton(draw);
            return CDRF_SKIPDEFAULT;

        case BS_AUTORADIOBUTTON:
        case BS_RADIOBUTTON:
            DrawCheckButton(draw, true);
            return CDRF_SKIPDEFAULT;

        case BS_3STATE:
        case BS_AUTO3STATE:
        case BS_AUTOCHECKBOX:
        case BS_CHECKBOX:
            DrawCheckButton(draw, false);
            return CDRF_SKIPDEFAULT;

        default:
            return CDRF_DODEFAULT;
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::OnCtlColor

    A read-only edit box (WM_CTLCOLORSTATIC) is drawn like an editable one, in both looks, as the VCL GUI
    runner's are, rather than gray. While dark, everything else gets the palette's colors; while light, the
    system's.
*/
bool TWin32GUITheme::OnCtlColor(UINT message, HDC context, HWND control, LRESULT& result) const
{
    bool const edit = ClassNameIs(control, WC_EDITW);
    bool const enabled = IsWindowEnabled(control) != FALSE;

    if (message == WM_CTLCOLORSTATIC && edit && enabled)
    {
        SetTextColor(context, WindowTextColor());
        SetBkColor(context, WindowColor());
        result = reinterpret_cast<LRESULT>(m_Dark ? m_WindowBrush : GetSysColorBrush(COLOR_WINDOW));
        return true;
    }

    if (!m_Dark)
        return false;

    switch (message)
    {
        case WM_CTLCOLOREDIT:
            SetTextColor(context, m_Palette.WindowText);
            SetBkColor(context, m_Palette.Window);
            result = reinterpret_cast<LRESULT>(m_WindowBrush);
            return true;

        case WM_CTLCOLORSTATIC:
            if (edit)
            {
                // A disabled edit box, e.g. the filter box during a run.
                SetTextColor(context, m_Palette.DisabledText);
                SetBkColor(context, m_Palette.Window);
                result = reinterpret_cast<LRESULT>(m_WindowBrush);
                return true;
            }

            SetTextColor(context, enabled ? m_Palette.ControlText : m_Palette.DisabledText);
            SetBkColor(context, m_Palette.Panel);
            result = reinterpret_cast<LRESULT>(m_PanelBrush);
            return true;

        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
            result = reinterpret_cast<LRESULT>(m_PanelBrush);
            return true;

        default:
            return false;
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::OnHeaderCustomDraw

    As Carbon draws a header: each column shaded from top to bottom (lighter under the mouse), with a line at
    its right and along the bottom, and the part right of the last column the same.
*/
LRESULT TWin32GUITheme::OnHeaderCustomDraw(NMCUSTOMDRAW const& draw) const
{
    HWND const header = draw.hdr.hwndFrom;
    HDC const context = draw.hdc;

    if (draw.dwDrawStage == CDDS_PREPAINT)
        return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;

    if (draw.dwDrawStage == CDDS_ITEMPREPAINT)
    {
        RECT rect = draw.rc;

        if ((draw.uItemState & CDIS_HOT) != 0)
            FillSolid(context, rect, m_Palette.HeaderHot);
        else
            FillGradient(context, rect, m_Palette.HeaderTop, m_Palette.HeaderBottom);

        RECT const divider = { rect.right - 1, rect.top, rect.right, rect.bottom };
        RECT const bottom = { rect.left, rect.bottom - 1, rect.right, rect.bottom };
        FillSolid(context, divider, m_Palette.HeaderDivider);
        FillSolid(context, bottom, m_Palette.HeaderDivider);

        wchar_t text[256] = {};
        HDITEMW item = {};
        item.mask = HDI_TEXT;
        item.pszText = text;
        item.cchTextMax = 256;
        SendMessageW(header, HDM_GETITEMW, draw.dwItemSpec, reinterpret_cast<LPARAM>(&item));

        rect.left += ScaleForDPI(6, WindowDPI(header));
        int const oldMode = SetBkMode(context, TRANSPARENT);
        COLORREF const oldColor = SetTextColor(context, m_Palette.ControlText);
        DrawTextW(context, text, -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SetTextColor(context, oldColor);
        SetBkMode(context, oldMode);
        return CDRF_SKIPDEFAULT;
    }

    if (draw.dwDrawStage == CDDS_POSTPAINT)
    {
        int const count = static_cast<int>(SendMessageW(header, HDM_GETITEMCOUNT, 0, 0));
        RECT last = {};

        if (count > 0)
            SendMessageW(header, HDM_GETITEMRECT, static_cast<WPARAM>(count - 1), reinterpret_cast<LPARAM>(&last));

        RECT rest;
        GetClientRect(header, &rest);
        rest.left = last.right;

        if (rest.left < rest.right)
        {
            FillGradient(context, rest, m_Palette.HeaderTop, m_Palette.HeaderBottom);
            RECT const bottom = { rest.left, rest.bottom - 1, rest.right, rest.bottom };
            FillSolid(context, bottom, m_Palette.HeaderDivider);
        }
    }

    return CDRF_DODEFAULT;
}
//---------------------------------------------------------------------------
LRESULT TWin32GUITheme::OnListCustomDraw(NMLVCUSTOMDRAW& draw, COLORREF rowTextColor) const
{
    if (draw.nmcd.dwDrawStage == CDDS_PREPAINT)
        return CDRF_NOTIFYITEMDRAW;

    if (draw.nmcd.dwDrawStage != CDDS_ITEMPREPAINT)
        return CDRF_DODEFAULT;

    if (m_Dark)
    {
        DrawListRow(draw, rowTextColor);
        return CDRF_SKIPDEFAULT;
    }

    draw.clrText = rowTextColor;
    return CDRF_DODEFAULT;
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::OnTreeCustomDraw

    The selected node as the VCL GUI runner shows it under Carbon: white text in a gray box with a darker frame,
    whether the tree has the focus or not. Without the selected, hot and focus states, the theme draws neither
    its own selection nor its highlight under the mouse.
*/
LRESULT TWin32GUITheme::OnTreeCustomDraw(NMTVCUSTOMDRAW& draw) const
{
    if (draw.nmcd.dwDrawStage == CDDS_PREPAINT)
        return CDRF_NOTIFYITEMDRAW;

    HTREEITEM const item = reinterpret_cast<HTREEITEM>(draw.nmcd.dwItemSpec);
    bool const selected = SendMessageW(draw.nmcd.hdr.hwndFrom, TVM_GETNEXTITEM, TVGN_CARET, 0) ==
        static_cast<LRESULT>(draw.nmcd.dwItemSpec);

    if (draw.nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
        draw.nmcd.uItemState &= ~static_cast<UINT>(CDIS_SELECTED | CDIS_HOT | CDIS_FOCUS);
        draw.clrText = selected ? m_Palette.HotText : m_Palette.WindowText;
        draw.clrTextBk = selected ? m_Palette.Selection : m_Palette.Window;

        return selected ? CDRF_NOTIFYPOSTPAINT : CDRF_DODEFAULT;
    }

    if (draw.nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT && selected)
    {
        // TVM_GETITEMRECT takes the item in the rectangle and gives back its text's bounds (wParam TRUE).
        RECT text = {};
        std::memcpy(&text, &item, sizeof(item));

        if (SendMessageW(draw.nmcd.hdr.hwndFrom, TVM_GETITEMRECT, TRUE, reinterpret_cast<LPARAM>(&text)) != 0)
            FrameSolid(draw.nmcd.hdc, text, m_Palette.SelectionFrame);
    }

    return CDRF_DODEFAULT;
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintBorder

    The border a bordered control has of its own while dark (see ApplyToBorder()), in Carbon's two edit border
    colors, the outer one a pixel wide; or, on one that still has WS_EX_CLIENTEDGE, that border painted over.
    Into 'context', in the window's coordinates.
*/
void TWin32GUITheme::PaintBorder(HWND control, HDC context) const
{
    if (!HasThemedBorder(control))
        return;

    RECT rect;
    GetWindowRect(control, &rect);
    OffsetRect(&rect, -rect.left, -rect.top);

    int const edge = BorderWidth(control);
    FrameSolid(context, rect, m_Palette.EditBorderOuter);

    for (int i = 1; i < edge; ++i)
        FrameSolid(context, Inset(rect, i), m_Palette.EditBorderInner);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintMenuBarLine

    The line DefWindowProc() draws under the menu bar, in a light system color, painted over in the
    window's background.
*/
void TWin32GUITheme::PaintMenuBarLine(HWND window) const
{
    MENUBARINFO bar = {};
    bar.cbSize = sizeof(bar);

    if (GetMenuBarInfo(window, OBJID_MENU, 0, &bar) == 0)
        return;

    RECT windowRect;
    GetWindowRect(window, &windowRect);
    RECT line = bar.rcBar;
    OffsetRect(&line, -windowRect.left, -windowRect.top);
    line.top = line.bottom;
    line.bottom = line.top + 1;

    HDC const context = GetWindowDC(window);

    if (context == nullptr)
        return;

    FillSolid(context, line, m_Palette.Panel);
    ReleaseDC(window, context);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintMenuFrame

    A menu window's non-client area, its frame, as Carbon's: two one-pixel lines, then the menu's background.
    Into 'given' if it isn't nullptr (WM_PRINT's context), or else the window's own.
*/
void TWin32GUITheme::PaintMenuFrame(HWND menu, HDC given) const
{
    RECT window;
    GetWindowRect(menu, &window);
    RECT client;
    GetClientRect(menu, &client);
    MapWindowPoints(menu, nullptr, reinterpret_cast<POINT*>(&client), 2);
    OffsetRect(&client, -window.left, -window.top);
    OffsetRect(&window, -window.left, -window.top);

    HDC const context = given != nullptr ? given : GetWindowDC(menu);

    if (context == nullptr)
        return;

    int const saved = SaveDC(context);
    ExcludeClipRect(context, client.left, client.top, client.right, client.bottom);
    FillSolid(context, window, m_Palette.MenuBackground);
    FrameSolid(context, window, m_Palette.MenuBorderOuter);
    FrameSolid(context, Inset(window, 1), m_Palette.MenuBorderInner);
    RestoreDC(context, saved);

    if (given == nullptr)
        ReleaseDC(menu, context);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintNonClient

    Each part painted once: the control (and Windows) paint 'update', WM_NCPAINT's region, less what
    PaintBorder() and PaintScrollBars() paint, which they then paint. Letting the control paint it all first,
    as before, showed its light parts in between, e.g. a disabled scroll bar's arrows and a light border without
    the scroll bars, which flickered while the control was resized, e.g. by a splitter.
*/
void TWin32GUITheme::PaintNonClient(HWND control, HRGN update) const
{
    RECT window;
    GetWindowRect(control, &window);

    // In screen coordinates, as WM_NCPAINT's region is: inside the border.
    RECT const inside = HasThemedBorder(control) ? Inset(window, BorderWidth(control)) : window;
    HRGN const rest = CreateRectRgnIndirect(&inside);

    if (rest != nullptr)
    {
        // 1 is the whole window.
        if (update != reinterpret_cast<HRGN>(1))
            CombineRgn(rest, rest, update, RGN_AND);

        SCROLLBARINFO bars[2] = {};
        LONG const ids[2] = { OBJID_VSCROLL, OBJID_HSCROLL };
        bool visible[2] = { false, false };

        for (int i = 0; i < 2; ++i)
        {
            bars[i].cbSize = sizeof(bars[i]);
            visible[i] = GetScrollBarInfo(control, ids[i], &bars[i]) != 0 &&
                (bars[i].rgstate[0] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN)) == 0;

            if (visible[i] && (bars[i].rgstate[0] & STATE_SYSTEM_UNAVAILABLE) != 0)
                ExcludeRect(rest, bars[i].rcScrollBar);
        }

        if (visible[0] && visible[1])
        {
            RECT const corner = { bars[0].rcScrollBar.left, bars[1].rcScrollBar.top, bars[0].rcScrollBar.right,
                                  bars[1].rcScrollBar.bottom };
            ExcludeRect(rest, corner);
        }

        DefSubclassProc(control, WM_NCPAINT, reinterpret_cast<WPARAM>(rest), 0);
        DeleteObject(rest);
    }

    HDC const context = GetWindowDC(control);

    if (context == nullptr)
        return;

    PaintScrollBars(control, context);
    PaintBorder(control, context);
    ReleaseDC(control, context);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintProgressBar

    As Carbon draws it: a border around a dark track, and the bar shaded from top to bottom, red once a test
    has failed (PBST_ERROR).
*/
void TWin32GUITheme::PaintProgressBar(HWND progressBar, HDC context) const
{
    RECT client;
    GetClientRect(progressBar, &client);
    int const line = (std::max)(1, ScaleForDPI(1, WindowDPI(progressBar)));

    FillSolid(context, client, m_Palette.ProgressBorder);
    RECT const track = Inset(client, line);
    FillSolid(context, track, m_Palette.ProgressTrack);

    PBRANGE range = {};
    SendMessageW(progressBar, PBM_GETRANGE, TRUE, reinterpret_cast<LPARAM>(&range));
    int const position = static_cast<int>(SendMessageW(progressBar, PBM_GETPOS, 0, 0));
    int const span = range.iHigh - range.iLow;

    if (span <= 0)
        return;

    RECT bar = track;
    int const done = (std::max)(0, (std::min)(span, position - range.iLow));
    bar.right = track.left + MulDiv(track.right - track.left, done, span);

    if (bar.right <= bar.left)
        return;

    bool const failed = SendMessageW(progressBar, PBM_GETSTATE, 0, 0) == PBST_ERROR;
    FillGradient(context, bar, failed ? m_Palette.ProgressFailedTop : m_Palette.ProgressChunkTop,
        failed ? m_Palette.ProgressFailedBottom : m_Palette.ProgressChunkBottom);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintScrollBars

    What the dark scroll bar theme ("DarkMode_Explorer") leaves light, painted over:
    - the corner between a vertical and a horizontal scroll bar, which Windows paints in the system colors
      whatever the theme (a light edge around a gray square), painted as the theme's track;
    - a disabled scroll bar's arrows, which the theme draws nearly white, e.g. the empty detail pane's, which
      keeps its scroll bar while there's nothing to scroll. Such a bar is drawn again in the theme's normal
      state, which has no thumb when there's nothing to scroll, with its arrows faded halfway toward the track,
      so they read as disabled.
    An enabled bar is left as it is. Called after each message that may draw the bars (see
    MayRedrawScrollBars()), so it's quick when there's nothing to paint. Into 'given' if it isn't nullptr
    (PaintNonClient()'s), in the window's coordinates, or else the window's own context.
*/
void TWin32GUITheme::PaintScrollBars(HWND control, HDC given) const
{
    SCROLLBARINFO bars[2] = {};
    LONG const ids[2] = { OBJID_VSCROLL, OBJID_HSCROLL };
    bool visible[2] = { false, false };

    for (int i = 0; i < 2; ++i)
    {
        bars[i].cbSize = sizeof(bars[i]);
        visible[i] = GetScrollBarInfo(control, ids[i], &bars[i]) != 0 &&
            (bars[i].rgstate[0] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN)) == 0;
    }

    bool const corner = visible[0] && visible[1];
    bool const disabled[2] = {
        visible[0] && (bars[0].rgstate[0] & STATE_SYSTEM_UNAVAILABLE) != 0,
        visible[1] && (bars[1].rgstate[0] & STATE_SYSTEM_UNAVAILABLE) != 0,
    };

    if (!corner && !disabled[0] && !disabled[1])
        return;

    HDC const context = given != nullptr ? given : GetWindowDC(control);
    HTHEME const theme = OpenThemeData(control, L"ScrollBar");
    RECT window;
    GetWindowRect(control, &window);

    if (context != nullptr && theme != nullptr)
    {
        if (corner)
        {
            // Below the vertical bar and right of the horizontal one.
            RECT box = { bars[0].rcScrollBar.left, bars[1].rcScrollBar.top, bars[0].rcScrollBar.right,
                         bars[1].rcScrollBar.bottom };
            OffsetRect(&box, -window.left, -window.top);
            DrawThemeBackground(theme, context, SBP_LOWERTRACKVERT, SCRBS_NORMAL, &box, nullptr);
        }

        for (int i = 0; i < 2; ++i)
        {
            if (!disabled[i])
                continue;

            bool const vertical = ids[i] == OBJID_VSCROLL;
            RECT bar = bars[i].rcScrollBar;
            OffsetRect(&bar, -window.left, -window.top);

            // The arrow buttons at its two ends, each as long as a line button.
            RECT first = bar;
            RECT second = bar;

            if (vertical)
            {
                first.bottom = first.top + bars[i].dxyLineButton;
                second.top = second.bottom - bars[i].dxyLineButton;
            }
            else
            {
                first.right = first.left + bars[i].dxyLineButton;
                second.left = second.right - bars[i].dxyLineButton;
            }

            TPaintBuffer const buffer(context, bar);
            HDC const target = buffer.Get();
            DrawThemeBackground(theme, target, vertical ? SBP_UPPERTRACKVERT : SBP_UPPERTRACKHORZ, SCRBS_NORMAL, &bar,
                nullptr);
            DrawThemeBackground(theme, target, SBP_ARROWBTN, vertical ? ABS_UPNORMAL : ABS_LEFTNORMAL, &first,
                nullptr);
            DrawThemeBackground(theme, target, SBP_ARROWBTN, vertical ? ABS_DOWNNORMAL : ABS_RIGHTNORMAL, &second,
                nullptr);

            for (RECT const& button : { first, second })
            {
                TGlyphCanvas canvas(button.right - button.left, button.bottom - button.top, m_Palette.Window);
                canvas.CopyFrom(target, button.left, button.top);
                // Toward its corner's pixel, the button's background.
                canvas.Fade(canvas.PixelAt(0, 0), 1, 2);
                canvas.Draw(target, button.left, button.top);
            }
        }
    }

    if (theme != nullptr)
        CloseThemeData(theme);

    if (context != nullptr && given == nullptr)
        ReleaseDC(control, context);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintStatusBar

    As Carbon draws it: a line along the top, each part's text, and the size grip's dots at the bottom right
    (unless its window is maximized), where Carbon's 16-pixel grip has them.
*/
void TWin32GUITheme::PaintStatusBar(HWND statusBar, HDC context) const
{
    RECT client;
    GetClientRect(statusBar, &client);
    UINT const dpi = WindowDPI(statusBar);
    int const line = (std::max)(1, ScaleForDPI(1, dpi));

    FillSolid(context, client, m_Palette.Panel);
    RECT const top = { client.left, client.top, client.right, client.top + line };
    FillSolid(context, top, m_Palette.StatusLine);

    HGDIOBJ const oldFont = SelectObject(context, reinterpret_cast<HGDIOBJ>(SendMessageW(statusBar, WM_GETFONT, 0, 0)));
    int const oldMode = SetBkMode(context, TRANSPARENT);
    COLORREF const oldColor = SetTextColor(context, m_Palette.ControlText);
    int const parts = static_cast<int>(SendMessageW(statusBar, SB_GETPARTS, 0, 0));

    for (int part = 0; part < parts; ++part)
    {
        RECT rect = {};
        SendMessageW(statusBar, SB_GETRECT, static_cast<WPARAM>(part), reinterpret_cast<LPARAM>(&rect));

        std::wstring text(static_cast<std::size_t>(LOWORD(SendMessageW(statusBar, SB_GETTEXTLENGTHW,
            static_cast<WPARAM>(part), 0))) + 1, L'\0');
        text.resize(static_cast<std::size_t>(LOWORD(SendMessageW(statusBar, SB_GETTEXTW, static_cast<WPARAM>(part),
            reinterpret_cast<LPARAM>(&text[0])))));

        rect.left += ScaleForDPI(4, dpi);
        DrawTextW(context, text.c_str(), static_cast<int>(text.size()), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE |
            DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    SetTextColor(context, oldColor);
    SetBkMode(context, oldMode);
    SelectObject(context, oldFont);

    HWND const parent = GetParent(statusBar);
    bool const maximized = parent != nullptr && IsZoomed(parent) != FALSE;

    if ((GetWindowLongPtrW(statusBar, GWL_STYLE) & SBARS_SIZEGRIP) == 0 || maximized)
        return;

    // Carbon's dots, at 96 DPI in its 16-pixel grip: a triangle of four rows, two pixels apart.
    int const cell = ScaleForDPI(16, dpi);
    int const left = client.right - cell;
    int const gripTop = client.bottom - cell;

    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column <= row; ++column)
        {
            int const x = left + ScaleForDPI(10 - 2 * column, dpi);
            int const y = gripTop + ScaleForDPI(7 + 2 * row, dpi);
            RECT const dot = { x, y, x + line, y + line };
            FillSolid(context, dot, m_Palette.GripDot);
        }
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintTabControl

    As Carbon draws the results' tabs: the strip in the window's background, the pages framed by two lines,
    the selected tab shaded and joined to them, the others darker (lighter under the mouse), the selected tab's
    text brighter. The pages themselves are separate windows above the tab control.
*/
void TWin32GUITheme::PaintTabControl(HWND tabControl, HDC context) const
{
    RECT client;
    GetClientRect(tabControl, &client);
    UINT const dpi = WindowDPI(tabControl);
    int const line = (std::max)(1, ScaleForDPI(1, dpi));

    FillSolid(context, client, m_Palette.Panel);

    RECT pages = client;
    SendMessageW(tabControl, TCM_ADJUSTRECT, FALSE, reinterpret_cast<LPARAM>(&pages));
    pages = Inset(pages, -2 * line);
    FillSolid(context, pages, m_Palette.TabOuter);
    FillSolid(context, Inset(pages, line), m_Palette.TabBorder);
    FillSolid(context, Inset(pages, 2 * line), m_Palette.Panel);

    int const count = static_cast<int>(SendMessageW(tabControl, TCM_GETITEMCOUNT, 0, 0));
    int const selected = static_cast<int>(SendMessageW(tabControl, TCM_GETCURSEL, 0, 0));
    int const hot = static_cast<int>(reinterpret_cast<INT_PTR>(GetPropW(tabControl, HotTabProperty))) - 1;
    bool const showFocus = GetFocus() == tabControl && !HidesFocus(tabControl);
    UINT const format = DT_CENTER | DT_VCENTER | DT_SINGLELINE | (HidesAccelerators(tabControl) ? DT_HIDEPREFIX : 0);

    HGDIOBJ const oldFont = SelectObject(context, reinterpret_cast<HGDIOBJ>(SendMessageW(tabControl, WM_GETFONT, 0,
        0)));
    int const oldMode = SetBkMode(context, TRANSPARENT);

    for (int i = 0; i < count; ++i)
    {
        RECT tab = {};
        SendMessageW(tabControl, TCM_GETITEMRECT, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(&tab));

        if (i == selected)
        {
            // Raised and widened, and down over the pages' frame, which it opens into.
            tab.left -= 2 * line;
            tab.right += 2 * line;
            tab.top -= 2 * line;
            tab.bottom += 2 * line;
            FillSolid(context, tab, m_Palette.TabOuter);
            RECT const inside = { tab.left + line, tab.top + line, tab.right - line, tab.bottom };
            FillSolid(context, inside, m_Palette.TabBorder);
            RECT const face = { inside.left + line, inside.top + line, inside.right - line, inside.bottom };
            FillGradient(context, face, m_Palette.TabActiveTop, m_Palette.TabActiveBottom);
        }
        else
        {
            FillSolid(context, tab, m_Palette.TabInactiveBorder);
            FillSolid(context, Inset(tab, line), i == hot ? m_Palette.TabHot : m_Palette.TabInactive);
        }

        wchar_t text[128] = {};
        TCITEMW item = {};
        item.mask = TCIF_TEXT;
        item.pszText = text;
        item.cchTextMax = 128;
        SendMessageW(tabControl, TCM_GETITEMW, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(&item));

        RECT textRect = tab;

        if (i == selected)
            textRect.bottom -= 2 * line;

        COLORREF const oldColor = SetTextColor(context, i == selected ? m_Palette.WindowText : m_Palette.ControlText);
        DrawTextW(context, text, -1, &textRect, format);
        SetTextColor(context, oldColor);

        if (i == selected && showFocus)
        {
            RECT focus = Inset(textRect, 3 * line);
            DrawFocusRect(context, &focus);
        }
    }

    SetBkMode(context, oldMode);
    SelectObject(context, oldFont);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::PaintUpDown

    As Carbon draws a pair of horizontal spin buttons, e.g. the results tabs' scroll arrows: each an outline
    around a face shaded from top to bottom, lighter under the mouse, with a darker edge at its top and left
    while pressed, and an arrow pointing left (decrease) or right (increase).
*/
void TWin32GUITheme::PaintUpDown(HWND upDown, HDC context) const
{
    RECT client;
    GetClientRect(upDown, &client);
    UINT const dpi = WindowDPI(upDown);
    int const line = (std::max)(1, ScaleForDPI(1, dpi));
    bool const disabled = IsWindowEnabled(upDown) == FALSE;

    POINT cursor = {};
    GetCursorPos(&cursor);
    bool const underMouse = WindowFromPoint(cursor) == upDown;
    ScreenToClient(upDown, &cursor);
    bool const pressing = GetCapture() == upDown && (GetKeyState(VK_LBUTTON) & 0x8000) != 0;

    int const middle = (client.left + client.right) / 2;
    RECT const halves[] = {
        { client.left, client.top, middle, client.bottom },
        { middle, client.top, client.right, client.bottom },
    };

    for (int i = 0; i < 2; ++i)
    {
        RECT const& half = halves[i];
        bool const hot = !disabled && underMouse && PtInRect(&half, cursor) != FALSE;

        FillSolid(context, half, disabled ? m_Palette.SpinDisabledOutline : m_Palette.SpinOutline);
        RECT const face = Inset(half, line);

        if (disabled)
            FillGradient(context, face, m_Palette.SpinDisabledFaceTop, m_Palette.SpinDisabledFaceBottom);
        else if (hot)
            FillGradient(context, face, m_Palette.SpinHotFaceTop, m_Palette.SpinHotFaceBottom);
        else
            FillGradient(context, face, m_Palette.SpinFaceTop, m_Palette.SpinFaceBottom);

        if (hot && pressing)
        {
            RECT const top = { face.left, face.top, face.right, face.top + line };
            RECT const left = { face.left, face.top, face.left + line, face.bottom };
            FillSolid(context, top, m_Palette.SpinPressedEdge);
            FillSolid(context, left, m_Palette.SpinPressedEdge);
        }

        int const size = (std::min)(face.right - face.left, face.bottom - face.top);

        if (size <= 0)
            continue;

        int const left = face.left + (face.right - face.left - size) / 2;
        int const top = face.top + (face.bottom - face.top - size) / 2;
        TGlyphCanvas canvas(size, size, m_Palette.SpinFaceTop);
        canvas.CopyFrom(context, left, top);
        canvas.Blend(i == 0 ? EWin32GUIGlyph::ArrowLeft : EWin32GUIGlyph::ArrowRight, 0, 0, size, disabled ?
                m_Palette.SpinDisabledArrow : m_Palette.SpinArrow);
        canvas.Draw(context, left, top);
    }
}
//---------------------------------------------------------------------------
TWin32GUIPalette const& TWin32GUITheme::Palette() const
{
    return m_Palette;
}
//---------------------------------------------------------------------------
void TWin32GUITheme::Refresh()
{
    ApplyToMenus();

    for (HWND const window : m_Windows)
        ApplyToTitleBar(window);

    // By index, on a copy: applying the look can resize a tab control, which may then create and attach its
    // scroll arrows.
    for (std::size_t i = 0; i < m_Controls.size(); ++i)
    {
        TAttachedControl const attached = m_Controls[i];
        ApplyToControl(attached);
    }

    for (HWND const window : m_Windows)
        RedrawWindow(window, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::RefreshMenu

    Only the items of 'menu' and its submenus are made again, and the old ones forgotten, so the items of the
    menu bar and of the other menus keep theirs.
*/
void TWin32GUITheme::RefreshMenu(HMENU menu)
{
    if (m_MenuBar == nullptr || menu == nullptr)
        return;

    std::vector<std::unique_ptr<TMenuItem> > items;
    ApplyToMenu(menu, items);

    // The menus just walked: 'menu', and those of its submenus' new items.
    std::vector<HMENU> walked = { menu };

    for (std::unique_ptr<TMenuItem> const& item : items)
        walked.push_back(item->Menu);

    m_MenuItems.erase(std::remove_if(m_MenuItems.begin(), m_MenuItems.end(),
        [&walked](std::unique_ptr<TMenuItem> const& item)
        {
            return std::find(walked.begin(), walked.end(), item->Menu) != walked.end();
        }), m_MenuItems.end());

    for (std::unique_ptr<TMenuItem>& item : items)
        m_MenuItems.push_back(std::move(item));
}
//---------------------------------------------------------------------------
void TWin32GUITheme::SetDark(bool dark)
{
    if (dark == m_Dark)
        return;

    m_Dark = dark;
    Refresh();
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::SetDarkTheme

    A file that can't be read leaves Carbon's colors, as does one ReadGUIThemeFile() finds unusable.
*/
std::vector<std::string> TWin32GUITheme::SetDarkTheme(std::string const& name)
{
    std::vector<std::string> problems;
    TWin32GUIPalette palette = CarbonPalette();

    if (name != GUIDefaultDarkThemeName())
    {
        try
        {
            std::string const path = GUIThemesFolderPath() + "\\" + name + ".ini";

            if (!FileExists(path))
            {
                problems.push_back(GUIThemeProblemLine(name, "There's no theme file " + path + ", so " +
                    GUIDefaultDarkThemeName() + " is used instead."));
            }
            else
            {
                TGUIThemeFile const file = ReadGUIThemeFile(ReadFileBytes(path), PaletteRoleNames());

                for (std::string const& problem : file.Problems)
                    problems.push_back(GUIThemeProblemLine(name, problem));

                palette = PaletteWithThemeColors(palette, file.Colors);
            }
        }
        catch (std::exception const& exception)
        {
            problems.push_back(GUIThemeProblemLine(name, "Its theme file couldn't be read, so " +
                GUIDefaultDarkThemeName() + " is used instead: " + exception.what()));
        }
    }

    SetPalette(palette);
    return problems;
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::SetPalette

    The brushes are made again in the new colors, and the old ones deleted once the menus no longer use them.
    The dark check boxes are drawn again into their image lists, which the trees keep.
*/
void TWin32GUITheme::SetPalette(TWin32GUIPalette const& palette)
{
    m_Palette = palette;

    HBRUSH const oldBrushes[] = { m_MenuBrush, m_PanelBrush, m_WindowBrush };
    m_MenuBrush = CreateSolidBrush(m_Palette.MenuBackground);
    m_PanelBrush = CreateSolidBrush(m_Palette.Panel);
    m_WindowBrush = CreateSolidBrush(m_Palette.Window);

    for (std::pair<UINT, HIMAGELIST> const& entry : m_DarkCheckImages)
    {
        for (int state = 0; state < 4; ++state)
        {
            HBITMAP const bitmap = CreateDarkCheckImage(state, entry.first);

            if (bitmap != nullptr)
            {
                ImageList_Replace(entry.second, state, bitmap, nullptr);
                DeleteObject(bitmap);
            }
        }
    }

    if (m_Dark)
        Refresh();

    for (HBRUSH const brush : oldBrushes)
    {
        if (brush != nullptr)
            DeleteObject(brush);
    }
}
//---------------------------------------------------------------------------
COLORREF TWin32GUITheme::SkipTextColor() const
{
    return m_Dark ? m_Palette.SkipText : SkipTextColorOn(GetSysColor(COLOR_WINDOW));
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::Unattach

    A tree's own check box images, kept while it was dark, are destroyed with it, as a tree with check boxes
    leaves them to its owner.
*/
void TWin32GUITheme::Unattach(HWND window)
{
    m_Controls.erase(std::remove_if(m_Controls.begin(), m_Controls.end(), [window](TAttachedControl const& attached)
        {
            return attached.Control == window;
        }), m_Controls.end());
    m_Windows.erase(std::remove(m_Windows.begin(), m_Windows.end(), window), m_Windows.end());

    for (std::vector<std::pair<HWND, HIMAGELIST> >::iterator themed = m_ThemedCheckImages.begin();
         themed != m_ThemedCheckImages.end();)
    {
        if (themed->first == window)
        {
            ImageList_Destroy(themed->second);
            themed = m_ThemedCheckImages.erase(themed);
        }
        else
        {
            ++themed;
        }
    }

    if (window == m_MenuBarWindow)
    {
        // The menu bar is destroyed with its window.
        m_MenuBar = nullptr;
        m_MenuBarWindow = nullptr;
        m_MenuItems.clear();
    }
}
//---------------------------------------------------------------------------
COLORREF TWin32GUITheme::WindowColor() const
{
    return m_Dark ? m_Palette.Window : GetSysColor(COLOR_WINDOW);
}
//---------------------------------------------------------------------------
/*
    TWin32GUITheme::WindowProc

    An attached window's subclass. No exception may leave a window procedure; one that would leaves the
    message to the window.
*/
LRESULT CALLBACK TWin32GUITheme::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
    DWORD_PTR theme)
{
    try
    {
        if (message == WM_NCDESTROY)
        {
            reinterpret_cast<TWin32GUITheme*>(theme)->Unattach(window);
            RemoveWindowSubclass(window, &WindowProc, id);
            return DefSubclassProc(window, message, wParam, lParam);
        }

        return reinterpret_cast<TWin32GUITheme*>(theme)->HandleWindowMessage(window, message, wParam, lParam);
    }
    catch (...)
    {
        return DefSubclassProc(window, message, wParam, lParam);
    }
}
//---------------------------------------------------------------------------
COLORREF TWin32GUITheme::WindowTextColor() const
{
    return m_Dark ? m_Palette.WindowText : GetSysColor(COLOR_WINDOWTEXT);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
