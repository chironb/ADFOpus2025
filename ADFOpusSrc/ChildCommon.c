/*
 * ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file ChildCommon.c
 *  \brief MDI child window functions.
 *
 * ChildCommon.c - Routines common to both types of child window
 */


#include <windows.h>
#include <commctrl.h>
#include <direct.h>     // for _chdir, _mkdir
#include <stdio.h>      // for sprintf


	
#include "ADFOpus.h"     // <-- must define CHILDINFO, newWinType, etc.
#include <windows.h>
#include <commctrl.h>    // for SetWindowSubclass / DefSubclassProc
#include <windowsx.h>    // for GET_X_LPARAM / GET_Y_LPARAM macros


#include <windows.h>





#include "MenuIcons.h"

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>

#include "ADFOpus.h"        // for newWinType, CHILD_WINLISTER, etc.
#include "ChildCommon.h"    // <-- declares typedef struct _CHILDINFO {...} CHILDINFO

#include "Bootblock.h"
extern ends_with(const char* str, const char* suffix);
extern BOOL ensure_extension(char* path, size_t buffer_size, const char* ext);
#include "Options.h"
extern struct OPTIONS Options;

extern HIMAGELIST ghwndImageList;
extern HINSTANCE instance;
extern HWND ghwndFrame;
extern BOOL gbIsDragging;
extern HWND ghwndDragSource;
extern HWND ghwndSB;
extern HWND	ghwndTB;
extern char gstrFileName[MAX_PATH * 2];
extern BOOL ReadOnly;

extern HCURSOR ghcurNormal;
extern HCURSOR ghcurNo;

extern int volToOpen;
extern struct OPTIONS Options;

// Chiron 2025: This is part of fixed to address an annoying issue that I suspect is 
// either a problem with the way the original codebase was setup or, it's Win32 issues.
// In any case if you left-click a file in one windows, and then right click a file
// in another window, it acts like you're trying to get the properies of the previous file.
// It's a stupid bug. Basically I tried to get it fixed so it's like every
// right-click triggers something like a left-click first. Like as if the user 
// left-clicked and then right-clicked. But nothing worked and it just broken everything. 
// So instead as you hover it automatically picks the window you're hovering over. 
// That means if you right-click a file your mouse is hovering over that windows 
// so it *HAS* to be selecting the correct file from the correct place. 
// 
// This is a KLUDGE of a fix! But it works and I've spent a lot of energy trying to 
// fix it "properly" in a way that matches modern file manager behaviour. 
// I've given up for now I have no idea what to do. 
// I'm new to Win32, Visual Studio 2022, and this codebase, so... I got nothin'!
// This works well enough and prevents behaviour that might lead to files being 
// damaged or deleted by accident or something so overall it's going to have to do!
//
// Unique ID for our hover subclass
#define HOVER_SUBCLASS_ID  0x100

static LRESULT CALLBACK
ListViewHoverProc(
	HWND      hwnd,        // the ListView window
	UINT      msg,
	WPARAM    wp,
	LPARAM    lp,
	UINT_PTR  uIdSubclass,
	DWORD_PTR dwRefData    // we pass CHILDINFO* here
)
{
	if (msg == WM_MOUSEMOVE)
	{
		// dwRefData is our CHILDINFO*
		CHILDINFO* ci = (CHILDINFO*)dwRefData;
		HWND       hChild = GetParent(hwnd);   // MDI‐child
		HWND       hMDI = GetParent(hChild); // MDI‐client

		// Only activate if it’s not already active
		HWND hActive = (HWND)SendMessage(hMDI, WM_MDIGETACTIVE, 0, 0);
		if (hActive != hChild)
		{
			SendMessage(hMDI, WM_MDIACTIVATE, (WPARAM)hChild, 0);
			EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_GRAYED); // <-- This doesn't work fuck you I hate you so much fuck! I've spent way too long trying to get these stupid fucking things to grey-out themselves! 
			EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_GRAYED); // <-- This doesn't work fuck you I hate you so much fuck! I've spent way too long trying to get these stupid fucking things to grey-out themselves! 
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(FALSE, 0));
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(FALSE, 0));
		}
	}

	return DefSubclassProc(hwnd, msg, wp, lp);
}



extern HINSTANCE instance;    // or however you name your HINSTANCE
extern HWND      ghwndFrame;  // your main window handle
extern BOOL      bDirClicked;
extern BOOL      bFileClicked;




// at the top of your .c/.cpp
#ifndef WINVER
#  define WINVER    0x0501    // XP+
#endif
#ifndef _WIN32_IE
#  define _WIN32_IE 0x0400    // ComCtl32 v6 APIs
#endif

#include <windows.h>
#include <commctrl.h>           // for SetWindowSubclass
#pragma comment(lib, "comctl32.lib")





#include <windows.h>
#include <commctrl.h>   // for SetWindowSubclass / DefSubclassProc



// Subclass callback – clamps the cursor after every WM_MOUSEMOVE
static LRESULT CALLBACK ListViewClampProc(
	HWND      hwnd,
	UINT      uMsg,
	WPARAM    wParam,
	LPARAM    lParam,
	UINT_PTR  uIdSubclass,
	DWORD_PTR dwRefData
)
{

	// Chiron 2025 - TODO: I think this is where there is a bug if you click down and 
	//                     start dragging the mouse it shoots way over to the 
	//                     bottom right corner of the screen. 
	//                     I think that this code must be responsible. 
	//                     There must be some math bug that causes this to happen.
	//                     But I'm not sure. 
	// UPDATE:             When I comment out this entire if statement, the bug remains. WTF?!?!?
	if (uMsg == WM_MOUSEMOVE)
	{
		// 1) Let the ListView scroll/drag‐select as usual
		LRESULT lr = DefSubclassProc(hwnd, uMsg, wParam, lParam);

		// 2) Clamp the cursor into the client rectangle
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);

		RECT rc;
		GetClientRect(hwnd, &rc);
		pt.x = pt.x < rc.left ? rc.left : (pt.x >= rc.right ? rc.right - 1 : pt.x);
		pt.y = pt.y < rc.top ? rc.top : (pt.y >= rc.bottom ? rc.bottom - 1 : pt.y);

		ClientToScreen(hwnd, &pt);
		SetCursorPos(pt.x, pt.y);

		return lr;
	}

	// For all other messages, just chain to default
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}








#include "Pch.h"

#include "ADFOpus.h"
#include "ChildCommon.h"
#include "ListView.h"
#include "Utils.h"
#include "VolSelect.h"
#include "Options.h"
#include <direct.h>

	
#include "ADFLib.h"
#include "ADF_err.h" ////////// this shouldn't be needed

// For conversions.
#include "xDMS.h"
#include "BatchConvert.h"
#include "zLib.h"

// Chiron 2025 - For the default folder being the users home folder instead of the C:\
//
#include <windows.h>
#include <stdlib.h>    // malloc
#include <string.h>    // strcpy_s, strncpy_s

extern HWND ghwndMDIClient;


//BOOL	bClicked = FALSE;
BOOL	bDirClicked = FALSE, bFileClicked = FALSE, bNothingClicked = FALSE;		// File or dir or nothing selection flags.
BOOL	bUndeleting = FALSE;	// Undeletion flag.

///////// could maybe do without this
long newWinType;

/* local function prototypes */
LRESULT ChildOnCreate(HWND);
BOOL ChildOnCommand(HWND, WPARAM, LPARAM);
void ChildOnPaint(HWND);
void ChildOnDestroy(HWND);
BOOL ChildOnNotify(HWND, WPARAM, LPARAM);
void ChildOnSize(HWND);
HWND CreateListView(HWND);
void ChildUpOneLevel(HWND);
void ChildSelectAll(HWND);
void ChildSelectNone(HWND);
void ChildInvertSelection(HWND);
void ChildUpdate(HWND);
void ChildClearContent(HWND);
void ChildDelete(HWND);
BOOL ChildShowUndeletable(HWND);
BOOL ChildUndelete(HWND);
void WinGetDir(HWND);
BOOL WinAddFile(CHILDINFO *, WIN32_FIND_DATA *);
void AmiGetDir(HWND);
void AmiAddFile(CHILDINFO *, struct List *);
void WinGetDrives(HWND);
void ChildSortDir(HWND, long);
void SwapContent(DIRENTRY *, DIRENTRY *);
BOOL ChildOnContextMenu(HWND, int, int);
void DisplayContextMenu(HWND, POINT);
BOOL ChildRename(HWND, LV_DISPINFO *);
BOOL ChildCheckRename(HWND, LV_DISPINFO *);
void ChildMakeDir(HWND);
BOOL RemoveDirectoryRecursive(char *);
BOOL RemoveAmiDirectoryRecursive(struct Volume *, SECTNUM, char *);

/***************************************************************************/

HWND CreateChildWin(HWND client, long type)
/*! creates and displays a new MDI child window 
*/
{
	MDICREATESTRUCT mcs;
	char title[MAX_PATH + 12];

	if (type == CHILD_AMILISTER) {
		strcpy(title, gstrFileName);
		if (ReadOnly)
			strcat(title, " [read-only]");
		volToOpen = -1;
		DialogBox(instance, MAKEINTRESOURCE(IDD_VOLSELECT), ghwndFrame, (DLGPROC)VolSelectProc);
		if (volToOpen == -1)
			return NULL;
	} else {
			strcpy(title, "Local Filesystem");
	}




	/* Initialize the MDI create struct */
	mcs.szClass = (type == CHILD_WINLISTER) ? WINLISTERCLASSNAME : AMILISTERCLASSNAME;
	mcs.szTitle = title;
	mcs.hOwner = instance;
	mcs.x = CW_USEDEFAULT;
	mcs.y = CW_USEDEFAULT;
	mcs.cx = CW_USEDEFAULT;
	mcs.cy = CW_USEDEFAULT;
	mcs.style = 0l;
	mcs.lParam  = 0l;

	newWinType = type;

	return (HWND)SendMessage(client, WM_MDICREATE, 0, (LONG)(LPMDICREATESTRUCT)&mcs);
}



LRESULT CALLBACK ChildWinProc(HWND win, UINT msg, WPARAM wp, LPARAM lp)
{
	CHILDINFO *ci;

	switch(msg)
	{
		case WM_CREATE:
			return ChildOnCreate(win);
			break;
		case WM_MDIACTIVATE:
			ci = (CHILDINFO *)GetWindowLong(win, 0);
			SetFocus(ci->lv);
			UpdateToolbar();
//			bClicked = FALSE;					// Reset the Properties context menu item.
			bDirClicked = bFileClicked = FALSE;					// Reset the Properties menu item.
			break;
		case WM_COMMAND:
			ChildOnCommand(win, wp, lp);
			break;
		case WM_PAINT:
			ChildOnPaint(win);
			break;
		case WM_NOTIFY:
			ChildOnNotify(win, wp, lp);
			break;
		case WM_DESTROY:
			ChildOnDestroy(win);
			break;
		case WM_CONTEXTMENU:
			if (! ChildOnContextMenu(win, LOWORD(lp), HIWORD(lp)))
				return DefMDIChildProc(win, msg, wp, lp);
			break;
		case WM_NCDESTROY:
			UpdateToolbar();
		case WM_SIZE:
			ChildOnSize(win);
			/* must fall though to DefMDIChildProc */
		default:
			return(DefMDIChildProc(win, msg, wp, lp));
	}
	return 0l;
}




#include "ADFOpus.h"    // for g_defaultLocalPath, newWinType, CHILD_WINLISTER
#include <windows.h>

LRESULT ChildOnCreate(HWND win)
{
	CHILDINFO* ci;

	// 1) remember which child‐type this is
	SetWindowLongPtr(win, GWLP_USERDATA, (LONG_PTR)newWinType);

	// 2) allocate per‐window state
	ci = malloc(sizeof * ci);
	if (!ci)
		return -1;
	SetWindowLongPtr(win, 0, (LONG_PTR)ci);

	// 3) initialize based on type
	if (newWinType == CHILD_WINLISTER)
	{
		// Use the folder we computed in WinMain (or %USERPROFILE%\ if no args).
		strcpy_s(ci->curDir, sizeof ci->curDir, g_defaultLocalPath);
		ci->atRoot = FALSE;

		// If other code assumes the process CWD matches, update it:
		SetCurrentDirectory(ci->curDir);
	}
	else  // CHILD_AMILISTER (your ADF‐lister)
	{
		strcpy_s(ci->curDir, sizeof ci->curDir, "/");
		ci->dfDisk = dfDisk;
		ci->compSize = comp_size;

		// MessageBoxA(win, gstrFileName, "DEBUG:gstrFileName", MB_OK );

		ci->dev = adfMountDev(gstrFileName, ReadOnly);
		if (!ci->dev) return -1;

		ci->vol = adfMount(ci->dev, volToOpen, FALSE);
		if (!ci->vol) return -1;

		ci->atRoot = TRUE;
	}

	// 4) common post‐init
	ci->readOnly = ReadOnly;

	// … after setting up ci …
	ci->lv = CreateListView(win);

	if (Options.autoPaneOnHover) {
		// Install hover-activate subclass on this ListView
		SetWindowSubclass(
			ci->lv,                   // the ListView HWND
			ListViewHoverProc,        // our subclass proc
			HOVER_SUBCLASS_ID,        // unique ID
			(DWORD_PTR)ci             // pass CHILDINFO* into dwRefData
		);
	};/*end-if*/

	// create the status bar at top of the child
	ci->sb = CreateWindow(
		STATUSCLASSNAME, "",
		WS_CHILD | WS_VISIBLE | CCS_TOP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		win, NULL, instance, NULL
	);

	// initialize other fields
	ci->content = NULL;
	ci->isAmi = (newWinType == CHILD_AMILISTER);

	// stash original path buffer if you use buf/gstrFileName later
	strcpy_s(ci->orig_path, sizeof ci->orig_path, buf);
	if (ci->dfDisk != (enum DiskFormat)ADF)
		strcpy_s(ci->temp_path, sizeof ci->temp_path, gstrFileName);

	// disable the Properties button until a selection is made
	{
		HMENU hMenu = GetMenu(ghwndFrame);
		EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
		SendMessage(
			ghwndTB,
			TB_ENABLEBUTTON,
			ID_ACTION_PROPERTIES,
			MAKELONG(FALSE, 0)
		);
	}

	// finally, populate the view
	ChildUpdate(win);

	return 0;
}



BOOL ChildOnCommand(HWND win, WPARAM wp, LPARAM lp)
{
    CHILDINFO *ci;

    // Forward these commands to the main frame window so right-click/context menu items work
    switch (wp)
    {
    case ID_TOOLS_TEXT_VIEWER:
	case ID_TOOLS_HEX_VIEWER:
    case ID_TOOLS_DISPLAYBOOTBLOCK:
    case ID_TOOLS_INSTALL:
	case ID_TOOLS_WRITE_RAW_BOOTBLOCK:
    case ID_FIL_INFORMATION:
    case ID_ACTION_PROPERTIES:
    case ID_TOOLS_BATCHCONVERTER:
    case ID_TOOLS_GREASEWEAZLE:
	case ID_TOOLS_GREASEWEAZLEWRITE:
    case ID_TOOLS_OPTIONS:
    case ID_HELP_ABOUT:
        SendMessage(ghwndFrame, WM_COMMAND, wp, lp);
        return TRUE;
    }

	switch(wp)
	{
	case ID_ACTION_UPONELEVEL:
		ChildUpOneLevel(win);
		break;
	case ID_EDIT_SELECTALL:
		ChildSelectAll(win);
		break;
	case ID_EDIT_SELECTNONE:
		ChildSelectNone(win);
		break;
	case ID_EDIT_INVERTSELECTION:
		ChildInvertSelection(win);
		break;
	case ID_VIEW_REFRESH:
		ChildUpdate(win);
		break;
	case ID_VIEW_SHOWUNDELETABLEFILES:
		ChildShowUndeletable(win);
		break;
	case ID_ACTION_DELETE:
		ChildDelete(win);
		break;
	case ID_ACTION_UNDELETE:
		ChildUndelete(win);
		break;
	case ID_ACTION_RENAME:
		ci = (CHILDINFO *)GetWindowLong(win, 0);
		ListView_EditLabel(ci->lv, LVGetItemFocused(ci->lv));
		break;
	case ID_ACTION_NEWDIRECTORY:
		ChildMakeDir(win);
		break;
//	case ID_TOOLS_TEXT_VIEWER:
//		break;
	}
	return TRUE;
}

void ChildOnPaint(HWND win)
/* standard do-nothing WM_PAINT handler */
{
	HDC dc;
	PAINTSTRUCT ps;

	dc = BeginPaint(win, &ps);
	if (dc)
		EndPaint(win, &ps);
}

void ChildOnDestroy(HWND win)
// Cleanup after child window. Delete temp adf and recompress if adz.
// Gets items from VolSelect.h and ADFOpus.h.
{
	CHILDINFO	*ci = (CHILDINFO *)GetWindowLong(win, 0);

	/* unmount volume and device if this an amiga lister */
///////////////////////FIXME - to allow for multiple views, etc.

	
	if (GetWindowLong(win, GWL_USERDATA) == CHILD_AMILISTER) {
		if (ci->vol) {
			adfUnMount(ci->vol);
		}
		if (ci->dev) {
			adfUnMountDev(ci->dev);
		}
	}

	// Chiron 2025
	// 
	//// Cleanup temp files. Recompress adz.
	//if(ci->dfDisk == ADZ){
	//	GZCompress(NULL, ci->temp_path, ci->orig_path);
	//	remove(ci->temp_path);
	//}
	//
	// Cleanup temp files. Recompress adz.
	if (ci->dfDisk == (enum DiskFormat)ADZ) {
		GZCompress(NULL, ci->temp_path, ci->orig_path);
		remove(ci->temp_path);
	}

	// Disable properties menu item and toolbar button, in case they've been left active.
	hMenu = GetMenu(ghwndFrame);
	EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));

	RemoveWindowSubclass(
		ci->lv,
		ListViewHoverProc,
		HOVER_SUBCLASS_ID
	);

	/* free the extra info we allocated when the window was created */
	free(ci);

}

void ChildOnSize(HWND win)
/* adjust the listview and status bar when window is resized */
{
	RECT winRec, sbRec;
	int sbHeight;
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);

	GetWindowRect(ci->sb, &sbRec);
	sbHeight = (sbRec.bottom - sbRec.top);

	GetClientRect(win, &winRec);
	MoveWindow(ci->lv, 0, sbHeight, winRec.right, winRec.bottom - sbHeight, TRUE);

	SendMessage(ci->sb, WM_SIZE, 0, 0l);
}

BOOL ChildOnNotify(HWND win, WPARAM wp, LONG lp)
/* process WM_NOTIFY events.  these will always come from the list-view
   control. */
{
	NMHDR		*nmhdr = (NMHDR *) lp;
	int			index;
	HMENU		hMenu;
	extern HWND	ghwndFrame;
	char		szError[MAX_PATH], szWinFile[MAX_PATH];
	int			iError, iSelectedType;
	BOOL		bIsFile = FALSE;


	ci = (CHILDINFO *)GetWindowLong(win, 0);			
	hMenu = GetMenu(ghwndFrame);
	switch(nmhdr->code){
		
		case NM_CLICK:
		{
			// NMITEMACTIVATE gives us the clicked row and point
			LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)nmhdr;
			index = pnmia->iItem;
			if (index < 0)
				break;  // clicked empty area

			// refine with hit-test
			LVHITTESTINFO hti = { 0 };
			hti.pt = pnmia->ptAction;
			ListView_SubItemHitTest(ci->lv, &hti);
			index = hti.iItem;
			if (index < 0)
				break;  // still empty

			// now existing single-click logic
			LVGetItemCaption(ci->lv, buf, sizeof(buf), index);
			iSelectedType = LVGetItemImageIndex(ci->lv, index);

			// drives return immediately
			switch (iSelectedType) {
			case ICO_DRIVEHD:
			case ICO_DRIVECD:
			case ICO_DRIVENET:
			case ICO_DRIVEFLOP35:
			case ICO_DRIVEFLOP514:
				return TRUE;
			}

			// file vs. dir
			switch (iSelectedType) {
			case ICO_AMIDIR:
			case ICO_WINDIR:
				bIsFile = FALSE;
				break;
			case ICO_AMIFILE:
			case ICO_WINFILE:
				bIsFile = TRUE;
				break;
			}

			// deleted‐file state
			if (ListView_GetItemState(ci->lv, index, LVIS_CUT)) {
				// enable Undelete
				EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_ENABLED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UNDELETE, MAKELONG(TRUE, 0));
				// disable everything else
				EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_GRAYED);
				EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
				EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
				EnableMenuItem(hMenu, ID_TOOLS_HEX_VIEWER, MF_GRAYED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(FALSE, 0));
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(FALSE, 0));
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
				bUndeleting = TRUE;
			}
			else {
				// normal item state
				EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_GRAYED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UNDELETE, MAKELONG(FALSE, 0));

				if (ListView_GetSelectedCount(ci->lv) > 0) {
					// enable Delete/Rename/Properties
					EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_ENABLED);
					EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_ENABLED);
					SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(TRUE, 0));
					SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(TRUE, 0));
					SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(TRUE, 0));

					if (!bIsFile) {
						// directory clicked
						SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
						EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
						SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_HEX_VIEWER, MAKELONG(FALSE, 0));
						EnableMenuItem(hMenu, ID_TOOLS_HEX_VIEWER, MF_GRAYED);
						bDirClicked = TRUE;
						bFileClicked = FALSE;
					}
					else {
						// file clicked
						SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(TRUE, 0));
						EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_ENABLED);
						SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_HEX_VIEWER, MAKELONG(TRUE, 0));
						EnableMenuItem(hMenu, ID_TOOLS_HEX_VIEWER, MF_ENABLED);
						bDirClicked = FALSE;
						bFileClicked = TRUE;
					}
				}
				bUndeleting = FALSE;
			}
			return TRUE;
		}

		case NM_DBLCLK:
		{
			LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)nmhdr;
			index = pnmia->iItem;
			if (index < 0)
				break;

			LVHITTESTINFO hti = { 0 };
			hti.pt = pnmia->ptAction;
			ListView_SubItemHitTest(ci->lv, &hti);
			index = hti.iItem;
			if (index < 0)
				break;

			// ignore deleted files
			if (ListView_GetItemState(ci->lv, index, LVIS_CUT))
				return TRUE;

			LVGetItemCaption(ci->lv, buf, sizeof(buf), index);
			// disable Properties while opening
			EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));

			bDirClicked = bFileClicked = FALSE;

			switch (LVGetItemImageIndex(ci->lv, index)) {
			case ICO_WINFILE: // Chiron 2025: TODO: This is where I want to check if it's an .adf file and then open it inside the program and not externally!
				strcpy(szWinFile, ci->curDir);
				strcat(szWinFile, buf);

					iError = (int)ShellExecute(win, "open", szWinFile, NULL, NULL, SW_SHOWNORMAL);
					ChildUpdate(win);
					break;

			case ICO_WINFILE_ADF:
				strcpy(szWinFile, ci->curDir);
				strcat(szWinFile, buf);


					// Open the .adf file within this program!
					strcpy(gstrFileName, szWinFile);
					// MessageBoxA(win, gstrFileName, "DEBUG:", MB_OK | MB_ICONERROR);
					CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
					break;

				

			case ICO_WINDIR:
				strcat(ci->curDir, buf);
				strcat(ci->curDir, "\\");
				ci->atRoot = FALSE;
				ChildUpdate(win);
				break;

			case ICO_AMIFILE:
				if (_chdir(dirTemp) == -1) {
					_mkdir(dirTemp);
					_chdir(dirTemp);
				}
				if (GetFileFromADF(ci->vol, buf) < 0) {
					sprintf(szError, "Error extracting %s from %s.", buf, ci->orig_path);
					MessageBox(win, szError, "ADF Opus Error", MB_ICONSTOP);
					break;
				}
				iError = (int)ShellExecute(win, "open", buf, NULL, NULL, SW_SHOWNORMAL);
				_chdir(dirOpus);
				if (iError == SE_ERR_NOASSOC) {
					sprintf(szError,
						"No association for %s. Register it in Windows or use the built-in viewer.",
						buf);
					MessageBox(win, szError, "ADF Opus Error", MB_ICONINFORMATION);
				}
				else if (iError <= 32) {
					sprintf(szError, "Unable to open %s under Windows.", buf);
					MessageBox(win, szError, "ADF Opus Error", MB_ICONEXCLAMATION);
				}
				ChildUpdate(win);
				break;

			case ICO_AMIDIR:
				strcat(ci->curDir, buf);
				strcat(ci->curDir, "/");
				ci->atRoot = FALSE;
				adfChangeDir(ci->vol, buf);
				ChildUpdate(win);
				break;

			case ICO_DRIVEHD:
			case ICO_DRIVECD:
			case ICO_DRIVENET:
			case ICO_DRIVEFLOP35:
			case ICO_DRIVEFLOP514:
				strcpy(ci->curDir, buf);
				strcat(ci->curDir, ":\\");
				ci->atRoot = FALSE;
				ChildUpdate(win);
				break;
			}
			return TRUE;
		}

		case NM_KILLFOCUS:
			// Disable properties menu item and toolbar button when the current window loses
			// focus, in case they've been left active.
			EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
			EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
			EnableMenuItem(hMenu, ID_TOOLS_HEX_VIEWER, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_HEX_VIEWER, MAKELONG(FALSE, 0));
//			bClicked = FALSE;											// Deactivate the Properties context menu item.
			bDirClicked = bFileClicked = FALSE;							// Deactivate the context menu items...
			// ..and undeletion items.
			EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_GRAYED);
			EnableMenuItem(hMenu, ID_VIEW_SHOWUNDELETABLEFILES, MF_GRAYED);
			break;

		case LVN_BEGINDRAG:
			/* user started dragging an item */
			SetCursor(ghcurNo);
			ghwndDragSource = win;
			SetCapture(ghwndFrame);
			gbIsDragging = TRUE;
			break;
		case LVN_BEGINLABELEDIT:
			/* stop user from editing things that can't be edited */
///////////////// THIS DOESN'T WORK!  Find out why and fix it.
			return ChildCheckRename(win, (LV_DISPINFO *)lp);
		case LVN_ENDLABELEDIT:
			/* an item was renamed */
			return ChildRename(win, (LV_DISPINFO *)lp);
	}
	return TRUE;
}


HWND CreateListView(HWND win)
{
	HWND lv = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		"",
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE |
		LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER,
		0, 0, 300, 100,
		win,
		NULL,
		instance,
		NULL
	);
	if (!lv)
		return NULL;

	// Add your columns
	LVAddColumn(lv, "Name", 150, 0); // Chiron 2025 - Kludgey fix for the MOUSE SHOOTS OFF bug! Changed the width of the Name column from 200 to 150.
	LVAddColumn(lv, "Size", 65, 1);
	LVAddColumn(lv, "Flags", 75, 2);
	LVAddColumn(lv, "Date", 90, 3);

	if (GetWindowLong(win, GWL_USERDATA) != CHILD_WINLISTER)
		LVAddColumn(lv, "Comment", 65, 4); 

	// Image list + full-row selection
	ListView_SetImageList(lv, ghwndImageList, LVSIL_SMALL);
	ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);

	// Install our subclass so the cursor never flies off
	SetWindowSubclass(lv,
		ListViewClampProc,
		/* uIdSubclass */ 1,
		/* dwRefData    */ 0);

	return lv;
}




//void ChildUpdate(HWND win)
///* fills the list view control with the directory content */
//{
//	CHILDINFO	*ci = (CHILDINFO *)GetWindowLong(win, 0);
//	DIRENTRY	*ce;
//	char		strBuf[20];
//	int			pos;
//	struct File *amiFile;
//	BOOL		bAmi = FALSE;
//
//
//	SetWindowText(ghwndSB, "Reading directory...");
//
//	if (GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER)
//		if (strcmp(ci->curDir, "") == 0)
//			WinGetDrives(win);
//		else
//			WinGetDir(win);
//	
//	else{
//		bAmi = TRUE;
//		AmiGetDir(win);
//	}
//	ChildSortDir(win, 0l);
//
//	SendMessage(ci->lv, WM_SETREDRAW, FALSE, 0);
//
//	ListView_DeleteAllItems(ci->lv);
//
//	ListView_SetItemCount(ci->lv, ci->totalCount);
//
//	ce = ci->content;
//	while (ce != NULL) {
//		pos = LVAddItem(ci->lv, ce->name, ce->icon);
//		if (pos == -1)
//			pos++;
//		if (ce->icon == ICO_WINFILE || ce->icon == ICO_AMIFILE) {
//			itoa(ce->size, strBuf, 10);
//			LVAddSubItem(ci->lv, strBuf, pos, 1);
//		}
//		LVAddSubItem(ci->lv, ce->flags, pos, 2);
//
//		// Display amiga file comment.
//		if(bAmi){
//			amiFile = adfOpenFile(ci->vol, ce->name, "r");
//			LVAddSubItem(ci->lv, amiFile->fileHdr->comment, pos, 3);
//			adfCloseFile(amiFile);
//			LVAddSubItem(ci->lv, "Sep/08/2025 8:03:23 AM", pos, 4);
//		} else {
//			LVAddSubItem(ci->lv, "Sep/08/2025 8:03:23 AM", pos, 3);
//		}
//
//		ce = ce->next;
//	}
//
//	SendMessage(ci->lv, WM_SETREDRAW, TRUE, 0);
//	InvalidateRect(ci->lv, NULL, FALSE);
//
//	/* update status bars */
//	if (! strcmp(ci->curDir, ""))
//		SetWindowText(ci->sb, "All drives");
//	else
//		SetWindowText(ci->sb, ci->curDir);
//	UpdateToolbar();
//	//SetWindowText(ghwndSB, "Idle");
//	SetWindowText(ghwndSB, "Welcome to ADF Opus 2025!");
//
//}



#include <windows.h>

// Converts an Amiga timestamp (days/minutes/ticks since 1978-01-01) into
// a local SYSTEMTIME.  You can then format that SYSTEMTIME with
// GetDateFormat()/GetTimeFormat() or similar.
void AmiDateToSystemTime(
	LONG        days,    // days since 1978-01-01
	LONG        mins,    // minutes since midnight (0–1439)
	LONG        ticks,   // 1/50ths of a second past that minute (0–49)
	SYSTEMTIME* pSt      // out: local date/time
)
{
	// 1) Build a FILETIME for the base date: 1978-01-01 00:00:00
	SYSTEMTIME stBase = { 0 };
	stBase.wYear = 1978;
	stBase.wMonth = 1;
	stBase.wDay = 1;
	// wHour, wMinute, wSecond, wMilliseconds all zero

	FILETIME ftBase;
	SystemTimeToFileTime(&stBase, &ftBase);

	// 2) Turn that into a 64-bit count of 100-ns intervals
	ULARGE_INTEGER uli;
	uli.LowPart = ftBase.dwLowDateTime;
	uli.HighPart = ftBase.dwHighDateTime;

	// 3) Add the Amiga offset in 100-ns units
	const ULONGLONG DAY_100NS = 86400ULL * 10000000ULL;
	const ULONGLONG MIN_100NS = 60ULL * 10000000ULL;
	const ULONGLONG TICK_100NS = 10000000ULL / 50ULL;

	uli.QuadPart += (ULONGLONG)days * DAY_100NS;
	uli.QuadPart += (ULONGLONG)mins * MIN_100NS;
	uli.QuadPart += (ULONGLONG)ticks * TICK_100NS;

	// 4) Convert back to FILETIME
	FILETIME ftTarget;
	ftTarget.dwLowDateTime = uli.LowPart;
	ftTarget.dwHighDateTime = uli.HighPart;

	// 5) Convert UTC FILETIME → local FILETIME → local SYSTEMTIME
	FILETIME ftLocal;
	FileTimeToLocalFileTime(&ftTarget, &ftLocal);
	FileTimeToSystemTime(&ftLocal, pSt);
}

#include <windows.h>

// Formats a raw byte count into a human‐readable string.
//
//   bytes   – the size in bytes
//   buf     – output buffer, at least 64 chars
//
// Examples of output:
//   “512 bytes”
//   “1.2 KB (1234 bytes)”
//   “3.4 MB (3562345 bytes)”
//   “5.6 GB (5987654321 bytes)”
void FormatByteSize(
	unsigned long bytes,
	char* buf
)
{
	const unsigned long KB = 1024UL;
	const unsigned long MB = 1024UL * KB;
	const unsigned long GB = 1024UL * MB;

	if (bytes < KB) {
		// under 1 KB: raw bytes
		wsprintfA(buf, "%lu bytes", bytes);
	}
	else if (bytes < MB) {
		// 1 KB–1 MB: show KB with one decimal
		unsigned long whole = bytes / KB;
		unsigned long frac = (bytes % KB) * 10 / KB;  // tenths
		//wsprintfA(buf, "%lu.%lu KB (%lu bytes)", whole, frac, bytes);
		wsprintfA(buf, "%lu.%lu KB", whole, frac, bytes);
	}
	else if (bytes < GB) {
		// 1 MB–1 GB: show MB with one decimal
		unsigned long whole = bytes / MB;
		unsigned long frac = (bytes % MB) * 10 / MB;
		//wsprintfA(buf, "%lu.%lu MB (%lu bytes)", whole, frac, bytes);
		wsprintfA(buf, "%lu.%lu MB", whole, frac, bytes);
	}
	else {
		// 1 GB+: show GB with one decimal
		unsigned long whole = bytes / GB;
		unsigned long frac = (bytes % GB) * 10 / GB;
		//wsprintfA(buf, "%lu.%lu GB (%lu bytes)", whole, frac, bytes);
		wsprintfA(buf, "%lu.%lu GB", whole, frac, bytes);
	}
}

//----------------------------------------------------------------------------
// Drop-in replacement for ChildUpdate that fills in real date/time
//----------------------------------------------------------------------------

void ChildUpdate(HWND win)
{
	CHILDINFO* ci = (CHILDINFO*)GetWindowLong(win, 0);
	DIRENTRY* ce;
	char         strBuf[20];
	int          pos;
	struct File* amiFile;
	BOOL         bAmi = FALSE;

	// Scratch buffers for date/time formatting
	TCHAR        dateBuf[64], timeBuf[64], dateTimeBuf[128];
	SYSTEMTIME   st;
	FILETIME     ftLocal;
	WIN32_FILE_ATTRIBUTE_DATA fad;
	char         fullPath[MAX_PATH];

	SetWindowText(ghwndSB, "Reading directory...");

	// Populate ci->content with either Windows or Amiga directory listing
	if (GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER) {
		if (strcmp(ci->curDir, "") == 0)
			WinGetDrives(win);
		else
			WinGetDir(win);
	}
	else {
		bAmi = TRUE;
		AmiGetDir(win);
	}

	ChildSortDir(win, 0l);
	SendMessage(ci->lv, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(ci->lv);
	ListView_SetItemCount(ci->lv, ci->totalCount);

	ce = ci->content;
	while (ce)
	{
		pos = LVAddItem(ci->lv, ce->name, ce->icon);
		if (pos < 0) pos = 0;

		// Size column
		if (ce->icon == ICO_WINFILE || ce->icon == ICO_AMIFILE)
		{
			//itoa((int)ce->size, strBuf, 10);
			//LVAddSubItem(ci->lv, strBuf, pos, 1);
			char strBuf[64];
			FormatByteSize((unsigned long)ce->size, strBuf);
			LVAddSubItem(ci->lv, strBuf, pos, /*columnIndex*/1);

		}

		// Flags column
		LVAddSubItem(ci->lv, ce->flags, pos, 2);

		if (bAmi)
		{
			// 1) Comment
			amiFile = adfOpenFile(ci->vol, ce->name, "r");
			LVAddSubItem(ci->lv,
				amiFile->fileHdr->comment,
				pos,
				4);

			// 2) Date/Time from Amiga header → SYSTEMTIME
			AmiDateToSystemTime(
				amiFile->fileHdr->days,
				amiFile->fileHdr->mins,
				amiFile->fileHdr->ticks,
				&st);
			adfCloseFile(amiFile);

			// 3) Format as "Sep/08/2025 8:03:23 AM"
			GetDateFormat(LOCALE_USER_DEFAULT,
				0,
				&st,
				TEXT("MMM/dd/yyyy"),
				dateBuf,
				ARRAYSIZE(dateBuf));

			GetTimeFormat(LOCALE_USER_DEFAULT,
				0,
				&st,
				TEXT("h:mm:ss tt"),
				timeBuf,
				ARRAYSIZE(timeBuf));

			wsprintf(dateTimeBuf, TEXT("%s %s"), dateBuf, timeBuf);
			LVAddSubItem(ci->lv, dateTimeBuf, pos, 3);
		}
		else
		{
			// 1) Build full Windows path to the file
			wsprintfA(fullPath,
				"%s\\%s",
				ci->curDir,
				ce->name);

			// 2) Read last-write timestamp
			if (GetFileAttributesExA(fullPath,
				GetFileExInfoStandard,
				&fad))
			{
				FileTimeToLocalFileTime(&fad.ftLastWriteTime,
					&ftLocal);
				FileTimeToSystemTime(&ftLocal, &st);

				// 3) Format date/time
				GetDateFormat(LOCALE_USER_DEFAULT,
					0,
					&st,
					TEXT("MMM/dd/yyyy"),
					dateBuf,
					ARRAYSIZE(dateBuf));

				GetTimeFormat(LOCALE_USER_DEFAULT,
					0,
					&st,
					TEXT("h:mm:ss tt"),
					timeBuf,
					ARRAYSIZE(timeBuf));

				wsprintf(dateTimeBuf, TEXT("%s %s"), dateBuf, timeBuf);
				LVAddSubItem(ci->lv, dateTimeBuf, pos, 3);
			}
			else
			{
				// on failure, leave blank
				LVAddSubItem(ci->lv, TEXT(""), pos, 3);
			}
		}

		ce = ce->next;
	}

	SendMessage(ci->lv, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(ci->lv, NULL, FALSE);

	/* update status bars */
	if (!strcmp(ci->curDir, ""))
		SetWindowText(ci->sb, "All drives");
	else
		SetWindowText(ci->sb, ci->curDir);

	UpdateToolbar();
	SetWindowText(ghwndSB, "Welcome to ADF Opus 2025!");
}



void WinGetDir(HWND win)
/* fills the internal directory list with the contents of the current windows dir */
{
	WIN32_FIND_DATA wfd;
	HANDLE search;
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	char searchPath[MAX_PATH];

	ChildClearContent(win);

	sprintf(searchPath, "%s*", ci->curDir);

	search = FindFirstFile(searchPath, &wfd);
	if (search == INVALID_HANDLE_VALUE)
		return;

	if (WinAddFile(ci, &wfd))
		ci->totalCount++;

	while (FindNextFile(search, &wfd))
		if (WinAddFile(ci, &wfd))
			ci->totalCount++;

	FindClose(search);
}

BOOL WinAddFile(CHILDINFO *ci, WIN32_FIND_DATA *wfd)
/* returns TRUE if a file was actually added (not . or ..), FALSE otherwise
 */
{
	DIRENTRY *de;

	if ((strcmp(wfd->cFileName, ".") == 0) || (strcmp(wfd->cFileName, "..") == 0))
		return FALSE;

	de = malloc(sizeof(DIRENTRY));
	strcpy(de->name, wfd->cFileName);
	de->size = wfd->nFileSizeLow;
	
	//de->icon = ((wfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		//? ICO_WINDIR : ICO_WINFILE;

	if (wfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		de->icon = ICO_WINDIR;
	}
	else {
		de->icon = ICO_WINFILE;
	}

	if (ends_with(wfd->cFileName, ".adf"))
		de->icon = ICO_WINFILE_ADF;

	// Chiron 2025: TODO: I think this is where I have to put somethign to check and set 
	// a special icon if it's an .adf file in a Windows List View. 
	// so when you double-click an .adf in a Windows List View
	// it opens the file inside the program. I just want it to show the icon!

	strcpy(de->flags, "");
	if (wfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		strcat(de->flags, "R");
	if (wfd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		strcat(de->flags, "H");
	if (wfd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
		strcat(de->flags, "S");
	if (wfd->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		strcat(de->flags, "A");

	de->next = ci->content;
	ci->content = de;

	return TRUE;
}

void AmiGetDir(HWND win)
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	struct List *list;

	ChildClearContent(win);

	list = adfGetDirEnt(ci->vol, ci->vol->curDirPtr);

	while (list) {
		AmiAddFile(ci, list);
		adfFreeEntry(list->content);
		list = list->next;
		ci->totalCount++;
	}
	freeList(list);
}

void AmiAddFile(CHILDINFO *ci, struct List *list)
{
	DIRENTRY *de = malloc(sizeof(DIRENTRY));
	struct Entry *ent = (struct Entry *)list->content;

	strcpy(de->name, ent->name);
	de->size = ent->size;
	de->icon = ent->type == ST_FILE ? ICO_AMIFILE : ICO_AMIDIR;
	strcpy(de->flags, "");
	if (!hasR(ent->access))
		strcat(de->flags, "R");
	if (!hasW(ent->access))
		strcat(de->flags, "W");
	if (!hasE(ent->access))
		strcat(de->flags, "E");
	if (!hasD(ent->access))
		strcat(de->flags, "D");
	if (hasS(ent->access))
		strcat(de->flags, "S");
	if (hasA(ent->access))
		strcat(de->flags, "A");
	if (hasP(ent->access))
		strcat(de->flags, "P");
	if (hasH(ent->access))
		strcat(de->flags, "H");

	de->next = ci->content;
	ci->content = de;
}

void ChildClearContent(HWND win)
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	DIRENTRY *ce, *cd;

	ce = ci->content;

	while (ce != NULL) {
		cd = ce;
		ce = ce->next;
		free(cd);
	}

	ci->content = NULL;
	ci->totalCount = 0;
}

void ChildUpOneLevel(HWND win)
/* go to the parent directory, or display the drive list if we are already
 * at the root directory
 */
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	int i, max;
	BOOL isAmi = GetWindowLong(win, GWL_USERDATA) == CHILD_AMILISTER;

	if (ci->atRoot)
		return;

	max = strlen(ci->curDir);

	if (max < (isAmi ? 2 : 4)) { /* at root */
		if (! isAmi)
			strcpy(ci->curDir, "");
		ci->atRoot = TRUE;
	}
	else{
		/* replace last \ character in pathname with a null */
		for (i = max - 2 ; i >= 0 ; i--) {
			if (ci->curDir[i] == (isAmi ? '/' : '\\')) {
				ci->curDir[i + 1] = 0;
				// Fix the above "up one level" button problem.
				if(strcmp(ci->curDir, "/") == 0)
					ci->atRoot = TRUE;				
				break;
			}

		}
		if (isAmi)
			adfParentDir(ci->vol);
	}

	// Deactivate the Properties context menu item, menu and toolbar items.
//	bClicked = FALSE;
	bDirClicked = bFileClicked = FALSE;							// Deactivate the context menu items...
	EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
	EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
	EnableMenuItem(hMenu, ID_TOOLS_HEX_VIEWER, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_HEX_VIEWER, MAKELONG(FALSE, 0));

	ChildUpdate(win);
}

void WinGetDrives(HWND win)
/* get available windows drives
 */
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	DIRENTRY *de;
	char driveLetter[2];
	DWORD dm;
	char i;
	char driveSpec[4];

	driveLetter[1] = 0; /* null-terminator */

	ChildClearContent(win);

	dm = GetLogicalDrives();

	for (i = 25 ; i >= 0 ; i--) {
		if (dm & (1 << i)) {
			driveLetter[0] = 'A' + i;
			de = malloc(sizeof(DIRENTRY));
			de->size = 0;
			strcpy(de->name, driveLetter);

			sprintf(driveSpec, "%c:", driveLetter[0]);

			switch (GetDriveType(driveSpec)) {
			case DRIVE_REMOVABLE:
				de->icon = ICO_DRIVEFLOP35;
				break;
			case DRIVE_REMOTE:
				de->icon = ICO_DRIVENET;
				break;
			case DRIVE_CDROM:
				de->icon = ICO_DRIVECD;
				break;
			default:
				de->icon = ICO_DRIVEHD;
			}

			strcpy(de->flags, "");
			de->next = ci->content;
			ci->content = de;
			ci->totalCount++;
		}
	}
}

void ChildSelectAll(HWND win)
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);

	LVSelectAll(ci->lv);
	SendMessage(win, WM_MDIACTIVATE, 0, 0l);
}

void ChildSelectNone(HWND win)
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);

	LVSelectNone(ci->lv);
	SendMessage(win, WM_MDIACTIVATE, 0, 0l);
}

void ChildInvertSelection(HWND win)
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);

	LVInvert(ci->lv);
	SendMessage(win, WM_MDIACTIVATE, 0, 0l);
}

void ChildSortDir(HWND win, long type)
/* My entry for the "shittest sorting code ever" championships
 */
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	DIRENTRY *de1, *de2;
	char t1[MAX_PATH], t2[MAX_PATH];

	if (ci->content == NULL)
		return;

	de1 = de2 = ci->content;

	/* sort by icon (type) first */
	if (de1->icon < ICO_DRIVEHD) { /* only if this isn't a drive list */
		while (de1) {
			de2 = ci->content;
			while (de2) {
				if (de1->icon > de2->icon)
					SwapContent(de1, de2);
				de2 = de2->next;
			}
			de1 = de1->next;
		}
	}

	/* now items with same icon by name */
	de1 = de2 = ci->content;

	while (de1) {
		de2 = ci->content;
		while (de2) {
			strcpy(t1, de1->name);
			strcpy(t2, de2->name);
			strupr(t1);
			strupr(t2);
			if ((de1->icon == de2->icon) && (strcmp(t1, t2) < 0))
				SwapContent(de1, de2);
			de2 = de2->next;
		}
		de1 = de1->next;
	}
}

void SwapContent(DIRENTRY *de1, DIRENTRY *de2)
/* the lamest function in this program
 */
{
	DIRENTRY tmpde;

	strcpy(tmpde.name, de1->name);
	strcpy(tmpde.flags, de1->flags);
	tmpde.size = de1->size;
	tmpde.icon = de1->icon;

	strcpy(de1->name, de2->name);
	strcpy(de1->flags, de2->flags);
	de1->size = de2->size;
	de1->icon = de2->icon;

	strcpy(de2->name, tmpde.name);
	strcpy(de2->flags, tmpde.flags);
	de2->size = tmpde.size;
	de2->icon = tmpde.icon;
}

BOOL ChildOnContextMenu(HWND win, int x, int y)
/* on context-click decide if we should display a context menu
 */
{
	RECT rec;
	POINT pt = { x, y };

	GetClientRect(win, &rec); 

	ScreenToClient(win, &pt);
 
	if (PtInRect(&rec, pt)) {
		ClientToScreen(win, &pt);
		DisplayContextMenu(win, pt);
		return TRUE;
	}
 
	return FALSE;
}


#include <windows.h>
#include "resource.h"
#include "MenuIcons.h"

extern HINSTANCE instance;
extern HWND      ghwndFrame;
extern BOOL      bDirClicked, bFileClicked, bNothingClicked;
//extern ContextInfo* ci;

//void DisplayContextMenu(HWND winLister, POINT pt)
//{
//	// 1) Load the bar resource and get its first submenu
//	HMENU menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_LISTERMENU));
//	if (!menu) return;
//	HMENU popup = GetSubMenu(menu, 0);
//	if (!popup) { DestroyMenu(menu); return; }
//
//	// 2) Reserve the icon/check gutter on the popup itself
//	MENUINFO mi = { sizeof(mi) };
//	GetMenuInfo(popup, &mi);
//	mi.fMask = MIM_STYLE;
//	mi.dwStyle = mi.dwStyle | MNS_CHECKORBMP;
//	SetMenuInfo(popup, &mi);
//
//	// 3) Owner-draw setup (loads the icons & marks the IDs you mapped)
//	InitMenuIcons(instance, popup);
//
//	// 4) Enable/disable your items on that same popup
//	EnableMenuItem(popup,
//		ID_ACTION_PROPERTIES,
//		(bDirClicked || bFileClicked) ? MF_ENABLED : MF_GRAYED
//	);
//	// …and the rest of your logic…
//
//	// 5) Make sure the frame is foreground (necessary for ContextMenus)
//	SetForegroundWindow(ghwndFrame);
//
//	// 6) Show the popup *with* the frame as the owner
//	TrackPopupMenuEx(
//		popup,
//		TPM_LEFTALIGN | TPM_RIGHTBUTTON,
//		pt.x, pt.y,
//		ghwndFrame,    // ← IMPORTANT: frame handles Measure/Draw
//		NULL
//	);
//
//	// 7) Post a zero-msg so the menu will dismiss correctly
//	PostMessage(ghwndFrame, WM_NULL, 0, 0);
//
//	// 8) Cleanup
//	CleanupMenuIcons();
//	DestroyMenu(menu);
//}



void DisplayContextMenu(HWND winLister, POINT ptScreen)
{
	// ---- Smart right-click selection: preserve multi-select or clear on empty ----
	{
		POINT pt = ptScreen;
		ScreenToClient(ci->lv, &pt);

		LVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		ListView_SubItemHitTest(ci->lv, &hti);

		if (hti.iItem < 0) {
			// clicked empty space → clear all selection
			ListView_SetItemState(ci->lv,
				-1,
				0,
				LVIS_SELECTED | LVIS_FOCUSED);
			bDirClicked = bFileClicked = FALSE;
			bNothingClicked = TRUE;
			buf[0] = '\0';
		}
		else {
			// clicked on an item
			UINT state = ListView_GetItemState(ci->lv,
				hti.iItem,
				LVIS_SELECTED);
			if (!(state & LVIS_SELECTED)) {
				// it wasn’t selected → clear others and select this one
				ListView_SetItemState(ci->lv,
					-1,
					0,
					LVIS_SELECTED | LVIS_FOCUSED);
				ListView_SetItemState(ci->lv,
					hti.iItem,
					LVIS_SELECTED | LVIS_FOCUSED,
					LVIS_SELECTED | LVIS_FOCUSED);
			}

			// populate buf for downstream dialogs
			LVGetItemCaption(ci->lv, buf, sizeof(buf), hti.iItem);

			// set flags for EnableMenuItem logic
			int img = LVGetItemImageIndex(ci->lv, hti.iItem);
			bFileClicked = (img == ICO_AMIFILE || img == ICO_WINFILE);
			bDirClicked = !bFileClicked;
			bNothingClicked = FALSE;
		}
	}
	// ------------------------------------------------------------------------------

	// 1) Load the menu resource and grab its first submenu
	HMENU menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_LISTERMENU));
	if (!menu) return;
	HMENU popup = GetSubMenu(menu, 0);
	if (!popup) { DestroyMenu(menu); return; }

	// 2) Reserve the icon/check gutter
	MENUINFO mi = { sizeof(mi) };
	GetMenuInfo(popup, &mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = mi.dwStyle | MNS_CHECKORBMP;
	SetMenuInfo(popup, &mi);

	// 3) Owner-draw setup
	InitMenuIcons(instance, popup);


	// 4) Enable/disable items based on the (possibly multi-)selection

	// Delete
	EnableMenuItem(popup,
		ID_ACTION_DELETE,
		(bDirClicked || bFileClicked)
		? MF_ENABLED
		: MF_GRAYED
	);

	// Rename
	EnableMenuItem(popup,
		ID_ACTION_RENAME,
		(bDirClicked || bFileClicked)
		? MF_ENABLED
		: MF_GRAYED
	);

	// Text Viewer
	EnableMenuItem(popup,
		ID_TOOLS_TEXT_VIEWER,
		bFileClicked
		? MF_ENABLED
		: MF_GRAYED
	);

	// Hex Viewer
	EnableMenuItem(popup,
		ID_TOOLS_HEX_VIEWER,
		bFileClicked
		? MF_ENABLED
		: MF_GRAYED
	);

	// Properties
	EnableMenuItem(popup,
		ID_ACTION_PROPERTIES,
		(bDirClicked || bFileClicked)
		? MF_ENABLED
		: MF_GRAYED
	);

	//  [0] MENUITEM SEPARATOR
//  [1] MENUITEM "&New Directory",              ID_ACTION_NEWDIRECTORY
//  [2] MENUITEM SEPARATOR
//  [3] MENUITEM "&Text Viewer",                ID_TOOLS_TEXT_VIEWER
//  [4] MENUITEM "&HEX Viewer",                 ID_TOOLS_HEX_VIEWER
//  [5] MENUITEM SEPARATOR
//  [6] MENUITEM "&Rename",                     ID_ACTION_RENAME
//  [7] MENUITEM "&Delete",                     ID_ACTION_DELETE
//  [8] MENUITEM SEPARATOR
//  [9] MENUITEM "&Select All",                 ID_EDIT_SELECTALL
// [10] MENUITEM "&Unselect All",               ID_EDIT_SELECTNONE
// [11] MENUITEM "&Invert Selection",           ID_EDIT_INVERTSELECTION
// [12] MENUITEM SEPARATOR
// [13] MENUITEM "&Properties",                 ID_ACTION_PROPERTIES
// [14] MENUITEM SEPARATOR
// [15] MENUITEM "Batch Converter",             ID_TOOLS_BATCHCONVERTER
// [16] MENUITEM "Greaseweazle Read",           ID_TOOLS_GREASEWEAZLE
// [17] MENUITEM "Greaseweazle Write",          ID_TOOLS_GREASEWEAZLEWRITE
// [18]MENUITEM SEPARATOR
// [19]MENUITEM "Close",                       ID_FIL_CLOSE
// [20]MENUITEM "Install Bootblock",           ID_TOOLS_INSTALL
// [21]MENUITEM "&View Bootblock",             ID_TOOLS_DISPLAYBOOTBLOCK
// [22]MENUITEM SEPARATOR
// [23]MENUITEM "Disk &Information",           ID_FIL_INFORMATION


	// FILE CONTEXT MENU
	if (bFileClicked == TRUE && bDirClicked == FALSE && bNothingClicked == FALSE) {
		
		//MessageBoxA(winLister, "FILE CONTEXT MENU", "DEBUG:", MB_OK | MB_ICONERROR);
		
		// Remove any separators first because you can only remove them 
		// by position and not by name. And you can't name them... WTF?
		// We only really know their position reliably if we remove them first.
		DeleteMenu(popup,  3 - 0, MF_BYPOSITION);	// The first one we remove is normally at position 6.
		DeleteMenu(popup, 15 - 1, MF_BYPOSITION);	// The second one we remove is normally at position 13, but now it's at 12 because we removed one above. (VisualStudio 2022 literally suggested this comment and it's freak as fuck! I don't know how I feel about it.)
		DeleteMenu(popup, 19 - 2, MF_BYPOSITION); 	// The third one we remove is normally at position 16, but now it's at 14 because we removed two above. (Ugh... VisualStudio 2022's AI must have heard me... it didn't suggest this comment and I had to write it all myself!)
		DeleteMenu(popup, 23 - 3, MF_BYPOSITION);

		DeleteMenu(popup, ID_ACTION_NEWDIRECTORY, MF_BYCOMMAND);

		DeleteMenu(popup, ID_TOOLS_BATCHCONVERTER, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_GREASEWEAZLE, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_GREASEWEAZLEWRITE, MF_BYCOMMAND);

		DeleteMenu(popup, ID_FIL_CLOSE, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_INSTALL, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_DISPLAYBOOTBLOCK, MF_BYCOMMAND);

		DeleteMenu(popup, ID_FIL_INFORMATION, MF_BYCOMMAND);

		EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_GRAYED);
		EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_GRAYED);
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(TRUE, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(TRUE, 0));

	};/*end-if*/


	// FOLDER CONTEXT MENU
	if (bFileClicked == FALSE && bDirClicked == TRUE && bNothingClicked == FALSE) {
		
		//MessageBoxA(winLister, "FOLDER CONTEXT MENU", "DEBUG:", MB_OK | MB_ICONERROR);
		
		// Remove any separators first because you can only remove them 
		// by position and not by name. And you can't name them... WTF?
		// We only really know their position reliably if we remove them first.
		DeleteMenu(popup,  3 - 0, MF_BYPOSITION);	// The first one we remove is normally at position 6.
		DeleteMenu(popup,  6 - 1, MF_BYPOSITION);	// The second one we remove is normally at position 13, but now it's at 12 because we removed one above. (VisualStudio 2022 literally suggested this comment and it's freak as fuck! I don't know how I feel about it.)
		DeleteMenu(popup, 15 - 2, MF_BYPOSITION);	// The third one we remove is normally at position 16, but now it's at 14 because we removed two above. (Ugh... VisualStudio 2022's AI must have heard me... it didn't suggest this comment and I had to write it all myself!)
		DeleteMenu(popup, 19 - 3, MF_BYPOSITION);
		DeleteMenu(popup, 23 - 4, MF_BYPOSITION);

		DeleteMenu(popup, ID_ACTION_NEWDIRECTORY, MF_BYCOMMAND);

		DeleteMenu(popup, ID_TOOLS_TEXT_VIEWER, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_HEX_VIEWER, MF_BYCOMMAND);

		DeleteMenu(popup, ID_TOOLS_BATCHCONVERTER, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_GREASEWEAZLE, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_GREASEWEAZLEWRITE, MF_BYCOMMAND);

		DeleteMenu(popup, ID_FIL_CLOSE, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_INSTALL, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_DISPLAYBOOTBLOCK, MF_BYCOMMAND);

		DeleteMenu(popup, ID_FIL_INFORMATION, MF_BYCOMMAND);

		EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_GRAYED); // <-- This doesn't work fuck you I hate you so much fuck! I've spent way too long trying to get these stupid fucking things to grey-out themselves! 
		EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_GRAYED); // <-- This doesn't work fuck you I hate you so much fuck! I've spent way too long trying to get these stupid fucking things to grey-out themselves! 
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(TRUE, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(TRUE, 0));

	};/*end-if*/

	// BACKGROUND NO SELECTION CONTEXT MENU
	if (bFileClicked == FALSE && bDirClicked == FALSE && bNothingClicked == TRUE) {
		
		//MessageBoxA(winLister, "BACKGROUND NO SELECTION CONTEXT MENU", "DEBUG:", MB_OK | MB_ICONERROR);
		
		// Remove any separators first because you can only remove them 
		// by position and not by name. And you can't name them... WTF?
		// We only really know their position reliably if we remove them first.
		DeleteMenu(popup,  6 - 0, MF_BYPOSITION);	// The first one we remove is normally at position 6.
		DeleteMenu(popup,  9 - 1, MF_BYPOSITION);	// The second one we remove is normally at position 13, but now it's at 12 because we removed one above. (VisualStudio 2022 literally suggested this comment and it's freak as fuck! I don't know how I feel about it.)	
		DeleteMenu(popup, 15 - 2, MF_BYPOSITION);	// The third one we remove is normally at position 16, but now it's at 14 because we removed two above. (Ugh... VisualStudio 2022's AI must have heard me... it didn't suggest this comment and I had to write it all myself!)

		DeleteMenu(popup, ID_TOOLS_TEXT_VIEWER, MF_BYCOMMAND);
		DeleteMenu(popup, ID_TOOLS_HEX_VIEWER, MF_BYCOMMAND);

		DeleteMenu(popup, ID_ACTION_RENAME, MF_BYCOMMAND);
		DeleteMenu(popup, ID_ACTION_DELETE, MF_BYCOMMAND);

		DeleteMenu(popup, ID_ACTION_PROPERTIES, MF_BYCOMMAND);

		EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_ENABLED); // <-- This doesn't work fuck you I hate you so much fuck! I've spent way too long trying to get these stupid fucking things to grey-out themselves! 
		EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_ENABLED); // <-- This doesn't work fuck you I hate you so much fuck! I've spent way too long trying to get these stupid fucking things to grey-out themselves! 
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(FALSE, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(FALSE, 0));

	};/*end-if*/



	//if (bNothingClicked == TRUE) 
	//	MessageBoxA(winLister, "bNothingClicked == TRUE", "DEBUG:", MB_OK | MB_ICONERROR);
	//else if (bNothingClicked == FALSE) 
	//	MessageBoxA(winLister, "bNothingClicked == FALSE", "DEBUG:", MB_OK | MB_ICONERROR);
	//else
	//	MessageBoxA(winLister, "bNothingClicked == DA FUCK?!??!!", "DEBUG:", MB_OK | MB_ICONERROR);




	// 5) Show the popup
	SetForegroundWindow(ghwndFrame);
	TrackPopupMenuEx(popup,
		TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		ptScreen.x, ptScreen.y,
		ghwndFrame,
		NULL);
	PostMessage(ghwndFrame, WM_NULL, 0, 0);

	// 6) Cleanup
	CleanupMenuIcons();
	DestroyMenu(menu);
}

void ChildDelete(HWND win)
/* delete selected files
 */
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	char nam[60];
	char path[MAX_PATH];
	char msg[MAX_PATH + 60];
	int i, icon;
	BOOL doit;

	HINSTANCE hInst = GetModuleHandle(NULL);

	/* confirm before commencing delete (unless user has disabled it) */
	if (Options.confirmDelete) {
		// Play Sound 3 --> Alert / Are You Sure? 
		// HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_3), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		if (MessageBox(win, "Are you sure you want to delete the selected item(s)?",
			"Question", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
			return;
	}

	/* delete windows files */
	if (GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER) {
		for (i = 0 ; i < ListView_GetItemCount(ci->lv) ; i++) {
			if (LVIsItemSelected(ci->lv, i)) {
				icon = LVGetItemImageIndex(ci->lv, i);
				if ((icon == ICO_WINFILE) || icon == ICO_WINFILE_ADF || (icon == ICO_WINDIR)) {
					strcpy(path, ci->curDir);
					LVGetItemCaption(ci->lv, nam, sizeof(nam), i);
					strcat(path, nam);
					if (icon == ICO_WINFILE || icon == ICO_WINFILE_ADF) {
						if (! DeleteFile(path)) {
							// Play Sound 1 --> Warning! / Error!
							// HINSTANCE hInst = GetModuleHandle(NULL);
							if (Options.playSounds)
								PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
							/*end-if*/
							sprintf(msg, "Couldn't delete file '%s'.  Maybe it is in use or the device is write-protected.", path);
							MessageBox(win, msg, "Error", MB_OK | MB_ICONERROR);
						}
					} else {
						if (Options.confirmDeleteDirs) {
							// Play Sound 3 --> Alert / Are You Sure? 
							// HINSTANCE hInst = GetModuleHandle(NULL);
							if (Options.playSounds)
								PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_3), hInst, SND_RESOURCE | SND_ASYNC);
							/*end-if*/
							sprintf(msg, "'%s' is a directory. Are you sure you want to delete it and all its contents?", path);
							doit = (MessageBox(win, msg, "Question", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES);
						} else
							doit = TRUE;
						if (doit)
							if (! RemoveDirectoryRecursive(path)) {
								// Play Sound 1 --> Warning! / Error!
								// HINSTANCE hInst = GetModuleHandle(NULL);
								if (Options.playSounds)
									PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
								/*end-if*/
								sprintf(msg, "Deletion of directory '%s' was not entirely successful.", path);
								MessageBox(win, msg, "Error", MB_OK );
							}
					}
				}
			}
		}
	}

	/* delete amiga files */
	if (GetWindowLong(win, GWL_USERDATA) == CHILD_AMILISTER) {
		for (i = 0 ; i < ListView_GetItemCount(ci->lv) ; i++) {
			if (LVIsItemSelected(ci->lv, i)) {
				icon = LVGetItemImageIndex(ci->lv, i);
				if (icon == ICO_AMIFILE || icon == ICO_AMIDIR) {
					LVGetItemCaption(ci->lv, path, sizeof(path), i);
					if (icon == ICO_AMIFILE) {
						if (adfRemoveEntry(ci->vol, ci->vol->curDirPtr, path) != RC_OK) {
							sprintf(msg, "Couldn't delete file '%s'.  I don't know why.", path);
							MessageBox(win, msg, "Error", MB_OK | MB_ICONERROR);
						}
					} else {
						if (Options.confirmDeleteDirs) {
							// Play Sound 3 --> Alert / Are You Sure? 
// HINSTANCE hInst = GetModuleHandle(NULL);
							if (Options.playSounds)
								PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_3), hInst, SND_RESOURCE | SND_ASYNC);
							/*end-if*/
							sprintf(msg, "'%s' is a directory. Are you sure you want to delete it and all its contents?", path);
							doit = (MessageBox(win, msg, "Question", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES);
						} else
							doit = TRUE;
						if (doit)
							if (! RemoveAmiDirectoryRecursive(ci->vol, ci->vol->curDirPtr, path) ) {
								sprintf(msg, "Deletion of directory '%s' was not entirely successful.", path);
								MessageBox(win, msg, "Error", MB_OK | MB_ICONERROR);
							}
					}
				}
			}
		}
	}

	/* refresh dir listing */
	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);
}


BOOL ChildShowUndeletable(HWND win)
/*  Show undeletable files.
 */
{
	struct List		*list, *cell;
	struct GenBlock	*block;
	char			undel[MAX_PATH];
	int				pos;


	//CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	CHILDINFO* ci = (CHILDINFO*)GetWindowLongPtr(win, 0); // Chiron TODO: Look at original non-ported code!
	cell = list = adfGetDelEnt(ci->vol);
	if(cell == NULL){
		// Play Sound 3 --> Alert / Are You Sure? 
		HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_3), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		MessageBox(win, "There are no undeletable files or directories on this volume.", "Undelete Message", 
					MB_OK);
		return FALSE;
	}
	// Read undeletable files list.
    while(cell){
        block =(struct GenBlock*) cell->content;
        sprintf(undel, "%s",block->name);

		//// DEBUGGING --> Instrument every block you get back:
		//char debug[256];
		//sprintf(debug, "sect=%06lx secType=%d name=%s\n",
		//	(unsigned long)block->sect, block->secType, block->name);
		////OutputDebugString(debug);
		//MessageBox(win, debug, "WTF?!?!?!?!", MB_ICONINFORMATION | MB_OK);



		if(adfCheckEntry(ci->vol, block->sect, 0) == RC_OK){
			if(block->secType == 2)
				pos = LVAddItem(ci->lv, undel, ICO_AMIDIR);
			else
				pos = LVAddItem(ci->lv, undel, ICO_AMIFILE);
			ListView_SetItemState(ci->lv, pos, LVIS_CUT, LVIS_CUT);
			pos++;
			
			//// DEBUGGING
			//// inside your ChildShowUndeletable loop, replace:
			//// if (adfCheckEntry(ci->vol, block->sect, 0) == RC_OK) { … }
			//// with:
			//{
			//	// always add to list, so you can see exactly what adfGetDelEnt returns // Chiron 2025 --> THIS WORKED!!!
			//	if (block->secType == 2) pos = LVAddItem(ci->lv, undel, ICO_AMIDIR);
			//	else            pos = LVAddItem(ci->lv, undel, ICO_AMIFILE);
			//	ListView_SetItemState(ci->lv, pos, LVIS_CUT, LVIS_CUT);
			//}


		}
		
		cell = cell->next;
    }
  
    adfFreeDelList(list);
	// Prevent multiple listings.
	EnableMenuItem(hMenu, ID_VIEW_SHOWUNDELETABLEFILES, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_VIEW_SHOWUNDELETABLEFILES, MAKELONG(FALSE, 0));
	bUndeleting = TRUE;
	return TRUE;
}





BOOL ChildUndelete(HWND win)
{
	struct List* list = NULL, * cell;
	struct GenBlock* block;
	int             index, iNumSelected, i;
	char            buf[MAX_PATH];
	CHILDINFO* ci = (CHILDINFO*)GetWindowLongPtr(win, 0);
	
	iNumSelected = ListView_GetSelectedCount(ci->lv);
	
	index = ListView_GetNextItem(ci->lv, -1, LVNI_CUT | LVNI_SELECTED);


	for (i = 0; i < iNumSelected; i++) {
		LVGetItemCaption(ci->lv, buf, sizeof(buf), index);

		// Get the deleted-entries list once per batch, not per file
		if (list == NULL) {
			list = adfGetDelEnt(ci->vol);
		}
		cell = list;

		while (cell) {
			block = (struct GenBlock*)cell->content;
			if (strcmp(buf, block->name) == 0) {
				if (adfUndelEntry(ci->vol, ci->vol->curDirPtr, block->sect) == RC_OK) {
					ListView_SetItemState(ci->lv, index, 0, LVIS_CUT);
					ChildUpdate(win);
				}
				break;
			}
			cell = cell->next;
		}
		index = ListView_GetNextItem(ci->lv, index, LVNI_CUT | LVNI_SELECTED);
	}

	if (list) {
		adfFreeDelList(list);
	}

	EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UNDELETE, MAKELONG(FALSE, 0));
	return TRUE;
}

BOOL ChildRename(HWND win, LV_DISPINFO *di)
/* rename a file
 */
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	char oldName[MAX_PATH];
	char newName[MAX_PATH];
	char newPath[MAX_PATH];
	char oldPath[MAX_PATH];
	char errMess[MAX_PATH * 2];

	LVGetItemCaption(ci->lv, oldName, sizeof(oldName), di->item.iItem);

	if (di->item.pszText == NULL)
		return FALSE;

	strcpy(newName, di->item.pszText);

	if (GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER) {
		strcpy(oldPath, ci->curDir);
		strcat(oldPath, oldName);
		strcpy(newPath, ci->curDir);
		strcat(newPath, newName);
		if (! MoveFile(oldPath, newPath)) {
			sprintf(errMess, "Could not rename '%s' to '%s' for some reason.", oldName, newName);
			MessageBox(win, errMess, "Error", MB_OK | MB_ICONERROR);
			return FALSE;
		} else {
			ListView_SetItemText(ci->lv, di->item.iItem, 0, newName);
			// Rebuild both the LV and your ci->content list. 
			// This fixes the bug where you rename a file and then copy the renamed file 
			// but it crashes everything when I tries to do it because 
			// it's trying to copy the old filename. 
			ChildUpdate(win);
		}
	}

	if (GetWindowLong(win, GWL_USERDATA) == CHILD_AMILISTER) {
		if (adfRenameEntry(ci->vol, ci->vol->curDirPtr, oldName, ci->vol->curDirPtr, newName) != RC_OK) {
			sprintf(errMess, "Could not rename '%s' to '%s'.  Don't ask me why.", oldName, newName);
			MessageBox(win, errMess, "Error", MB_OK | MB_ICONERROR);
			return FALSE;
		} else {
			ListView_SetItemText(ci->lv, di->item.iItem, 0, newName);
			// Rebuild both the LV and your ci->content list. 
			// This fixes the bug where you rename a file and then copy the renamed file 
			// but it crashes everything when I tries to do it because 
			// it's trying to copy the old filename. 
			ChildUpdate(win);
		}
	}
	return TRUE;
}

BOOL ChildCheckRename(HWND win, LV_DISPINFO *di)
/* return TRUE if item can be edited, FALSE otherwise */
//////// This function is FUCKED.
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	int ii;

	ii = LVGetItemImageIndex(ci->lv, di->item.iItem);

	if (ii >= ICO_DRIVEHD)
		return FALSE;
	else
		return TRUE;
}

void ChildMakeDir(HWND win)
/* create a new dir "New Directory" and call the rename function on it
 */
{
	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	char newPath[MAX_PATH];
	int pos;

	if (GetWindowLong(win, GWL_USERDATA) == CHILD_AMILISTER) {
		if (adfCreateDir(ci->vol, ci->vol->curDirPtr, "New Directory") != RC_OK) {
			MessageBox(win, "Couldn't create directory, the volume is probably full.",
				"Error", MB_OK | MB_ICONERROR);
			return;
		}
	} else {
		strcpy(newPath, ci->curDir);
		strcat(newPath, "New Directory");
		if (! CreateDirectory(newPath, NULL)) {
			MessageBox(win, "Couldn't create directory.", "Error", MB_OK | MB_ICONERROR);
			return;
		}
	}

	pos = LVAddItem(ci->lv, "New Directory", (GetWindowLong(win, GWL_USERDATA) ==
	 CHILD_AMILISTER) ? ICO_AMIDIR : ICO_WINDIR);
	
	LVAddSubItem(ci->lv, (GetWindowLong(win, GWL_USERDATA) ==
	 CHILD_AMILISTER) ? "RWED" : "", pos, 2);
	ListView_EditLabel(ci->lv, pos);
		
}

BOOL RemoveDirectoryRecursive(char *path)
{
	WIN32_FIND_DATA wfd;
	HANDLE search;
	char curPath[MAX_PATH * 2];
	char searchPath[MAX_PATH * 2];
	char subdir[MAX_PATH * 2];

	strcpy(curPath, path);
	sprintf(searchPath, "%s\\*", curPath);

	search = FindFirstFile(searchPath, &wfd);
	if (search == INVALID_HANDLE_VALUE)
		return FALSE;

	do {
		/* if current entry is a dir, and isn't the current or parent dir, then delete it */
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if ( !((wfd.cFileName[1] == 0) && (wfd.cFileName[0] == '.')) )
				if ( !((wfd.cFileName[2] == 0) && (wfd.cFileName[1] == '.') && (wfd.cFileName[0] == '.')) ) {
					sprintf(subdir, "%s\\%s", curPath, wfd.cFileName);
					RemoveDirectoryRecursive(subdir);
				}
		} else {
			sprintf(subdir, "%s\\%s", curPath, wfd.cFileName);
			DeleteFile(subdir); //// ERROR CHECKING!
		}

	} while (FindNextFile(search, &wfd));

	FindClose(search);

	return RemoveDirectory(curPath);
}

BOOL RemoveAmiDirectoryRecursive(struct Volume *vol, SECTNUM curDir, char *path)
{
	struct List *list;
	struct Entry *ent;

	adfChangeDir(vol, path);
	list = adfGetDirEnt(vol, vol->curDirPtr);

	while (list) {
		ent = (struct Entry *)list->content;
		if (ent->type == ST_DIR) {
			/* it's a dir - recurse into it */
			RemoveAmiDirectoryRecursive(vol, vol->curDirPtr, ent->name);
		} else {
			/* it's a file or a link, just remove it */
			adfRemoveEntry(vol, vol->curDirPtr, ent->name);
		}
		adfFreeEntry(list->content);
		list = list->next;
	}
	freeList(list);

	adfParentDir(vol);

	if (adfRemoveEntry(vol, curDir, path) != RC_OK)
		return FALSE;

	return TRUE;
}


int GetFileFromADF(struct Volume *vol, char	*szFileName)
// Reads a file from an ADF and writes it to Windows.
// Taken from an example in the ADFLib documentation by Laurent Clevy.
// Input:  The volume structure of the current ADF, the name of the file to extract.
// Output: 
{
	struct File*	file;
	FILE*			out;
	long			n;
	unsigned char	buf[600];
	int				len = 600;

	/* a device and a volume 'vol' has been successfully mounted */
	/* opens the Amiga file */
	file = adfOpenFile(vol, szFileName,"r");

	if(!file){ 
		/* frees resources and exits */
		return(-1);						//******************** value here
	}
	/* opens the output classic file */
	out = fopen(szFileName,"wb");
	if(!out){
		adfCloseFile(file);
		return(-2);						//******************** value here
	}
    
	/* copy the Amiga file into the standard file, 600 bytes per 600 bytes */
	n = adfReadFile(file, len, buf);
	while(!adfEndOfFile(file)){
		fwrite(buf, sizeof(unsigned char), n, out);
		n = adfReadFile(file, len, buf);
	}
	/* even if the EOF is reached, some bytes may need to be written */
	if(n > 0)
		fwrite(buf, sizeof(unsigned char), n, out);
	/* closes the standard file */
	fclose(out);
	/* closes the Amiga file */
	adfCloseFile(file);
	return(0);							//******************** value here
}
