// MenuIcons.c
#include <windows.h>
#include "resource.h"    // your IDI_* macros
#include "MenuIcons.h"   // prototypes for these functions

// Padding around icon + text
static const int PAD_X = 8;  // horizontal padding on left, between icon/text, and right
static const int PAD_Y = 4;  // vertical padding top and bottom

// 1) Define your menu-icon map here:
typedef struct { UINT cmdID, icoRes; } MenuIconEntry;
static const MenuIconEntry menuIconMap[] = {
    // File menu
    { ID_FIL_NEW,                   IDI_NEW },
    { ID_ACTION_NEWDIRECTORY,       IDI_CREATEDIR },
    { ID_VIEW_NEWWINDOWSLISTER,     IDI_NEWLIST },
    { ID_FIL_OPEN,                  IDI_OPEN },
    { ID_FIL_CLOSE,                 IDI_CLOSE },
    { ID_FIL_EXIT,                  IDI_EXIT },

    // Edit menu
    { ID_ACTION_DELETE,             IDI_DELETE },
    { ID_ACTION_UNDELETE,           IDI_UNDELETE },
    { ID_ACTION_RENAME,             IDI_RENAME },
    { ID_TOOLS_OPTIONS,             IDI_OPTIONS },
    { ID_EDIT_SELECTALL,            IDI_SELECTALL },
    { ID_EDIT_SELECTNONE,           IDI_SELECTNONE },
    { ID_EDIT_INVERTSELECTION,      IDI_INVERTSELECTION },

    // Action menu
    { ID_ACTION_UPONELEVEL,         IDI_UPONELEVEL },
    { ID_FIL_INFORMATION,           IDI_INFO },
    { ID_ACTION_PROPERTIES,         IDI_PROPERTIES },

    // Tools menu
    { ID_TOOLS_TEXT_VIEWER,         IDI_TEXTVIEWER },
    { ID_TOOLS_HEX_VIEWER,          IDI_HEXVIEWER },
    { ID_TOOLS_BATCHCONVERTER,      IDI_BATCH },
    { ID_TOOLS_GREASEWEAZLE,        IDI_GREASEWEAZLE },
    { ID_TOOLS_GREASEWEAZLEWRITE,   IDI_GREASEWEAZLEWRITE },
    { ID_TOOLS_INSTALL,             IDI_INSTALL },
    { ID_TOOLS_WRITE_RAW_BOOTBLOCK, IDI_RAWWRITEBOOTBLOCK },
    { ID_TOOLS_DISPLAYBOOTBLOCK,    IDI_DISPLAY },

    // View menu
    { ID_VIEW_SHOWUNDELETABLEFILES, IDI_SHOWUNDELETABLE },
    { ID_VIEW_REFRESH,              IDI_REFRESH },

    // Window menu
    { ID_WIN_CASCADE,               IDI_CASCADE },
    { ID_WIN_TILEHORIZONTAL,        IDI_TILEHOR },
    { ID_WIN_TILEVERTICAL,          IDI_TILEVER },
    { ID_WIN_CLOSEALL,              IDI_TILECLO },

    // Help menu
    { ID_HELP_ABOUT,                IDI_ABOUT },
	{ IDM_HELP_LICENCE,             IDI_README },
    { IDM_HELP_README,              IDI_README },
    { IDM_HELP_CHM,                 IDI_HELP_CHM }             
};

#define MENU_ICON_COUNT  (sizeof(menuIconMap)/sizeof(menuIconMap[0]))

// State for owner-draw
static HMENU g_hCurrentMenu = NULL;
static HICON g_hIcons[MENU_ICON_COUNT];
static int   g_iconW, g_iconH;

///////////////////////////////////////////////////////////////////////////
// InitMenuIcons: call in WM_INITMENUPOPUP for each submenu
///////////////////////////////////////////////////////////////////////////
void InitMenuIcons(HINSTANCE hInst, HMENU hMenu)
{
    g_hCurrentMenu = hMenu;

    // Use the system small-icon metrics (16×16 on standard DPI)
    g_iconW = GetSystemMetrics(SM_CXSMICON);
    g_iconH = GetSystemMetrics(SM_CYSMICON);

    MENUITEMINFOA mii = { sizeof(mii) };
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_OWNERDRAW;

    for (size_t i = 0; i < MENU_ICON_COUNT; ++i)
    {
        HICON hIcon = (HICON)LoadImageA(
            hInst,
            MAKEINTRESOURCEA(menuIconMap[i].icoRes),
            IMAGE_ICON,
            g_iconW, g_iconH,
            LR_DEFAULTCOLOR | LR_CREATEDIBSECTION
        );
        if (!hIcon) continue;

        g_hIcons[i] = hIcon;

        // Mark this item OWNERDRAW
        SetMenuItemInfoA(
            hMenu,
            menuIconMap[i].cmdID,
            FALSE,
            &mii
        );
    }

    // If this is your main frame’s menu bar, force a redraw
    DrawMenuBar(GetActiveWindow());
}

///////////////////////////////////////////////////////////////////////////
// OnMeasureItem: owner-draw measurement with extra padding
///////////////////////////////////////////////////////////////////////////
void OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* mis)
{
    if (mis->CtlType != ODT_MENU || !g_hCurrentMenu) return;

    static HMENU lastMenu = NULL;
    static int   maxW = 0;

    HMENU hMenu = g_hCurrentMenu;
    UINT  cmd = mis->itemID;

    if (hMenu != lastMenu) {
        lastMenu = hMenu;
        maxW = 0;
    }

    for (size_t i = 0; i < MENU_ICON_COUNT; ++i)
    {
        if (menuIconMap[i].cmdID != cmd) continue;

        // Get the menu text
        char buf[128];
        int  len = GetMenuStringA(
            hMenu, cmd,
            buf, sizeof(buf),
            MF_BYCOMMAND
        );
        if (len < 0) len = 0;

        // Measure text
        HDC   hdc = GetDC(hWnd);
        HFONT oldF = (HFONT)SelectObject(
            hdc, GetStockObject(DEFAULT_GUI_FONT)
        );
        SIZE  sz;
        GetTextExtentPoint32A(hdc, buf, len, &sz);
        SelectObject(hdc, oldF);
        ReleaseDC(hWnd, hdc);

        // Compute width = left padding + icon + mid padding + text + right padding
        int thisW = PAD_X + g_iconW + PAD_X + sz.cx + PAD_X;
        if (thisW > maxW) maxW = thisW;

        mis->itemWidth = maxW;
        // Height = top padding + max(icon,text) + bottom padding
        int contentH = (g_iconH > sz.cy ? g_iconH : sz.cy);
        mis->itemHeight = PAD_Y + contentH + PAD_Y;
        return;
    }

    // Fallback for un-mapped items
    mis->itemWidth = 0;
    mis->itemHeight = 0;
}

///////////////////////////////////////////////////////////////////////////
// OnDrawItem: owner-draw rendering with extra padding
///////////////////////////////////////////////////////////////////////////
void OnDrawItem(HWND hWnd, DRAWITEMSTRUCT* dis)
{
    if (dis->CtlType != ODT_MENU || !g_hCurrentMenu) return;

    UINT  cmd = dis->itemID;
    BOOL  selected = (dis->itemState & ODS_SELECTED) != 0;
    BOOL  disabled = (dis->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0;

    // Check state from the real menu
    UINT  mstate = GetMenuState(g_hCurrentMenu, cmd, MF_BYCOMMAND);
    BOOL  checked = (mstate != 0xFFFFFFFF && (mstate & MF_CHECKED));

    // 1) Fill background
    HBRUSH hbr = CreateSolidBrush(
        selected
        ? GetSysColor(COLOR_HIGHLIGHT)
        : GetSysColor(COLOR_MENU)
    );
    FillRect(dis->hDC, &dis->rcItem, hbr);
    DeleteObject(hbr);

    // 2) Compute gutter rectangle with padding
    int x0 = dis->rcItem.left + PAD_X;
    int y0 = dis->rcItem.top + PAD_Y
        + ((dis->rcItem.bottom - dis->rcItem.top - PAD_Y * 2 - g_iconH) / 2);
    RECT rcGutter = { x0, y0, x0 + g_iconW, y0 + g_iconH };

    // 3) Draw icon if mapped
    BOOL drewIcon = FALSE;
    for (size_t i = 0; i < MENU_ICON_COUNT; ++i)
    {
        if (menuIconMap[i].cmdID != cmd) continue;
        drewIcon = TRUE;

        if (disabled)
        {
            DrawState(
                dis->hDC, NULL, NULL,
                (LPARAM)g_hIcons[i], 0,
                rcGutter.left, rcGutter.top,
                g_iconW, g_iconH,
                DST_ICON | DSS_DISABLED
            );
        }
        else
        {
            DrawIconEx(
                dis->hDC,
                rcGutter.left, rcGutter.top,
                g_hIcons[i],
                g_iconW, g_iconH,
                0, NULL,
                DI_NORMAL
            );
        }
        break;
    }

    // 4) If no icon but checked, draw standard checkmark
    if (!drewIcon && checked)
    {
        UINT dfcs = DFCS_MENUCHECK | DFCS_CHECKED;
        if (disabled) dfcs |= DFCS_INACTIVE;
        DrawFrameControl(dis->hDC, &rcGutter, DFC_MENU, dfcs);
    }

    // 5) Draw the text with padding
    char buf[128] = { 0 };
    int  len = GetMenuStringA(
        g_hCurrentMenu, cmd, buf, sizeof(buf), MF_BYCOMMAND
    );
    if (len < 0) len = 0;

    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC,
        disabled
        ? GetSysColor(COLOR_GRAYTEXT)
        : (selected
            ? GetSysColor(COLOR_HIGHLIGHTTEXT)
            : GetSysColor(COLOR_MENUTEXT))
    );

    RECT rcText = dis->rcItem;
    rcText.left = rcGutter.right + PAD_X;
    rcText.top = dis->rcItem.top + PAD_Y;
    rcText.bottom = dis->rcItem.bottom - PAD_Y;
    DrawTextA(
        dis->hDC,
        buf, -1,
        &rcText,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );
}

///////////////////////////////////////////////////////////////////////////
// CleanupMenuIcons: call on WM_DESTROY or after TrackPopupMenuEx
///////////////////////////////////////////////////////////////////////////
void CleanupMenuIcons(void)
{
    for (size_t i = 0; i < MENU_ICON_COUNT; ++i)
    {
        if (g_hIcons[i])
        {
            DestroyIcon(g_hIcons[i]);
            g_hIcons[i] = NULL;
        }
    }
    g_hCurrentMenu = NULL;
}