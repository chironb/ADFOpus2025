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

//BOOL	bClicked = FALSE;
BOOL	bDirClicked = FALSE, bFileClicked = FALSE;		// File or dir selection flags.
BOOL	bUndeleting = FALSE;							// Undeletion flag.

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
	} else
		strcpy(title, "Windows Directory");

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

LRESULT ChildOnCreate(HWND win)
{
	CHILDINFO	*ci;

	/* set the user data long to identify what type of child this is */
	SetWindowLong(win, GWL_USERDATA, newWinType);

	/* allocate an info struct and store the address in the extra window space */
	ci = malloc(sizeof(CHILDINFO));
	SetWindowLong(win, 0, (LONG) ci);

	/* set the current directory */
	if(GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER){
		if(Options.defDriveList){
			strcpy(ci->curDir, "");
			ci->atRoot = TRUE;
		}
		else{
			strcpy(ci->curDir, Options.defaultDir);
			ci->atRoot = FALSE;
		}
	}
	else{
		strcpy(ci->curDir, "/");
		// Save disk type and compressed size to info structure.
		ci->dfDisk = dfDisk;
		ci->compSize = comp_size;

		ci->dev = adfMountDev(gstrFileName, ReadOnly);
		if (ci->dev == NULL)
			return -1;

		ci->vol = adfMount(ci->dev, volToOpen, FALSE);///////////////////
		if (ci->vol == NULL)
			return -1;
		ci->atRoot = TRUE;
	}

	ci->readOnly = ReadOnly;

	/* create the listview control and store the handle */
	ci->lv = CreateListView(win);

	/* create the status bar */
	ci->sb = CreateWindow(
		STATUSCLASSNAME,
		"",
		WS_CHILD | WS_VISIBLE | CCS_TOP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		win,
		NULL,
		instance,
		NULL
	);

	/* init linked list of dir entries */
	ci->content = NULL;

	ci->isAmi = (newWinType == CHILD_AMILISTER);

	strcpy(ci->orig_path, buf);					// Store original filename.
	if(ci->dfDisk != ADF)
		strcpy(ci->temp_path, gstrFileName);	// Store temp adf filename.
	
	// Disable properties menu item and toolbar button, in case they've been left active.
	hMenu = GetMenu(ghwndFrame);
	EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));

	ChildUpdate(win);

	return 0;
}

BOOL ChildOnCommand(HWND win, WPARAM wp, LPARAM lp)
/* WM_COMMAND messages, mostly passed on from frame window */
{
	CHILDINFO *ci;

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

	// Cleanup temp files. Recompress adz.
	if(ci->dfDisk == ADZ){
		GZCompress(NULL, ci->temp_path, ci->orig_path);
		remove(ci->temp_path);
	}

	// Disable properties menu item and toolbar button, in case they've been left active.
	hMenu = GetMenu(ghwndFrame);
	EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));

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
			// An item was single-clicked.
			// Find out which file.
			index = LVGetItemFocused(ci->lv);
			LVGetItemCaption(ci->lv, buf, sizeof(buf), index);
			// Return if a drive icon was clicked.
			iSelectedType = LVGetItemImageIndex(ci->lv, index);
			switch (iSelectedType){
				case ICO_DRIVEHD:
				case ICO_DRIVECD:
				case ICO_DRIVENET:
				case ICO_DRIVEFLOP35:
				case ICO_DRIVEFLOP514:
					return TRUE;
				// Selected item is a file or dir for text viewer purposes.
				case ICO_AMIDIR:
				case ICO_WINDIR:
					bIsFile = FALSE;							
					break;
				case ICO_AMIFILE:
				case ICO_WINFILE:
					bIsFile = TRUE;							
			}
			// Item is a deleted file.
			if(ListView_GetItemState(ci->lv, index, LVIS_CUT)){
				// Enable menu and toolbar items for undelete.
				EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_ENABLED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UNDELETE, MAKELONG(TRUE, 0));
				// Disable properties, rename, delete, text viewer.
				EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_GRAYED);
				EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_GRAYED);
				EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
				EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(FALSE, 0));
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(FALSE, 0));
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
				bUndeleting = TRUE;
			}
			else{	// Not a deleted file.
				EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_GRAYED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UNDELETE, MAKELONG(FALSE, 0));
				// Activate Properties items if not a drive.
				if(ListView_GetSelectedCount(ci->lv) > 0){
//					bClicked = TRUE;
					// Enable properties, rename, delete, text viewer.
					EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_ENABLED);
					EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_ENABLED);
					EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_ENABLED);
					SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(TRUE, 0));
					SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(TRUE, 0));
					SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(TRUE, 0));
					// Enable text viewer if file selected, else disable.
					switch (iSelectedType){
						// Selected item is a file or dir for text viewer purposes.
						case ICO_AMIDIR:
						case ICO_WINDIR:
							SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
							EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
							bDirClicked = TRUE;
							bFileClicked = FALSE;
							break;
						case ICO_AMIFILE:
						case ICO_WINFILE:
							SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(TRUE, 0));
							EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_ENABLED);
							bDirClicked = FALSE;
							bFileClicked = TRUE;							
					}

//					if(bIsFile){
//						SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(TRUE, 0));
//						EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_ENABLED);
//					}
//					else{
//						SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
//						EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
//					}
				}
				bUndeleting = FALSE;
			}

			break;
	
		case NM_DBLCLK:
			/* an item was double-clicked, find out which one */
			/*** THIS IS A KLUDGE - it will return the focused
				item even if the user double clicks an empty
				part of the listview control! */
			index = LVGetItemFocused(ci->lv);
			// Item is a deleted file, return.
			if(ListView_GetItemState(ci->lv, index, LVIS_CUT))
				return TRUE;
			LVGetItemCaption(ci->lv, buf, sizeof(buf), index);
			// Deactivate the Properties context and main menu items and toolbar button.
			EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
//			bClicked = FALSE;
			bDirClicked = bFileClicked = FALSE;					// Reset the context menu items.

			switch (LVGetItemImageIndex(ci->lv, index)) {
				case ICO_WINFILE:
					strcpy(szWinFile, ci->curDir);
					strcat(szWinFile, buf);
					iError = (int)ShellExecute(win, "open", szWinFile, NULL, NULL, SW_SHOWNORMAL);
					ChildUpdate(win);
					break;
			
				case ICO_WINDIR:
					/* it is a dir, append the name to the
					   current directory */
					strcat(ci->curDir, buf);
					strcat(ci->curDir, "\\");
					ci->atRoot = FALSE;

					ChildUpdate(win);
					break;
				case ICO_AMIFILE:
					// Get the file and copy it to the temp dir.
					if(_chdir(dirTemp) == -1){						// Change to temp dir.
						_mkdir(dirTemp);							// Create if doesn't yet exist.
						_chdir(dirTemp);
					}
					if(GetFileFromADF(ci->vol, buf) < 0){
						sprintf(szError, "Error extracting %s from %s.", buf, ci->orig_path);
						MessageBox(win, szError, "ADF Opus Error", MB_ICONSTOP);
						break;
					}
					// Run the file.
					iError = (int)ShellExecute(win, "open", buf, NULL, NULL, SW_SHOWNORMAL);
					_chdir(dirOpus);								// Change back to Opus dir.
					
					if(iError == SE_ERR_NOASSOC){		// No file asociation.
						sprintf(szError,
								"ADF Opus is unable to open %s because no application is "
								 "associated with this file type under Windows. See Help for how "
								 "to register this file type with Windows and then try again, "
								 "or use the built-in text viewer.",
								 buf);
						MessageBox(win, szError, "ADF Opus Error", MB_ICONINFORMATION);
					}
					else if(iError <= 32){				// Other error.
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
					/* it is a drive, set the current dir
					to the root dir of the drive */
					strcpy(ci->curDir, buf);
					strcat(ci->curDir, ":\\");
					ci->atRoot = FALSE;

					ChildUpdate(win);
					break;
			}
			break;
		case NM_KILLFOCUS:
			// Disable properties menu item and toolbar button when the current window loses
			// focus, in case they've been left active.
			EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
			EnableMenuItem(hMenu, ID_TOOLS_TEXT_VIEWER, MF_GRAYED);
			SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));
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
	HWND lv;

	/* Create the control */
	lv = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		"",
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE | 
			LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER,
		0, 0,
		300, 100,
		win,
		NULL,
		instance,
		NULL
	);

	if (lv == NULL)
		return NULL;

	/* add the columns */
	LVAddColumn(lv, "Name", 200, 0);
	LVAddColumn(lv, "Size", 70, 1);
	LVAddColumn(lv, "Flags", 75, 2);
	// Create a Comment column in Amiga listers.
	if(GetWindowLong(win, GWL_USERDATA) != CHILD_WINLISTER)
		LVAddColumn(lv, "Comment", 605, 3);

	/* assign the image list */
	ListView_SetImageList(lv, ghwndImageList, LVSIL_SMALL);
	// Set full row selection.
	ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);

	return lv;
} 

void ChildUpdate(HWND win)
/* fills the list view control with the directory content */
{
	CHILDINFO	*ci = (CHILDINFO *)GetWindowLong(win, 0);
	DIRENTRY	*ce;
	char		strBuf[20];
	int			pos;
	struct File *amiFile;
	BOOL		bAmi = FALSE;


	SetWindowText(ghwndSB, "Reading directory...");

	if (GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER)
		if (strcmp(ci->curDir, "") == 0)
			WinGetDrives(win);
		else
			WinGetDir(win);
	
	else{
		bAmi = TRUE;
		AmiGetDir(win);
	}
	ChildSortDir(win, 0l);

	SendMessage(ci->lv, WM_SETREDRAW, FALSE, 0);

	ListView_DeleteAllItems(ci->lv);

	ListView_SetItemCount(ci->lv, ci->totalCount);

	ce = ci->content;
	while (ce != NULL) {
		pos = LVAddItem(ci->lv, ce->name, ce->icon);
		if (pos == -1)
			pos++;
		if (ce->icon == ICO_WINFILE || ce->icon == ICO_AMIFILE) {
			itoa(ce->size, strBuf, 10);
			LVAddSubItem(ci->lv, strBuf, pos, 1);
		}
		LVAddSubItem(ci->lv, ce->flags, pos, 2);

		// Display amiga file comment.
		if(bAmi){
			amiFile = adfOpenFile(ci->vol, ce->name, "r");
			LVAddSubItem(ci->lv, amiFile->fileHdr->comment, pos, 3);
			adfCloseFile(amiFile);
		}
		ce = ce->next;
	}

	SendMessage(ci->lv, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(ci->lv, NULL, FALSE);

	/* update status bars */
	if (! strcmp(ci->curDir, ""))
		SetWindowText(ci->sb, "All drives");
	else
		SetWindowText(ci->sb, ci->curDir);
	UpdateToolbar();
	SetWindowText(ghwndSB, "Idle");

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
	de->icon = ((wfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		? ICO_WINDIR : ICO_WINFILE;

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

void DisplayContextMenu(HWND win, POINT pt)
/* bring up the context menu
 */
{
	HMENU	menu, popup;
	HBITMAP	bmpMenu;

	menu = LoadMenu(instance, MAKEINTRESOURCE(IDR_LISTERMENU));
	if (menu == NULL)
		return;
 	popup = GetSubMenu(menu, 0);
	// Load and display menu bitmaps.
	bmpMenu =  (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(IDB_UPONELEVEL), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
	SetMenuItemBitmaps(popup, 0, MF_BYPOSITION, bmpMenu, bmpMenu);
	bmpMenu =  (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(IDB_PROPERTIES), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
	SetMenuItemBitmaps(popup, 6, MF_BYPOSITION, bmpMenu, bmpMenu);
	bmpMenu =  (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(IDB_TEXTVIEWER), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
	SetMenuItemBitmaps(popup, 8, MF_BYPOSITION, bmpMenu, bmpMenu);
	
	// Activate the Properties item if a file has been clicked.
//	if(bClicked == TRUE)
	if(bDirClicked || bFileClicked)
		EnableMenuItem(popup, ID_ACTION_PROPERTIES, MF_ENABLED);
	if(bFileClicked)
		EnableMenuItem(popup, ID_TOOLS_TEXT_VIEWER, MF_ENABLED);

	TrackPopupMenuEx(popup, 0, pt.x, pt.y, ghwndFrame, NULL);
	DestroyMenu(menu);
	DeleteObject(bmpMenu);
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

	/* confirm before commencing delete (unless user has disabled it) */
	if (Options.confirmDelete)
		if (MessageBox(win, "Are you sure you want to delete the selected item(s)?",
		"Question", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
			return;

	/* delete windows files */
	if (GetWindowLong(win, GWL_USERDATA) == CHILD_WINLISTER) {
		for (i = 0 ; i < ListView_GetItemCount(ci->lv) ; i++) {
			if (LVIsItemSelected(ci->lv, i)) {
				icon = LVGetItemImageIndex(ci->lv, i);
				if ((icon == ICO_WINFILE) || (icon == ICO_WINDIR)) {
					strcpy(path, ci->curDir);
					LVGetItemCaption(ci->lv, nam, sizeof(nam), i);
					strcat(path, nam);
					if (icon == ICO_WINFILE) {
						if (! DeleteFile(path)) {
							sprintf(msg, "Couldn't delete file '%s'.  Maybe it is in use or the device is write-protected.", path);
							MessageBox(win, msg, "Error", MB_OK | MB_ICONERROR);
						}
					} else {
						if (Options.confirmDeleteDirs) {
							sprintf(msg, "'%s' is a directory. Are you sure you want to delete it and all its contents?", path);
							doit = (MessageBox(win, msg, "Question", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES);
						} else
							doit = TRUE;
						if (doit)
							if (! RemoveDirectoryRecursive(path)) {
								sprintf(msg, "Deletion of directory '%s' was not entirely successful.", path);
								MessageBox(win, msg, "Error", MB_OK | MB_ICONERROR);
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


	CHILDINFO *ci = (CHILDINFO *)GetWindowLong(win, 0);
	cell = list = adfGetDelEnt(ci->vol);
	if(cell == NULL){
		MessageBox(win, "There are no undeletable files or directories on this volume.", "Undelete Message", 
					MB_ICONINFORMATION|MB_OK);
		return FALSE;
	}
	// Read undeletable files list.
    while(cell){
        block =(struct GenBlock*) cell->content;
        sprintf(undel, "%s",block->name);
		if(adfCheckEntry(ci->vol, block->sect, 0) == RC_OK){
			if(block->secType == 2)
				pos = LVAddItem(ci->lv, undel, ICO_AMIDIR);
			else
				pos = LVAddItem(ci->lv, undel, ICO_AMIFILE);
			ListView_SetItemState(ci->lv, pos, LVIS_CUT, LVIS_CUT);
			pos++;
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
/*  Undelete files.
 */
{
	struct List		*list, *cell;
	struct GenBlock	*block;
	int				index, iNumSelected, i;
	char			buf[MAX_PATH];
	CHILDINFO		*ci = (CHILDINFO *)GetWindowLong(win, 0);

	
//	index = LVGetItemFocused(ci->lv);
	iNumSelected = ListView_GetSelectedCount(ci->lv);
	index = ListView_GetNextItem(ci->lv, -1, LVNI_CUT|LVNI_SELECTED);					// Get first selected file.
	for(i = 0;i < iNumSelected;i++){
		LVGetItemCaption(ci->lv, buf, sizeof(buf), index);
		cell = list = adfGetDelEnt(ci->vol);
		// Read undeletable files list.
		while(cell){
			block =(struct GenBlock*) cell->content;
			// Check if names match and undelete if they do.
			if(strcmp(buf, block->name) == 0){
				if(adfUndelEntry(ci->vol, ci->vol->curDirPtr, block->sect) == RC_OK){	// Undelete it.
					ListView_SetItemState(ci->lv, index, 0, LVIS_CUT);					// Reset list state.
					ChildUpdate(win);
				}
				break;
			}
			cell = cell->next;
		}
		index = ListView_GetNextItem(ci->lv, index, LVNI_CUT|LVNI_SELECTED);			// Get next selected file.
	}
    adfFreeDelList(list);
	EnableMenuItem(hMenu, ID_ACTION_UNDELETE, MF_GRAYED);								// Reset menu and toolbar.
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
		}
	}

	if (GetWindowLong(win, GWL_USERDATA) == CHILD_AMILISTER) {
		if (adfRenameEntry(ci->vol, ci->vol->curDirPtr, oldName, ci->vol->curDirPtr, newName) != RC_OK) {
			sprintf(errMess, "Could not rename '%s' to '%s'.  Don't ask me why.", oldName, newName);
			MessageBox(win, errMess, "Error", MB_OK | MB_ICONERROR);
			return FALSE;
		} else {
			ListView_SetItemText(ci->lv, di->item.iItem, 0, newName);
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
