#include <windows.h>
#include "resource.h"    // your ID_* and IDB_* macros
#include "MenuIcons.h"

// Declare ghwndFrame. Make sure to assign it the main frame window handle in your app initialization code.
extern HWND ghwndFrame;

// 1) Map each command‐ID in your .rc menus to the BITMAP resource ID.
typedef struct {
    UINT cmdID;
    UINT bmpRes;
} MenuIconEntry;

static const MenuIconEntry menuIconMap[] = {
    // File menu
    { ID_FIL_NEW,                   IDB_NEW },
    { ID_ACTION_NEWDIRECTORY,       IDB_CREATEDIR },
    { ID_VIEW_NEWWINDOWSLISTER,     IDB_TEXTVIEWER },
    { ID_FIL_OPEN,                  IDB_OPEN },
    { ID_FIL_CLOSE,                 IDB_CLOSE },
    { ID_FIL_EXIT,                  IDB_CLOSE },

    // Edit menu
    { ID_ACTION_DELETE,             IDB_DELETE },
    { ID_ACTION_UNDELETE,           IDB_UNDELETE },
    { ID_ACTION_RENAME,             IDB_RENAME },
    { ID_TOOLS_OPTIONS,             IDB_OPTIONS },
    { ID_EDIT_SELECTALL,            IDB_SELECTALL },
    { ID_EDIT_SELECTNONE,           IDB_SELECTNONE },
	{ ID_EDIT_INVERTSELECTION,      IDB_INVERTSELECTION },

    // Action menu
    { ID_ACTION_UPONELEVEL,         IDB_UPONELEVEL },
    { ID_FIL_INFORMATION,           IDB_INFO },
    { ID_ACTION_PROPERTIES,         IDB_PROPERTIES },

    // Tools menu
    { ID_TOOLS_TEXT_VIEWER,         IDB_TEXTVIEWER },
    { ID_TOOLS_BATCHCONVERTER,      IDB_BATCH },
    { ID_TOOLS_DISK2FDI,            IDB_DISK2FDI },
    { ID_TOOLS_INSTALL,             IDB_INSTALL },
    { ID_TOOLS_DISPLAYBOOTBLOCK,    IDB_DISPLAY },

    // View menu
    { ID_VIEW_SHOWUNDELETABLEFILES, IDB_SHOWUNDELETABLE },
    { ID_VIEW_REFRESH,              IDB_REFRESH },

    // Window menu
    { ID_WIN_CASCADE,               IDB_CASCADE },
    { ID_WIN_TILEHORIZONTAL,        IDB_TILEHOR },
    { ID_WIN_TILEVERTICAL,          IDB_TILEVER },
    { ID_WIN_CLOSEALL,              IDB_TILECLO },

    // Help menu
    { ID_HELP_ABOUT,                IDB_ABOUT },
};
    
static const size_t menuIconCount =
sizeof(menuIconMap) / sizeof(menuIconMap[0]);

static HBITMAP gMenuBitmaps[32]; // Enough for all menu items

// 2) The helper. Call it on a root HMENU (from LoadMenu or GetMenu) or a popup.
void SetMenuBitmaps(HINSTANCE hInst, HMENU hMenu)
{
    size_t i;
    for (i = 0; i < menuIconCount; ++i) {
        HBITMAP hBmp = (HBITMAP)LoadImage(
            hInst,
            MAKEINTRESOURCE(menuIconMap[i].bmpRes),
            IMAGE_BITMAP,
            0, 0,
            LR_DEFAULTSIZE | LR_CREATEDIBSECTION
        );
        if (!hBmp) {
            char buf[128];
            wsprintf(buf, "Failed to load bitmap resource %u", menuIconMap[i].bmpRes);
            MessageBox(NULL, buf, "Bitmap Load Error", MB_OK);
            continue;
        }
        gMenuBitmaps[i] = hBmp; // Store for later cleanup

        SetMenuItemBitmaps(
            hMenu,
            menuIconMap[i].cmdID,
            MF_BYCOMMAND,
            hBmp, hBmp
        );
    }
    DrawMenuBar(ghwndFrame);
}

// Call this on app exit:
void FreeMenuBitmaps()
{
    for (size_t i = 0; i < menuIconCount; ++i) {
        if (gMenuBitmaps[i]) {
            DeleteObject(gMenuBitmaps[i]);
            gMenuBitmaps[i] = NULL;
        }
    }
}