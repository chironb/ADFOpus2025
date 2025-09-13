/*! \file ADFOpus.c
 *  \brief Main programme functions.
 *
 * ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland and Gary Harris <gharris@zip.com.au>.	
 *	
 * Uses ADFlib Copyright 1997-2002 by Laurent Clevy <lclevy@club-internet.fr>
 *
 * Uses xDMS by Andre R. de la Rocha (Public Domain)
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * --------------------------------------------------------------------------
 *
 * This is my first attempt at a raw WinAPI application.  Hello world.
 *
 * Tab size is 8.
 *
 * ADFOpus.c - ADF Opus main program, registering of classes and all that is
 *             done in Init.c.
 */
#define OEMRESOURCE
#include "Pch.h"

#include "ADFOpus.h"
#include "Init.h"
#include "ChildCommon.h"
#include "About.h"
#include "Utils.h"
#include "ListView.h"
#include "New.h"
#include "VolInfo.h"
#include "BatchConvert.h"
#include "Options.h"
#include "Properties.h"
#include <direct.h>
#include "winbase.h"
#include "Install.h"
#include "FDI.h"
#include "Bootblock.h"
#include "TextViewer.h"
#include "HexViewer.h"

#include "ADFLib.h"
#include "Help\AdfOpusHlp.h"


// Chiron 2025
#include <windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")



#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")




// tell this .c file that ghwndFrame exists somewhere else
extern HWND ghwndFrame;
#include "MenuIcons.h"
extern HINSTANCE instance;    // or however you name your HINSTANCE
extern HWND      ghwndFrame;  // your main window handle
extern BOOL      bDirClicked;
extern BOOL      bFileClicked;

#include "Bootblock.h"   // now brings in RawWriteBootBlock()


// 1) include your shared header
// #include "ADFOpus.h" already did this above del this later. my code is messy like my brain! Messy.... like a fox!

// 2) define the variable here (no extern)
// This is the default local filesystem path that the Windows lister will open to when you open a new Windows lister window.
#include <shlwapi.h>      // for PathRemoveFileSpecA, PathAddBackslashA
#pragma comment(lib, "Shlwapi.lib")  // link shlwapi

char gstrCmdLineArgs[CMDLINE_BUFFER] = { 0 };
char g_defaultLocalPath[MAX_PATH] = { 0 };




#include <windows.h>
#include "resource.h"



extern char* adfGetVersionNumber(); /* this shouldn't be here */


extern BOOL isCurrentlyOpen(const char* gstrFileName);
extern BOOL BringAmigaListerToFront(const char* gstrFileName);
extern void adfCloseFileNoDate(struct File* file);

extern RETCODE adfSetEntryDate(
	struct Volume* vol,
	SECTNUM        parSect,
	char* name,
	long           newDays,
	long           newMins,
	long           newTicks);


ENV_DECLARATION;

/* function prototypes */
LRESULT CALLBACK MainWinProc(HWND, UINT, WPARAM, LPARAM);
BOOL CreateProc(HWND);
BOOL CommandProc(HWND, WPARAM, LPARAM);
void PaintProc(HWND);
void DestroyProc(HWND);
void MainWinOnDragOver(int, int);
void MainWinOnDrop();
void CopyAmi2Win(char *, char *, struct Volume *, long);
BOOL CopyAmiDir2Win(char *, char *, struct Volume *);
void CopyWin2Ami(char *, char *, struct Volume *, long);
BOOL CopyWinDir2Ami(char *, char *, struct Volume *);
void CopyWin2Win(char *, char *);
BOOL CopyWinDir2Win(char *, char *, char *);
void CopyAmi2Ami(char *, struct Volume *, struct Volume *, long);
BOOL CopyAmiDir2Ami(char *, struct Volume *, struct Volume *);
void GetTooltipText(char *, int);
void doCopy(void *);
LRESULT CALLBACK copyProgressProc(HWND, UINT, WPARAM, LPARAM);
long AOGetFileSize(CHILDINFO *, char *);


/* global variables (too many) */
HINSTANCE		instance = NULL;
HWND			ghwndFrame;
HWND			ghwndMDIClient = NULL;
HIMAGELIST		ghwndImageList = NULL;
HWND			ghwndTB = NULL;
HWND			ghwndSB = NULL;
char			gstrFileName[MAX_PATH * 2];
BOOL			gbToolbarVisible = TRUE;
BOOL			gbStatusBarVisible = TRUE;
BOOL			gbFirstTime = FALSE;
BOOL			gbIsDragging = FALSE;
HWND			ghwndDragSource, ghwndDragTarget = NULL;
HCURSOR			ghcurNormal, ghcurNo, ghcurDrag;
int				volToOpen;
int				Percent;
CHILDINFO		*CopySource, *CopyDest;
int				Done;
UINT			Timer;
long			CurrentSect;
struct OPTIONS	Options;
BOOL			ReadOnly;

BOOL			bCmdLineArgs = FALSE;
char			gstrCmdLineArgs[MAX_PATH * 2];			// Command line argument string.



/*
 * ensure_extension
 *
 * Ensures that `path` ends with the given `ext` (e.g. ".adf" or ".hdf").
 * If it already has that suffix, nothing changes. Otherwise ext is appended.
 *
 * path:        in/out buffer holding a NUL-terminated file name
 * buffer_size: total size of the `path` buffer, in bytes
 * ext:         extension to enforce, including the leading dot
 *
 * Returns true if path now ends in ext, false if buffer was too small.
 */
//BOOL ensure_extension(char* path, size_t buffer_size, const char* ext)
//{
//	size_t path_len = strlen(path);
//	size_t ext_len = strlen(ext);
//
//	// already ends with ext?
//	if (path_len >= ext_len
//		&& strcmp(path + path_len - ext_len, ext) == 0)
//	{
//		return TRUE;
//	}
//
//	// need room for ext plus NUL
//	if (path_len + ext_len + 1 > buffer_size) {
//		return FALSE;
//	}
//
//	// append extension (copies the NUL too)
//	memcpy(path + path_len, ext, ext_len + 1);
//	return TRUE;
//}
//
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool ensure_extension(char* path, size_t bufsize, const char* ext)
{
	size_t path_len = strlen(path);
	size_t ext_len = strlen(ext);

	if (path_len >= ext_len
		&& _stricmp(path + path_len - ext_len, ext) == 0)
	{
		return true;
	}
	if (path_len + ext_len + 1 > bufsize) {
		return false;
	}
	memcpy(path + path_len, ext, ext_len + 1);
	return true;
}


#include <windows.h>
#include <string.h>    // strlen, _stricmp, memcpy

// Appends ext (e.g. ".adf") if missing, then checks that the final path
// doesn’t already exist. On any error, pops up an MB and returns false.
BOOL MakeGoodOutputFilename(
	HWND        hwndDlg,    // parent window for MessageBox
	char* path,       // in/out buffer holding “C:\\somefile” or “C:\\somefile.adf”
	size_t      bufsize,    // total size of path[]
	const char* ext         // required extension, including the dot, e.g. ".adf"
) {
	size_t   len = strlen(path);
	size_t   extlen = strlen(ext);

	// 1) If it already ends with ext (case-insensitive), skip append
	if (len < extlen || _stricmp(path + len - extlen, ext) != 0)
	{
		// need to append → ensure there’s room for ext plus NUL
		if (len + extlen + 1 > bufsize)
		{
			MessageBoxA(
				hwndDlg,
				"Filename too long; cannot append the required extension.",
				"Error",
				MB_OK | MB_ICONERROR
			);
			return false;
		}

		// append ext and terminating NUL
		memcpy(path + len, ext, extlen + 1);
		len += extlen;
	}

	// 2) Check that the file doesn’t already exist
	if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
	{
		// Play Sound 1 --> Warning! / Error!
		HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		MessageBoxA(
			hwndDlg,
			"File already exists! Please choose another filename.",
			"Error",
			MB_OK
		);
		return false;
	}

	return true;
}

int PASCAL WinMain( HINSTANCE inst, HINSTANCE prevInst, LPSTR cmdLine, int show )
{
	MSG   msg;
	DWORD envLen;
	char  ArgArray[20][MAX_PATH];
	char  token[MAX_PATH];
	char* p;
	char* last;
	int   argCount;
	int   inQuote;
	int   ti;
	int   i;

	// Chiron TODO: Do we need this anymore? I think the Text Viewer is using just a regular viewer!
	// DONE: I guess not!
	// Must load BEFORE the dialog is created
	// LoadLibraryA("Riched20.dll");

	// 1) stash raw command line
	strcpy_s(gstrCmdLineArgs,
		sizeof gstrCmdLineArgs,
		cmdLine);

	// 2) parse filenames out into ArgArray
	argCount = 0;
	inQuote = 0;
	ti = 0;
	p = gstrCmdLineArgs;

	if (gstrCmdLineArgs[0] != '\0')
	{
		// tokenize on spaces, respecting quotes
		while (*p && argCount < _countof(ArgArray))
		{
			if (*p == '"')
			{
				inQuote = !inQuote;
			}
			else if (*p == ' ' && !inQuote && ti > 0)
			{
				token[ti] = '\0';
				strcpy_s(ArgArray[argCount++], MAX_PATH, token);
				ti = 0;
			}
			else if (*p != ' ' || inQuote)
			{
				token[ti++] = *p;
			}
			p++;
		}
		// final token
		if (ti > 0 && argCount < _countof(ArgArray))
		{
			token[ti] = '\0';
			strcpy_s(ArgArray[argCount++], MAX_PATH, token);
		}

		// last token → default path
		strcpy_s(g_defaultLocalPath,
			sizeof g_defaultLocalPath,
			ArgArray[argCount - 1]);
		PathRemoveFileSpecA(g_defaultLocalPath);
		PathAddBackslashA(g_defaultLocalPath);
	}
	else
	{
		// no args → use %USERPROFILE%
        envLen = GetEnvironmentVariableA(
		"USERPROFILE",
			g_defaultLocalPath,
			MAX_PATH
			);
			if (envLen == 0 || envLen >= MAX_PATH)
			{
				strcpy_s(g_defaultLocalPath,
					sizeof g_defaultLocalPath,
					"C:\\"); // Chiron TODO: This is using the C:\ as the default default path... I think this needs something better when I put a bunch of extra stuff into Preferences (Options). 
			}
			else
			{
				PathAddBackslashA(g_defaultLocalPath);
			}
	}

	// 3) standard initialization
	instance = inst;
	if (!RegisterAppClass(inst))
		return 1;

	{
		INITCOMMONCONTROLSEX icex = {
			sizeof(icex),
			ICC_LISTVIEW_CLASSES
		};
		InitCommonControlsEx(&icex);
	}

	ghcurNormal = LoadCursor(inst, MAKEINTRESOURCE(IDC_AMIGA_CURSOR));
	ghcurNo     = LoadCursor(inst, MAKEINTRESOURCE(IDC_AMIGA_NO_CIRCLE_CURSOR));
	ghcurDrag   = LoadCursor(inst, MAKEINTRESOURCE(IDC_POINTER_COPY));

	if (!ghcurNormal) {
		DWORD err = GetLastError();
		char buf[128];
		wsprintfA(buf,
			"Failed to load Amiga cursor (ID=%d)\nGetLastError() = %u",
			IDC_AMIGA_CURSOR, err);
		MessageBoxA(NULL, buf, "Cursor Load Error", MB_OK | MB_ICONERROR); // Chiron 2025: TODO: Is this just debugging? Is it needed? 
	}

	ReadOptions();
	adfEnvInitDefault();
	adfSetEnvFct(ADFError, ADFWarning, ADFVerbose);
	adfEnv.rwhAccess = ADFAccess; // ABOUT THAT WARNING *** NOTE THIS OFTEN THROWS AN ERROR BUT LIKE LEAVE IT ALONE FOR NOW! IT'S FINE... I think... Warning: 'typedef ': ignored on left of 'unsigned __int64' when no variable is declared
	adfEnv.progressBar = ADFProgress;
	adfEnv.useRWAccess = TRUE;
	adfEnv.useProgressBar = TRUE;
	adfEnv.useDirCache = Options.useDirCache;

	GetModuleFileName(NULL, dirOpus, MAX_PATH);
	last = strrchr(dirOpus, '\\');
	if (last)
		*last = '\0';
	SetCurrentDirectory(dirOpus);

	strcpy_s(dirTemp, sizeof dirTemp, dirOpus);
	strcat_s(dirTemp, sizeof dirTemp, "\\opustemp\\");

	// 4) create main frame & MDI client
	ghwndFrame = CreateAppWindow(inst);
	if (!ghwndFrame)
		goto CLEANUP;

	ShowWindow(ghwndFrame, show);
	UpdateWindow(ghwndFrame);
	// … after ShowWindow/UpdateWindow …

	// … after ShowWindow/UpdateWindow …
	SetClassLong(ghwndFrame, GCL_HCURSOR, (LONG)ghcurNormal);
	SetCursor(ghcurNormal);

	// TODO: WTF?!?!?!?! This does nothing??!??!?!
	// 5) spawn initial child windows
	if (argCount > 0)
	{
		for (i = 0; i < argCount; i++)
		{
			strcpy_s(gstrFileName,
				sizeof gstrFileName,
				ArgArray[i]);
			//CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
		}
	}
	else
	{
		//CreateChildWin(ghwndMDIClient, CHILD_WINLISTER);
	}

	// 6) message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateMDISysAccel(ghwndMDIClient, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

CLEANUP:
	adfEnvCleanUp();
	WriteOptions();
	UnregisterAllClasses(inst);
	return (int)msg.wParam;
}


#include <windows.h>

// Call this after CreateWindow/ShowWindow (once you know your final window size)
static void CenterWindow(HWND hwnd, int winWidth, int winHeight)
{
	// Get the primary monitor work‐area (excludes taskbar)
	RECT rcWork;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

	int screenW = rcWork.right - rcWork.left;
	int screenH = rcWork.bottom - rcWork.top;

	// Calculate top-left so the window is centered
	int x = rcWork.left + (screenW - winWidth) / 2;
	int y = rcWork.top + (screenH - winHeight) / 2;

	// Move & resize
	SetWindowPos(hwnd,
		NULL,
		x, y,
		winWidth, winHeight,
		SWP_NOZORDER);
}



#include <windows.h>
#include "resource.h"     // IDR_LISTERMENU, ID_FIL_*, IDI_*, etc.
#include "MenuIcons.h"    // InitMenuIcons, CleanupMenuIcons, OnMeasureItem, OnDrawItem
LRESULT CALLBACK MainWinProc(
	HWND   hwndFrame,
	UINT   wMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	HANDLE        hDrop;
	int           i, iCount;
	NMHDR* nmhdr = (NMHDR*)lParam;
	LPTOOLTIPTEXT lpTTT;

	switch (wMsg)
	{
	case WM_INITMENU:
	{
		// Re-evaluate enabled/disabled on the main menu bar
		HMENU hMenu = (HMENU)wParam;
		UpdateMenuItems(hMenu);
		break;
	}

	case WM_CREATE:
		CreateProc(hwndFrame);
		// SetWindowPos(hwndFrame, NULL, 0, 0, 800, 600, SWP_NOZORDER | SWP_NOMOVE);
		SetWindowPos(hwndFrame, NULL, 0, 0, 1024, 768, SWP_NOZORDER | SWP_NOMOVE);

		if (!gbFirstTime)
		{
			if (gstrCmdLineArgs[0] != '\0')
			{
				CreateChildWin(ghwndMDIClient, CHILD_WINLISTER);

				{
					char ArgArray[20][MAX_PATH];
					char token[MAX_PATH];
					int  argCount = 0, inQuote = 0, ti = 0;
					char* p = gstrCmdLineArgs;

					while (*p && argCount < _countof(ArgArray))
					{
						if (*p == '"')
							inQuote = !inQuote;
						else if (*p == ' ' && !inQuote && ti > 0)
						{
							token[ti] = '\0';
							strcpy_s(ArgArray[argCount++], MAX_PATH, token);
							ti = 0;
						}
						else if (*p != ' ' || inQuote)
						{
							token[ti++] = *p;
						}
						p++;
					}
					if (ti > 0 && argCount < _countof(ArgArray))
					{
						token[ti] = '\0';
						strcpy_s(ArgArray[argCount++], MAX_PATH, token);
					}

					for (i = 0; i < argCount; i++)
					{
						strcpy_s(gstrFileName, sizeof gstrFileName, ArgArray[i]);

						// CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);

						if (isCurrentlyOpen(gstrFileName)) {
							HINSTANCE hInst = GetModuleHandle(NULL);
							if (Options.playSounds) PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
							MessageBoxA(hwndFrame, gstrFileName, "Warning: This file is already open!", MB_OK);
							if (BringAmigaListerToFront(gstrFileName));
						}
						else {
							CreateChildWin(ghwndMDIClient, CHILD_AMILISTER); // Open the .adf file within this program!
						};/*end-if*/

					}

					bCmdLineArgs = TRUE;
				}
			}
			else
			{
				CreateChildWin(ghwndMDIClient, CHILD_WINLISTER);
			}

			gbFirstTime = TRUE;
		}
		break;

	case WM_PAINT:
		if (bCmdLineArgs)
		{
			HWND hwndActiveChild =
				(HWND)SendMessage(ghwndMDIClient,
					WM_MDIGETACTIVE, 0, 0);
			SendMessage(ghwndMDIClient, WM_MDINEXT,
				(WPARAM)hwndActiveChild, 0);
			SendMessage(ghwndMDIClient, WM_MDICASCADE, 0, 0);
			bCmdLineArgs = FALSE;
		}
		// Chiron TODO: Should the MDITILE_VERTICAL thing below go here instead of where it is? 
		PaintProc(hwndFrame);
		// Chiron TODO: Make this an option!
		// I'm pretty sure this is what I added that needs to be an option!
		if (Options.autoHoriTile) SendMessage(ghwndMDIClient, WM_MDITILE, MDITILE_VERTICAL, 0); 

		break;

	case WM_SIZE:
		SendMessage(ghwndTB, WM_SIZE, 0, 0);
		SendMessage(ghwndSB, WM_SIZE, 0, 0);
		ResizeMDIClientWin(hwndFrame, ghwndMDIClient);
		break;

	case WM_COMMAND:
		if (!CommandProc(hwndFrame, wParam, lParam))
		{
			return DefFrameProc(hwndFrame,
				ghwndMDIClient,
				wMsg, wParam, lParam);
		}
		break;

	case WM_DROPFILES:
		hDrop = (HANDLE)wParam;
		iCount = DragQueryFile(hDrop, 0xFFFFFFFF,
			gstrFileName, sizeof gstrFileName);
		for (i = 0; i < iCount; i++)
		{
			DragQueryFile(hDrop, i, gstrFileName, sizeof gstrFileName);
			
			//CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);

			if (isCurrentlyOpen(gstrFileName)) {
				HINSTANCE hInst = GetModuleHandle(NULL);
				if (Options.playSounds) PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
				MessageBoxA(hwndFrame, gstrFileName, "Warning: This file is already open!", MB_OK);
				if (BringAmigaListerToFront(gstrFileName));
			}
			else {
				CreateChildWin(ghwndMDIClient, CHILD_AMILISTER); // Open the .adf file within this program!
			};/*end-if*/

		}
		DragFinish(hDrop);
		break;

	case WM_INITMENUPOPUP:
	{
		BOOL  isSysMenu = (BOOL)HIWORD(lParam);
		HMENU hPopup = (HMENU)wParam;

		// Let the MDI frame do merging & disabling
		DefFrameProc(hwndFrame,
			ghwndMDIClient,
			WM_INITMENUPOPUP,
			wParam,
			lParam);

		// Re-run custom enable/disable logic
		UpdateMenuItems(hPopup);

		// Set checks on View→Toolbar and View→Status Bar
		CheckMenuItem(hPopup,
			ID_VIEW_TOOL_BAR,
			MF_BYCOMMAND |
			(gbToolbarVisible
				? MF_CHECKED
				: MF_UNCHECKED));
		CheckMenuItem(hPopup,
			ID_VIEW_THE_STATUSBAR,
			MF_BYCOMMAND |
			(gbStatusBarVisible
				? MF_CHECKED
				: MF_UNCHECKED));

		if (!isSysMenu)
		{
			MENUINFO mi = { sizeof(mi) };
			GetMenuInfo(hPopup, &mi);
			mi.fMask = MIM_STYLE;
			mi.dwStyle = mi.dwStyle | MNS_CHECKORBMP;
			SetMenuInfo(hPopup, &mi);

			InitMenuIcons(instance, hPopup);
		}

		return 0;
	}

	case WM_NOTIFY:
		if (nmhdr->code == TTN_NEEDTEXT)
		{
			lpTTT = (LPTOOLTIPTEXT)lParam;
			GetTooltipText(lpTTT->szText, wParam);
		}
		break;

	case WM_LBUTTONUP:
		if (gbIsDragging)
		{
			MainWinOnDrop();
			return 0;
		}
		return DefFrameProc(hwndFrame,
			ghwndMDIClient,
			wMsg, wParam, lParam);

	case WM_MOUSEMOVE:
		if (gbIsDragging)
		{
			MainWinOnDragOver(
				LOWORD(lParam),
				HIWORD(lParam)
			);
			return 0;
		}
		return DefFrameProc(hwndFrame,
			ghwndMDIClient,
			wMsg, wParam, lParam);

	case WM_KEYDOWN:
		if (gbIsDragging)
		{
			MainWinOnDrop();
			return 0;
		}
		break;

	case WM_MEASUREITEM:
		OnMeasureItem(hwndFrame,
			(MEASUREITEMSTRUCT*)lParam);
		return TRUE;

	case WM_DRAWITEM:
		OnDrawItem(hwndFrame,
			(DRAWITEMSTRUCT*)lParam);
		return TRUE;

	case WM_DESTROY:
		CleanupMenuIcons();
		PostQuitMessage(0);
		return 0;

	case WM_SETCURSOR:
		// wParam == (HWND)window under the mouse
		// lParam LOWORD gives hit-test code (HTCLIENT, HTCAPTION, etc.)
		// Only override for the client area:
		if (LOWORD(lParam) == HTCLIENT) {
			SetCursor(ghcurNormal);
			return TRUE;    // we’ve handled it
		}
		break;



	default:
		return DefFrameProc(hwndFrame,
			ghwndMDIClient,
			wMsg, wParam, lParam);
	}

	return 0;
}



BOOL CreateProc(HWND hwndFrame)
/* handler for the WM_CREATE message - creates the toolbar, status bar and
 * MDI client window
 */
{
	ghwndTB = CreateToolBar(hwndFrame);
	ghwndSB = CreateStatusBar(hwndFrame);
	ghwndImageList = CreateImageList(hwndFrame);
	ghwndMDIClient = CreateMDIClientWindow(hwndFrame);

	/* register this window as a shell drop target */
	DragAcceptFiles(hwndFrame, TRUE);

	return TRUE;
}

BOOL CommandProc(HWND win, WPARAM wp, LONG lp)
/* handles WM_COMMAND messages for the main window
 */
{
	CHILDINFO *ci;

	HWND hwndActiveChild;

	hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
	ci = (CHILDINFO *)GetWindowLong(hwndActiveChild, 0);
	switch(wp)
	{
	/* commands that are passed on to active MDI child window */
	case ID_EDIT_SELECTALL:
	case ID_EDIT_SELECTNONE:
	case ID_EDIT_INVERTSELECTION:
	case ID_VIEW_REFRESH:
	case ID_VIEW_SHOWUNDELETABLEFILES:
	case ID_ACTION_UPONELEVEL:
	case ID_ACTION_RENAME:
	case ID_ACTION_DELETE:
	case ID_ACTION_UNDELETE:
	case ID_ACTION_NEWDIRECTORY:
		hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
		SendMessage(hwndActiveChild, WM_COMMAND, wp, 0);
		break;

	/* commands from the file menu */
	case ID_FIL_NEW:
		DialogBox(instance, MAKEINTRESOURCE(IDD_NEWVOLUME), win, (DLGPROC) NewDlgProc);
		break;		
	case ID_FIL_OPEN:
		if (OpenDlg(win)) {

			//CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);

			if (isCurrentlyOpen(gstrFileName)) {
				HINSTANCE hInst = GetModuleHandle(NULL);
				if (Options.playSounds) PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
				MessageBoxA(win, gstrFileName, "Warning: This file is already open!", MB_OK);
				if (BringAmigaListerToFront(gstrFileName));
			}
			else {
				CreateChildWin(ghwndMDIClient, CHILD_AMILISTER); // Open the .adf file within this program!
			};/*end-if*/
		}
		break;
	case ID_FIL_OPENDEVICE: // Chiron 2025: TODO: I think I removed this from the program menu because it didn't work. Might be worth fixing and putting back maybe? 
		
		strcpy(gstrFileName, "|H1"); //// TODO: RDSK autodetection
		
		//CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);

		if (isCurrentlyOpen(gstrFileName)) {
			HINSTANCE hInst = GetModuleHandle(NULL);
			if (Options.playSounds) PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
			MessageBoxA(win, gstrFileName, "Warning: This file is already open!", MB_OK);
			if (BringAmigaListerToFront(gstrFileName));
		}
		else {
			CreateChildWin(ghwndMDIClient, CHILD_AMILISTER); // Open the .adf file within this program!
		};/*end-if*/

		break;

	case ID_FIL_CLOSE:
		hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
		SendMessage(hwndActiveChild, WM_CLOSE, 0, 0);
		break;
	case ID_FIL_INFORMATION:

#ifdef DEBUG_INFO
		// Debug info dialogue using adfVolumeInfoWin().
		hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
		ci = (CHILDINFO *)GetWindowLong(hwndActiveChild, 0);
		adfVolumeInfoWin(win, ci->vol);
#else
		// Normal dialogue.
		DialogBox(instance, MAKEINTRESOURCE(IDD_VOLINFO), win, (DLGPROC)VolInfoDlgProc);
#endif

		break;
	case ID_FIL_EXIT:
		SendMessage(win, WM_CLOSE, 0, 0l);
		break;

	/* commands from the view menu */
	case ID_VIEW_TOOL_BAR:
		gbToolbarVisible = ! gbToolbarVisible;
		SendMessage(ghwndFrame, WM_INITMENU, (WPARAM) ghmenuMain, 0);
		ShowWindow(ghwndTB, gbToolbarVisible ? SW_SHOW : SW_HIDE);
		ResizeMDIClientWin();			
		break;
	case ID_VIEW_THE_STATUSBAR:
		gbStatusBarVisible = ! gbStatusBarVisible;
		SendMessage(ghwndFrame, WM_INITMENU, (WPARAM) ghmenuMain, 0);
		ShowWindow(ghwndSB, gbStatusBarVisible ? SW_SHOW : SW_HIDE);
		ResizeMDIClientWin();
		break;
	case ID_VIEW_NEWWINDOWSLISTER:
		CreateChildWin(ghwndMDIClient, CHILD_WINLISTER);
		break;

	/* commands from the Action menu */
	case ID_ACTION_PROPERTIES:
	// The file properties dialogue.
		if(ci->isAmi)
			DialogBox(instance, MAKEINTRESOURCE(IDD_FILEPROPERTIES_AMI), win, (DLGPROC)PropertiesProcAmi);
		else
			DialogBox(instance, MAKEINTRESOURCE(IDD_FILEPROPERTIES_WIN), win, (DLGPROC)PropertiesProcWin);
		break;

	/* commands from the Tools menu */
	case ID_TOOLS_TEXT_VIEWER:
		DialogBox(instance, MAKEINTRESOURCE(IDD_TEXT_VIEWER), win, (DLGPROC)TextViewerProc);
		break;

	case ID_TOOLS_HEX_VIEWER:
		DialogBox(instance, MAKEINTRESOURCE(IDD_HEX_VIEWER), win, (DLGPROC)HexViewerProc);
		break;

	case ID_TOOLS_BATCHCONVERTER:
		DialogBox(instance, MAKEINTRESOURCE(IDD_BATCHCONVERTER), win, (DLGPROC)BatchConvertProc);
		break;

	case ID_TOOLS_DISPLAYBOOTBLOCK:
		DialogBox(instance, MAKEINTRESOURCE(IDD_DISPLAY_BOOTBLOCK), win, (DLGPROC)DisplayBootblockProc);
		break;
	
	case ID_TOOLS_INSTALL:
		InstallBootBlock(win, ci->vol, FALSE);
		EnableMenuItem(ghmenuMain, ID_TOOLS_INSTALL, MF_GRAYED);
		break;

	case ID_TOOLS_WRITE_RAW_BOOTBLOCK:
		RawWriteBootBlock(win, ci->vol, FALSE);
		EnableMenuItem(ghmenuMain, ID_TOOLS_WRITE_RAW_BOOTBLOCK, MF_GRAYED);
		break;

	case ID_TOOLS_GREASEWEAZLE:
		DialogBox(instance, MAKEINTRESOURCE(IDD_GREASEWEAZLE), win, (DLGPROC)GreaseweazleProc);
		break;

	case ID_TOOLS_GREASEWEAZLEWRITE:
		DialogBox(instance, MAKEINTRESOURCE(IDD_GREASEWEAZLE_WRITE), win, (DLGPROC)GreaseweazleProcWrite);
		break;

	case ID_TOOLS_OPTIONS:
		DialogBox(instance, MAKEINTRESOURCE(IDD_OPTIONS), win, (DLGPROC)OptionsProc);
		break;

	/* commands from the Window menu */
	case ID_WIN_CASCADE:
		SendMessage(ghwndMDIClient, WM_MDICASCADE, 0, 0l);
		break;
	case ID_WIN_TILEHORIZONTAL:
		SendMessage(ghwndMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0l);
		break;
	case ID_WIN_TILEVERTICAL:
		SendMessage(ghwndMDIClient, WM_MDITILE, MDITILE_VERTICAL, 0l);
		break;
	case ID_WIN_ARRANGEICONS:
		SendMessage(ghwndMDIClient, WM_MDIICONARRANGE, 0, 0l);
		break;
	case ID_WIN_CLOSEALL:
		hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
		while (hwndActiveChild != NULL) {
			SendMessage(hwndActiveChild, WM_CLOSE, 0, 0l);
			hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
		}
		break;

	/* commands from the Help menu */
	case ID_HELP_ABOUT:
		DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUT), win, (DLGPROC)AboutDlgProc);
		break;

	case IDM_HELP_LICENCE:
	{
		char exeDir[MAX_PATH], fullPath[MAX_PATH];
		if (GetModuleFileNameA(NULL, exeDir, MAX_PATH)) {
			char* slash = strrchr(exeDir, '\\');
			if (slash) *(slash + 1) = '\0';
			strcpy_s(fullPath, sizeof(fullPath), exeDir);
			strcat_s(fullPath, sizeof(fullPath), "Licence.txt");
			ShellExecuteA(win, "open", fullPath, NULL, NULL, SW_SHOWNORMAL);
		}
		else {
			ShellExecuteA(win, "open", "Licence.txt", NULL, NULL, SW_SHOWNORMAL);
		}
		return TRUE;
	}


	case IDM_HELP_README:
	{
		char exeDir[MAX_PATH], fullPath[MAX_PATH];
		if (GetModuleFileNameA(NULL, exeDir, MAX_PATH)) {
			char* slash = strrchr(exeDir, '\\');
			if (slash) *(slash + 1) = '\0';
			strcpy_s(fullPath, sizeof(fullPath), exeDir);
			strcat_s(fullPath, sizeof(fullPath), "Readme.txt");
			ShellExecuteA(win, "open", fullPath, NULL, NULL, SW_SHOWNORMAL);
		}
		else {
			ShellExecuteA(win, "open", "Readme.txt", NULL, NULL, SW_SHOWNORMAL);
		}
		return TRUE;
	}

	case IDM_HELP_CHM:
	{
		char exeDir[MAX_PATH], fullPath[MAX_PATH];
		if (GetModuleFileNameA(NULL, exeDir, MAX_PATH)) {
			char* slash = strrchr(exeDir, '\\');
			if (slash) *(slash + 1) = '\0';
			strcpy_s(fullPath, sizeof(fullPath), exeDir);
			strcat_s(fullPath, sizeof(fullPath), "ADFOpus2025Help.chm");
			ShellExecuteA(win, "open", fullPath, NULL, NULL, SW_SHOWNORMAL);
		}
		else {
			ShellExecuteA(win, "open", "ADFOpus2025Help.chm", NULL, NULL, SW_SHOWNORMAL);
		}
		return TRUE;
	}

	// Chiron 2025 TODO: Windows no longer supports hlp files. This should be removed. Maybe in the future something better can happen here. 
	// Implement help menu items.
	case ID_HELP_HELPTOPICS:
		//WinHelp(win, "ADFOpus.hlp>Opus_win", HELP_FINDER, 0L);
		return TRUE;
	
	case ID_HELP_MAIN:
		//WinHelp(win, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_MAIN);
		return TRUE;
	
	default:
		return(FALSE);
	}
	return(TRUE);
}

VOID PaintProc(HWND win)
/* handles WM_PAINT messages for the MDI client window
 */
{
	HDC dc;
	PAINTSTRUCT ps;

	dc = BeginPaint(win, &ps);
	if(dc)
		EndPaint(win, &ps);

	return;
}

VOID DestroyProc(HWND hwndFrame)
/* game over
 */
{
	WIN32_FIND_DATA	fileFound;
	HANDLE			hFile;
	char			lpFileName[MAX_PATH];

	// Delete any files left in temp directory.
	strcpy(tempName , dirTemp);
	strcat(tempName, "*.*");			// Include any files extracted by GetFileFromADF().
	strcpy(lpFileName, tempName);

	hFile = FindFirstFile(lpFileName, &fileFound);
	if(hFile != INVALID_HANDLE_VALUE){
		strcpy(tempName, dirTemp);
		strcat(tempName, fileFound.cFileName);
		remove(tempName);
		while(FindNextFile(hFile, &fileFound)){
			strcpy(tempName, dirTemp);
				strcat(tempName, fileFound.cFileName);
			remove(tempName);
		}
	}
	FindClose(hFile);

	(void)_chdir(dirOpus);
	(void)_rmdir(dirTemp);										// Remove "Temp" directory.
	
	//WinHelp(hwndFrame, "ADFOpus.hlp", HELP_QUIT, 0L);		// Free help resources.
//	DeleteObject(bmpMenu);									// Delete menu bitmap.

	PostQuitMessage(0);

	return;
}

/* the callback functions for ADFLib - simply redirects the messages to a
 * standard Windows message box
 */
VOID ADFError(char *strMessage)
{
	// Play Sound 1 --> Warning! / Error!
	HINSTANCE hInst = GetModuleHandle(NULL);
	if (Options.playSounds)
		PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
	/*end-if*/
	MessageBox(ghwndFrame, strMessage, "ADFLib Error", MB_OK );
}

VOID ADFWarning(char *strMessage)
{
	// Play Sound 1 --> Warning! / Error!
	HINSTANCE hInst = GetModuleHandle(NULL);
	if (Options.playSounds)
		PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
	/*end-if*/
	MessageBox(ghwndFrame, strMessage, "ADFLib Warning", MB_OK );
}

VOID ADFVerbose(char *strMessage)
{
	// Play Sound 1 --> Warning! / Error!
	HINSTANCE hInst = GetModuleHandle(NULL);
	if (Options.playSounds)
		PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
	/*end-if*/

	MessageBox(ghwndFrame, strMessage, "ADFLib Message", MB_OK );
}

void ADFAccess(SECTNUM physical, SECTNUM logical, BOOL write) // ABOUT THAT WARNING *** NOTE THIS OFTEN THROWS AN ERROR BUT LIKE LEAVE IT ALONE FOR NOW! IT'S FINE... I think... Warning: 'typedef ': ignored on left of 'unsigned __int64' when no variable is declared
{
	CurrentSect = physical;
}

void ADFProgress(int perCentDone)
{
	Percent = perCentDone;
}

void MainWinOnDragOver(int x, int y)
{
	POINT pt;
	HWND target = NULL;
	long type;
	RECT rTB;
	CHILDINFO *ci;

	pt.x = x;
	pt.y = y;

	// Chiron 2025 TODO: Maybe this is where I can have the dragging of files from outside the App, like Windows Explorer, figure out if it's being dragged over one open Amiga disk image verses another. Not sure. There's another place where this might be handled so I'm not sure where the best place in the program is to deal with these dragged over files. 
	/* the co-ords are from the frame window - convert them to MDI client co-ords */
	if (gbToolbarVisible) {
		GetWindowRect(ghwndTB, &rTB);
		pt.y -= (rTB.bottom - rTB.top);
	}

	target = ChildWindowFromPoint(ghwndMDIClient, pt);

	if ((target == NULL) || (target == ghwndDragSource)) {
		SetCursor(ghcurNo);
		ghwndDragTarget = NULL;
	} else {
		type = GetWindowLong(target, GWL_USERDATA);
		ghwndDragTarget = NULL;

		if ((type == CHILD_AMILISTER) || (type == CHILD_WINLISTER)) {
			ci = (CHILDINFO *)GetWindowLong(target, 0);
			if (! ( (type == CHILD_WINLISTER) && (ci->atRoot) ) )
				ghwndDragTarget = target;
			if (ci->readOnly)
				ghwndDragTarget = NULL;
		}
		SetCursor((ghwndDragTarget == NULL) ? ghcurNo : ghcurDrag);
	}
}

void MainWinOnDrop()
{
	// Chiron 2025 TODO: Maybe this is where I can have the dragging of files from outside the App? There's like 3 or more places where this maybe should be figured out and handled. 
	/* stop the dragging action */
	gbIsDragging = FALSE;
	ReleaseCapture();
	SetCursor(ghcurNormal);

	if (ghwndDragTarget == NULL)
		return;

	CopySource = (CHILDINFO *)GetWindowLong(ghwndDragSource, 0);
	CopyDest = (CHILDINFO *)GetWindowLong(ghwndDragTarget, 0);
	Done = FALSE;
	Percent = 0;

	_beginthread(doCopy, 0, ghwndFrame);
	DialogBox(instance, MAKEINTRESOURCE(IDD_PROGRESS2), ghwndFrame, (DLGPROC)copyProgressProc);

	/* refresh destination lister */
	SendMessage(ghwndDragTarget, WM_COMMAND, ID_VIEW_REFRESH, (LPARAM)NULL);
}

void doCopy(void *arse)
{
	char curFile[256];
	char destPath[MAX_PATH];
	char srcPath[MAX_PATH];
	int i;
	BOOL isDir;

	for (i = 0 ; i < ListView_GetItemCount(CopySource->lv) ; i++) {
		if (LVIsItemSelected(CopySource->lv, i)) {
			LVGetItemCaption(CopySource->lv, curFile, sizeof(curFile), i);
			isDir = ((LVGetItemImageIndex(CopySource->lv, i) == ICO_AMIDIR) || (LVGetItemImageIndex(CopySource->lv, i)
				== ICO_WINDIR));

			/* ami to win */
			if ((CopySource->isAmi == TRUE) && (CopyDest->isAmi == FALSE)) {
				strcpy(destPath, CopyDest->curDir);
				strcat(destPath, curFile);
				if (isDir)
					CopyAmiDir2Win(curFile, destPath, CopySource->vol);
				else
					CopyAmi2Win(curFile, destPath, CopySource->vol, AOGetFileSize(CopySource, curFile));
			}

			/* win to ami */
			if ((CopySource->isAmi == FALSE) && (CopyDest->isAmi == TRUE)) {
				strcpy(srcPath, CopySource->curDir);
				strcat(srcPath, curFile);
				if (isDir)
					CopyWinDir2Ami(curFile, srcPath, CopyDest->vol);
				else
					CopyWin2Ami(curFile, srcPath, CopyDest->vol, AOGetFileSize(CopySource, curFile));
			}

			/* win to win */
			if ((CopySource->isAmi == FALSE) && (CopyDest->isAmi == FALSE)) {
				strcpy(srcPath, CopySource->curDir);
				strcpy(destPath, CopyDest->curDir);
				if (isDir)
					CopyWinDir2Win(srcPath, destPath, curFile);
				else {
					strcat(srcPath, curFile);
					strcat(destPath, curFile);
					CopyWin2Win(srcPath, destPath);
				}
			}

			/* ami to ami */
			if ((CopySource->isAmi == TRUE) && (CopyDest->isAmi == TRUE))
				if (isDir)
					CopyAmiDir2Ami(curFile, CopySource->vol, CopyDest->vol);
				else
					CopyAmi2Ami(curFile, CopySource->vol, CopyDest->vol, AOGetFileSize(CopySource, curFile));
		}
	}

	Done = TRUE;
}





#include <windows.h>

// Helper: convert Amiga date (days since 1 Jan 1978), minutes since midnight,
// and ticks (1/50 s) into a SYSTEMTIME.
static void AmiDateToSystemTime(
	LONG days,
	LONG mins,
	LONG ticks,
	SYSTEMTIME* pSt
)
{
	// 1 Jan 1978 epoch
	int year = 1978;
	LONG dayCount = days;

	// Roll forward year by year
	while (true)
	{
		bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
		int daysInYear = leap ? 366 : 365;
		if (dayCount >= daysInYear)
		{
			dayCount -= daysInYear;
			++year;
		}
		else
		{
			break;
		}
	}

	// Month/day within that year
	static const int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	int month = 1;
	for (int i = 0; i < 12; ++i)
	{
		int dim = mdays[i];
		if (i == 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
			dim = 29;
		if (dayCount >= dim)
		{
			dayCount -= dim;
			++month;
		}
		else
		{
			break;
		}
	}

	int day = (int)dayCount + 1;
	int hour = (int)(mins / 60);
	int minute = (int)(mins % 60);
	int second = (int)(ticks / 50);
	int msec = (int)((ticks % 50) * 20);  // each tick = 20 ms

	pSt->wYear = (WORD)year;
	pSt->wMonth = (WORD)month;
	pSt->wDay = (WORD)day;
	pSt->wHour = (WORD)hour;
	pSt->wMinute = (WORD)minute;
	pSt->wSecond = (WORD)second;
	pSt->wMilliseconds = (WORD)msec;
	pSt->wDayOfWeek = 0;  // not required for FileTime conversion
}

void CopyAmi2Win(char* fileName, char* destPath, struct Volume* vol, long fileSize)
{
	struct File* amiFile;
	long act;
	DWORD dwActual;
	unsigned char buf[600];
	HANDLE winFile;
	long bread = 0l;
	HINSTANCE hInst = GetModuleHandle(NULL);

	if (fileSize <= 0)
	{
		MessageBox(ghwndFrame,
			"Can't copy zero byte file",
			"ADF Opus Error",
			MB_OK | MB_ICONERROR);
		return;
	}

	winFile = CreateFile(destPath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (winFile == INVALID_HANDLE_VALUE)
	{
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
				hInst,
				SND_RESOURCE | SND_ASYNC);

		MessageBox(ghwndFrame,
			"Couldn't create destination file.",
			"Error",
			MB_OK);
		return;
	}

	amiFile = adfOpenFile(vol, fileName, "r");
	if (!amiFile)
	{
		CloseHandle(winFile);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
				hInst,
				SND_RESOURCE | SND_ASYNC);

		MessageBox(ghwndFrame,
			"Error opening source file.", // "Error opening source file (probably a bug)",
			"Error",
			MB_OK);
		return;
	}

	// copy data
	while (!adfEndOfFile(amiFile))
	{
		act = adfReadFile(amiFile, sizeof(buf), buf);
		bread += act;
		if (!WriteFile(winFile, buf, act, &dwActual, NULL))
		{
			CloseHandle(winFile);
			if (Options.playSounds)
				PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
					hInst,
					SND_RESOURCE | SND_ASYNC);

			MessageBox(ghwndFrame,
				"Error writing destination file (disk full?)",
				"Error",
				MB_OK);
			adfCloseFile(amiFile);
			return;
		}
		Percent = (100 * bread) / fileSize;
	}

	// === NEW: Set creation and modified timestamps ===
	{
		SYSTEMTIME stLocal;
		AmiDateToSystemTime(amiFile->fileHdr->days,
			amiFile->fileHdr->mins,
			amiFile->fileHdr->ticks,
			&stLocal);

		// Local SYSTEMTIME -> local FILETIME -> UTC FILETIME
		FILETIME ftLocal, ftUTC;
		SystemTimeToFileTime(&stLocal, &ftLocal);
		LocalFileTimeToFileTime(&ftLocal, &ftUTC);

		// Apply the same timestamp to creation, access, and write times
		SetFileTime(winFile, &ftUTC, &ftUTC, &ftUTC);
	}

	adfCloseFile(amiFile);
	CloseHandle(winFile);
}









#include <windows.h>

// Helper: convert a LOCAL SYSTEMTIME into Amiga days/mins/ticks
static void SystemTimeToAmiDate(
	const SYSTEMTIME* st,
	LONG* outDays,
	LONG* outMins,
	LONG* outTicks
)
{
	// 1 Jan 1978 epoch
	LONG days = 0;
	for (int y = 1978; y < st->wYear; ++y)
	{
		bool leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
		days += leap ? 366 : 365;
	}

	static const int mdays[12] =
	{ 31,28,31,30,31,30,31,31,30,31,30,31 };
	bool isLeap = (st->wYear % 4 == 0 &&
		(st->wYear % 100 != 0 || st->wYear % 400 == 0));
	for (int m = 1; m < st->wMonth; ++m)
	{
		int dim = mdays[m - 1];
		if (m == 2 && isLeap) dim = 29;
		days += dim;
	}

	days += (st->wDay - 1);

	*outDays = days;
	*outMins = st->wHour * 60 + st->wMinute;
	*outTicks = st->wSecond * 50 + (st->wMilliseconds / 20);
}

void CopyWin2Ami(
	char* fileName,
	char* srcPath,
	struct Volume* vol,
	long         fileSize
)
{
	struct File* amiFile;
	HANDLE       winFile;
	unsigned char buf[600];
	DWORD         act;
	char          errMess[200];
	long          bread = 0, bn;
	HINSTANCE     hInst = GetModuleHandle(NULL);

	if (fileSize <= 0)
	{
		MessageBox(ghwndFrame,
			"Can't copy zero byte file",
			"ADF Opus Error",
			MB_OK | MB_ICONERROR);
		return;
	}

	bn = adfFileRealSize(fileSize, LOGICAL_BLOCK_SIZE, NULL, NULL);
	if (adfCountFreeBlocks(vol) < bn)
	{
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
				hInst,
				SND_RESOURCE | SND_ASYNC);

		MessageBox(ghwndFrame,
			"Could not copy file. There is insufficient free space on the destination volume.",
			"Error",
			MB_OK);
		return;
	}

	// 1) Open the source Windows file
	winFile = CreateFile(srcPath,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (winFile == INVALID_HANDLE_VALUE)
	{
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
				hInst,
				SND_RESOURCE | SND_ASYNC);

		MessageBox(ghwndFrame,
			"Couldn't open source file.",
			"Error",
			MB_OK);
		return;
	}

	// 2) Read its Creation time and convert to Amiga date fields
	{
		FILETIME ftCreate, ftAccess, ftWrite;
		if (GetFileTime(winFile, &ftCreate, &ftAccess, &ftWrite))
		{
			// UTC FILETIME -> local FILETIME -> SYSTEMTIME (local)
			FILETIME   ftLocal;
			SYSTEMTIME stLocal;
			FileTimeToLocalFileTime(&ftCreate, &ftLocal);
			FileTimeToSystemTime(&ftLocal, &stLocal);

			LONG days, mins, ticks;
			SystemTimeToAmiDate(&stLocal, &days, &mins, &ticks);

			// 3) Open (or create) the Amiga file and set its header
			amiFile = adfOpenFile(vol, fileName, "w");
			if (!amiFile)
			{
				CloseHandle(winFile);
				if (Options.playSounds)
					PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
						hInst,
						SND_RESOURCE | SND_ASYNC);

				MessageBox(ghwndFrame,
					"Error opening destination file (volume full perhaps?)",
					"Error",
					MB_OK);
				return;
			}

			amiFile->fileHdr->days = days;
			amiFile->fileHdr->mins = mins;
			amiFile->fileHdr->ticks = ticks;
		}
		else
		{
			// Fallback: still need to open the Amiga file if timestamp fetch fails
			amiFile = adfOpenFile(vol, fileName, "w");
			if (!amiFile)
			{
				CloseHandle(winFile);
				MessageBox(ghwndFrame,
					"Error opening destination file after time fetch failed.",
					"Error",
					MB_OK);
				return;
			}
		}
	}

	// 4) Copy data from Windows file into Amiga file
	act = 1;
	while (act > 0)
	{
		ReadFile(winFile, buf, sizeof(buf), &act, NULL);
		bread += act;
		if (adfWriteFile(amiFile, act, buf) != (long)act)
		{
			CloseHandle(winFile);
			adfCloseFile(amiFile);
			if (Options.playSounds)
				PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
					hInst,
					SND_RESOURCE | SND_ASYNC);

			sprintf(errMess,
				"Could not write file '%s'. Not enough free space on volume.",
				fileName);
			MessageBox(ghwndFrame, errMess, "Error", MB_OK);
			return;
		}
		Percent = (100 * bread) / fileSize;
	}

	// 5) Close handles (adfCloseFile will write out your updated date fields)
	adfCloseFileNoDate(amiFile);
	CloseHandle(winFile);

}






#include <windows.h>

void CopyWin2Win(char* srcPath, char* destPath)
{
	HANDLE    srcFile, destFile;
	unsigned char buf[600];
	DWORD     readBytes = 1, written = 0, highSize = 0;
	long      fileSize, bread = 0;
	FILETIME  ftCreate, ftAccess, ftWrite;
	HINSTANCE hInst = GetModuleHandle(NULL);

	// 1) Open source file for reading
	srcFile = CreateFile(
		srcPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (srcFile == INVALID_HANDLE_VALUE) {
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
				hInst,
				SND_RESOURCE | SND_ASYNC);
		MessageBox(ghwndFrame,
			"Couldn't open source file.",
			"Error",
			MB_OK);
		return;
	}

	// 2) Grab the source file's timestamps
	if (!GetFileTime(srcFile, &ftCreate, &ftAccess, &ftWrite)) {
		// If timestamp retrieval fails, zero them (destFile will get "now")
		ZeroMemory(&ftCreate, sizeof(ftCreate));
		ZeroMemory(&ftAccess, sizeof(ftAccess));
		ZeroMemory(&ftWrite, sizeof(ftWrite));
	}

	// 3) Determine source file size (for progress calculation)
	fileSize = (long)GetFileSize(srcFile, &highSize);

	// 4) Create destination file for writing
	destFile = CreateFile(
		destPath,
		GENERIC_WRITE | FILE_WRITE_ATTRIBUTES,
		0,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL
	);
	if (destFile == INVALID_HANDLE_VALUE) {
		CloseHandle(srcFile);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
				hInst,
				SND_RESOURCE | SND_ASYNC);
		MessageBox(ghwndFrame,
			"Couldn't create destination file.",
			"Error",
			MB_OK);
		return;
	}

	// 5) Copy the data
	while (readBytes > 0) {
		ReadFile(srcFile, buf, sizeof(buf), &readBytes, NULL);
		if (readBytes == 0) break;

		if (!WriteFile(destFile, buf, readBytes, &written, NULL)
			|| written != readBytes) {
			if (Options.playSounds)
				PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
					hInst,
					SND_RESOURCE | SND_ASYNC);
			MessageBox(ghwndFrame,
				"Error writing destination file. Maybe disk full?",
				"Error",
				MB_OK);
			CloseHandle(srcFile);
			CloseHandle(destFile);
			return;
		}
		bread += readBytes;
		Percent = (100 * bread) / fileSize;
	}

	// 6) Apply the original timestamps to the new file
	SetFileTime(destFile, &ftCreate, &ftAccess, &ftWrite);

	// 7) Clean up
	CloseHandle(srcFile);
	CloseHandle(destFile);
}






void CopyAmi2Ami(char *fileName, struct Volume *srcVol,	struct Volume *destVol, long fileSize)
{
	struct File *srcFile, *destFile;
	long read, written;
	unsigned char buf[600];
	long bread = 0;
	long bn;
	HINSTANCE hInst = GetModuleHandle(NULL);


	// Backup the parent folder's date information. 






	// Prevent divide by zero and other errors.
	if(fileSize <= 0){
		// Play Sound 1 --> Warning! / Error!
		// HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		MessageBox(ghwndFrame, "Can't copy zero byte file", "ADF Opus Error", MB_OK);
		return;
	}

	bn = adfFileRealSize(fileSize, LOGICAL_BLOCK_SIZE, NULL, NULL);
	if (adfCountFreeBlocks(destVol) < bn) {
		// Play Sound 1 --> Warning! / Error!
		// HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		MessageBox(ghwndFrame, "Could not copy file. There is insufficient "
			"free space on the destination volume.", "Error", MB_OK);
		return;
	}

	/* open source file */
	srcFile = adfOpenFile(srcVol, fileName, "r");

	if (! srcFile) {
		// Play Sound 1 --> Warning! / Error!
		// HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		//MessageBox(ghwndFrame, "Error opening source file (probably a bug).", "Error", MB_OK);
		MessageBox(ghwndFrame, "Error opening source file.", "Error", MB_OK);
		return;
	}

	/* open dest file */
	destFile = adfOpenFile(destVol, fileName, "w");

	if (! destFile) {
		adfCloseFile(srcFile);
		// Play Sound 1 --> Warning! / Error!
		// HINSTANCE hInst = GetModuleHandle(NULL);
		if (Options.playSounds)
			PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
		/*end-if*/
		//MessageBox(ghwndFrame, "Error opening destination file (probably a bug).", "Error", MB_OK);
		MessageBox(ghwndFrame, "Error opening destination file.", "Error", MB_OK);
		return;
	}

	/* copy data */
	while(! adfEndOfFile(srcFile)) {
		read = adfReadFile(srcFile, sizeof(buf), buf);
		bread += read;
		written = adfWriteFile(destFile, read, buf);
		if (written != read) {
			adfCloseFile(srcFile);
			adfCloseFile(destFile);
			// Play Sound 1 --> Warning! / Error!
			// HINSTANCE hInst = GetModuleHandle(NULL);
			if (Options.playSounds)
				PlaySound(MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1), hInst, SND_RESOURCE | SND_ASYNC);
			/*end-if*/
			MessageBox(ghwndFrame, "Error writing destination file (volume full?).",
				"ADF Opus Error", MB_OK);
		}
		Percent = (100 * bread) / fileSize;
	}

	// Copy Comments
	strcpy(destFile->fileHdr->comment, srcFile->fileHdr->comment);

	// Copy Comment Length (char)
	destFile->fileHdr->commLen = srcFile->fileHdr->commLen;

	// Copy Access Flags (long)
	destFile->fileHdr->access = srcFile->fileHdr->access;

	// Copy Date and Time (longs)
	destFile->fileHdr->days  = srcFile->fileHdr->days;	//long	days;  /* Date of last change (days since 1 jan 78).			    */
	destFile->fileHdr->mins  = srcFile->fileHdr->mins;	//long	mins;  /* Time of last change (mins since midnight).			    */
	destFile->fileHdr->ticks = srcFile->fileHdr->ticks;	//long	ticks; /* Time of last change (1/50ths of a second since last min). */

	adfCloseFileNoDate(srcFile);
	adfCloseFileNoDate(destFile);


	// Restore the parent folder's date information.



}

void GetTooltipText(char *buf, int cmd)
{
	switch(cmd) {
	case ID_FIL_NEW:
		strcpy(buf, "New");
		break;
	case ID_FIL_OPEN:
		strcpy(buf, "Open");
		break;
	case ID_FIL_CLOSE:
		strcpy(buf, "Close");
		break;
	case ID_FIL_INFORMATION:
		strcpy(buf, "Information");
		break;
	case ID_ACTION_RENAME:
		strcpy(buf, "Rename");
		break;
	case ID_ACTION_DELETE:
		strcpy(buf, "Delete");
		break;
	case ID_ACTION_NEWDIRECTORY:
		strcpy(buf, "New Directory");
		break;
	case ID_ACTION_PROPERTIES:
		strcpy(buf, "Properties");
		break;
	case ID_VIEW_NEWWINDOWSLISTER:
		strcpy(buf, "New Windows Lister");
		break;
	case ID_ACTION_UPONELEVEL:
		strcpy(buf, "Up One Level");
		break;
	case ID_VIEW_SHOWUNDELETABLEFILES:
		strcpy(buf, "Show Undeletable Files");
		break;
	case ID_ACTION_UNDELETE:
		strcpy(buf, "Undelete");
		break;
	case ID_TOOLS_TEXT_VIEWER:
		strcpy(buf, "Text Viewer");
		break;
	case ID_TOOLS_HEX_VIEWER:
		strcpy(buf, "Hex Viewer");
		break;
	case ID_TOOLS_BATCHCONVERTER:
		strcpy(buf, "Batch Converter");
		break;
	case ID_TOOLS_GREASEWEAZLE:
		strcpy(buf, "Greaseweazle Read Floppy");
		break;
	case ID_TOOLS_GREASEWEAZLEWRITE:
		strcpy(buf, "Greaseweazle Write Floppy");
		break;
	case ID_TOOLS_INSTALL:
		strcpy(buf, "Install Bootblock");
		break;
	case ID_TOOLS_WRITE_RAW_BOOTBLOCK:
		strcpy(buf, "Raw Write Bootblock");
		break;
	case ID_TOOLS_DISPLAYBOOTBLOCK:
		strcpy(buf, "View Bootblock");
		break;
	case ID_TOOLS_OPTIONS:
		strcpy(buf, "Preferences");
		break;

	}
}
	

LRESULT CALLBACK copyProgressProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static char cs[20];

	switch (msg) {
	case WM_INITDIALOG:
		Timer = SetTimer(dlg, 1, 100, NULL);
		return TRUE;
	case WM_COMMAND:
		switch (wp) {
		case IDCANCEL:
			Done = TRUE;
			return TRUE;
		}
	case WM_TIMER:
		if (Done) {
			KillTimer(dlg, Timer);
			EndDialog(dlg, TRUE);
		}
		SendMessage(GetDlgItem(dlg, IDC_CURFILEPROGRESS), PBM_SETPOS, Percent, 0l);
		itoa(CurrentSect, cs, 10);
		SetDlgItemText(dlg, IDC_CURRENTSECTOR, cs);
		return TRUE;
	}

	return FALSE;
}

long AOGetFileSize(CHILDINFO *ci, char *fn)
{
	DIRENTRY *ce = ci->content;

	while (strcmp(ce->name, fn) != 0)
		ce = ce->next;

	return ce->size;
}


// Chiron 2025: TODO: I could turn this recursive bit into 
// a way to do an entire amiga disk image export! 
// Like an Export All or something!
BOOL CopyAmiDir2Win(char *srcDir, char *destPath, struct Volume *vol)
{
	char tp[4096];
	struct List *list;
	struct Entry *ent;

	CreateDirectory(destPath, NULL);

	adfChangeDir(vol, srcDir);
	list = adfGetDirEnt(vol, vol->curDirPtr);

	while (list) {
		ent = (struct Entry *)list->content;
		strcpy(tp, destPath);
		strcat(tp, "\\");
		strcat(tp, ent->name);
		if (ent->type == ST_DIR) {
			/* it's a dir - recurse into it */
			CopyAmiDir2Win(ent->name, tp, vol);
		} else {
			/* it's a file or a link, just copy it */
			CopyAmi2Win(ent->name, tp, vol, ent->size);
		}
		adfFreeEntry(list->content);
		list = list->next;
	}
	freeList(list);

	adfParentDir(vol);

	return TRUE;
}

BOOL CopyWinDir2Ami(char *srcDir, char *srcPath, struct Volume *vol)
{
	WIN32_FIND_DATA wfd;
	HANDLE search;
	char curPath[MAX_PATH * 2];
	char searchPath[MAX_PATH * 2];
	char subdir[MAX_PATH * 2];

	strcpy(curPath, srcPath);
	sprintf(searchPath, "%s\\*", curPath);

	adfCreateDir(vol, vol->curDirPtr, srcDir);
	adfChangeDir(vol, srcDir);

	search = FindFirstFile(searchPath, &wfd);
	if (search == INVALID_HANDLE_VALUE)
		return FALSE;

	do {
		/* if current entry is a dir, and isn't the current or parent dir, then copy it */
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if ( !((wfd.cFileName[1] == 0) && (wfd.cFileName[0] == '.')) )
				if ( !((wfd.cFileName[2] == 0) && (wfd.cFileName[1] == '.') && (wfd.cFileName[0] == '.')) ) {
					sprintf(subdir, "%s\\%s", curPath, wfd.cFileName);
					CopyWinDir2Ami(wfd.cFileName, subdir, vol);
					//RemoveDirectoryRecursive(subdir);
				}
		} else {
			/* it's a file so just copy it */
			sprintf(subdir, "%s\\%s", curPath, wfd.cFileName);
			CopyWin2Ami(wfd.cFileName, subdir, vol, wfd.nFileSizeLow);
		}

	} while (FindNextFile(search, &wfd));

	FindClose(search);

	adfParentDir(vol);

	return TRUE;
}

BOOL CopyWinDir2Win(char *srcPath, char *destPath, char *dirName)
{
	WIN32_FIND_DATA wfd;
	HANDLE search;
	char searchPath[MAX_PATH * 2];
	char subdir[MAX_PATH * 2];
	char subdir2[MAX_PATH * 2];

	sprintf(searchPath, "%s\\%s", destPath, dirName);
	CreateDirectory(searchPath, NULL);

	sprintf(searchPath, "%s\\%s\\*", srcPath, dirName);

	search = FindFirstFile(searchPath, &wfd);
	if (search == INVALID_HANDLE_VALUE)
		return FALSE;

	do {
		/* if current entry is a dir, and isn't the current or parent dir, then copy it */
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if ( !((wfd.cFileName[1] == 0) && (wfd.cFileName[0] == '.')) )
				if ( !((wfd.cFileName[2] == 0) && (wfd.cFileName[1] == '.') && (wfd.cFileName[0] == '.')) ) {
					sprintf(subdir, "%s\\%s", srcPath, dirName);
					sprintf(subdir2, "%s\\%s", destPath, dirName);
					CopyWinDir2Win(subdir, subdir2, wfd.cFileName);
				}
		} else {
			/* it's a file so just copy it */
			sprintf(subdir, "%s\\%s\\%s", srcPath, dirName, wfd.cFileName);
			sprintf(subdir2, "%s\\%s\\%s", destPath, dirName, wfd.cFileName);
			CopyWin2Win(subdir, subdir2);
		}

	} while (FindNextFile(search, &wfd));

	FindClose(search);

	return TRUE;
}





///////////////////////////////////////////////////////////////////////////////////
/////////////// DO NOT DELETE YET - I STILL WANT TO CRACK THIS NUT! //////////////////////
///////////////////////////////////////////////////////////////////////////////////

/* Original routine for reference */

//BOOL CopyAmiDir2Ami(char* dirName, struct Volume* src, struct Volume* dest)
//{
//	struct List* list;
//	struct Entry* ent;
//
//	adfCreateDir(dest, dest->curDirPtr, dirName);
//
//	adfChangeDir(src, dirName);
//	adfChangeDir(dest, dirName);
//
//	list = adfGetDirEnt(src, src->curDirPtr);
//
//	while (list) {
//		ent = (struct Entry*)list->content;
//		if (ent->type == ST_DIR) {
//			/* it's a dir - recurse into it */
//			CopyAmiDir2Ami(ent->name, src, dest);
//		}
//		else {
//			/* it's a file or a link, just copy it */
//			CopyAmi2Ami(ent->name, src, dest, ent->size);
//		}
//		adfFreeEntry(list->content);
//		list = list->next;
//	}
//	freeList(list);
//
//	adfParentDir(dest);
//	adfParentDir(src);
//
//	return TRUE;
//}

// Chiron 2025: TODO: Figure out how to copy the date and time from one folder to another properly!
//
// The function below DOES NOT WORK FOR COPYING THE DATE AND TIME!
// 
// Go to the file: adf_dir.c
// Look at this function: void printEntry(struct Entry* entry)
// 
// That function is for printing to the console as an example of something. 
// Basically the code below tries to use it for that, but that doesn't work!
// That's also why this was wrong: 
// days = e->days;   // days since 1/1/1978    <-- WRONG! That's the day of the month!
// mins = e->mins;   // minutes since midnight <-- WRONG! That's the mins part of the time of day!
// 
// So In order to do this right I need figure out how to read and write the epoch from 
// the headers. Like I did in the copy routine - look here: 
// 
//// Copy Comments
//strcpy(destFile->fileHdr->comment, srcFile->fileHdr->comment);
//
//// Copy Comment Length (char)
//destFile->fileHdr->commLen = srcFile->fileHdr->commLen;
//
//// Copy Access Flags (long)
//destFile->fileHdr->access = srcFile->fileHdr->access;
//
//// Copy Date and Time (longs)
//destFile->fileHdr->days = srcFile->fileHdr->days;	//long	days;      /* Date of last change (days since 1 jan 78).			    */
//destFile->fileHdr->mins = srcFile->fileHdr->mins;	//long	mins;      /* Time of last change (mins since midnight).			    */
//destFile->fileHdr->ticks = srcFile->fileHdr->ticks;	//long	ticks; /* Time of last change (1/50ths of a second since last min). */
//
// I tried to do something like that but I can't get it working.
// I tried to get Copilot to help but it kept
// getting hung up on fucking struct Entry* which is for nicey nice printing
// and not for what I want! Fuck! Why is this hard? 



BOOL CopyAmiDir2Ami(char* dirName, struct Volume* src, struct Volume* dest)
{
	struct List* list;
	struct Entry* ent;

	// buffers to hold the source folder’s metadata
	char  commentBuf[MAXCMMTLEN + 1];
	long  accessFlags;
	long  days, mins, ticks;

	//// Chiron 2025: TODO: This seems like total overkill. 
	//// Jesus Copilot what the fuck were you thinking!
	//// 1) Read the source‐dir’s comment, flags and date (days+mins)
	//{
	//	struct List* meta = adfGetDirEnt(src, src->curDirPtr);
	//	while (meta)
	//	{
	//		struct Entry* e = (struct Entry*)meta->content;
	//		if (e->type == ST_DIR && strcmp(e->name, dirName) == 0)
	//		{
	//			strcpy(commentBuf, e->comment);
	//			accessFlags = e->access;
	//			adfFreeEntry(meta->content);
	//			break;
	//		}
	//		adfFreeEntry(meta->content);
	//		meta = meta->next;
	//	}
	//	freeList(meta);
	//}
	
	// 2) Create the directory in the destination volume
	adfCreateDir(dest, dest->curDirPtr, dirName);

	struct File* srcDirFile;
	srcDirFile = adfOpenFile(src, dirName, "r");

	strcpy(commentBuf , srcDirFile->fileHdr->comment);
	accessFlags       = srcDirFile->fileHdr->access;

	days  = srcDirFile->fileHdr->days;
	mins  = srcDirFile->fileHdr->mins;
	ticks = srcDirFile->fileHdr->ticks;

	adfCloseFileNoDate(srcDirFile);

	// THESE WORK!!!
	// 
	// Immediately re‐apply comment & flags
	adfSetEntryComment(
		dest,
		dest->curDirPtr,
		dirName,
		commentBuf
	);
	
	adfSetEntryAccess(
		dest,
		dest->curDirPtr,
		dirName,
		accessFlags
	);

	// THIS WORKS!!!!!!!!!!
	// But...............
	// ...only if it's empty!
	// Writing (from coping) a new file inside 
	// actually causes the folder's date to 
	// when the file was copied. 
	adfSetEntryDate(
		dest,
		dest->curDirPtr,
		dirName,
		days,
		mins,
		ticks
	);











	//// 4) Now poke the days+mins back in, then flush via comment‐setter
	//{
	//	struct List* meta2 = adfGetDirEnt(dest, dest->curDirPtr);
	//	while (meta2)
	//	{
	//		struct Entry* e2 = (struct Entry*)meta2->content;
	//		if (e2->type == ST_DIR && strcmp(e2->name, dirName) == 0)
	//		{
	//			//e2->days = days;
	//			//e2->mins = mins;
	//			// flush the block (updates days/mins on disk)
	//			adfSetEntryComment(dest,
	//				dest->curDirPtr,
	//				dirName,
	//				e2->comment);
	//			adfFreeEntry(meta2->content);
	//			break;
	//		}
	//		adfFreeEntry(meta2->content);
	//		meta2 = meta2->next;
	//	}
	//	freeList(meta2);
	//}

	// 5) Descend on both volumes
	adfChangeDir(src, dirName);
	adfChangeDir(dest, dirName);

	// 6) Copy each child (recurse on subdirs)
	list = adfGetDirEnt(src, src->curDirPtr);
	while (list)
	{
		ent = (struct Entry*)list->content;
		if (ent->type == ST_DIR)
			CopyAmiDir2Ami(ent->name, src, dest);
		else
			CopyAmi2Ami(ent->name, src, dest, ent->size);

		adfFreeEntry(list->content);
		list = list->next;
	}
	freeList(list);

	// 7) Step back out
	adfParentDir(dest);
	adfParentDir(src);

	// RESTORE THE ORIGINAL DIRECTORY DATE
	// We've gone and copied all the files,
	// so this is where we restore the original
	// date that we recorded eariler. 
	// If we didn't do this then all the files
	// getting copied into the folder 
	// would trigger the ADFLib functions to 
	// update the date on the folder because 
	// that's a change. But it's not a change 
	// to the file, just a change as to when 
	// it was copied somewhere.
	adfSetEntryDate(
		dest,
		dest->curDirPtr,
		dirName,
		days,
		mins,
		ticks
	);

	// I didn't want to blow away the original
	// date just because the file was copied!
	// I want to be able to edit an Amiga disk
	// and copy files and preserve things like 
	// comments, access flags, and dates!
	// I look at ADF Opus through the lens
	// of editing an Amiga disk image, 
	// not through the lens on an 
	// Amiga Operating System. Yes an OS should
	// update these dates based on these kinds of 
	// changes. But this isn't that. This is an
	// editor and it's meant to edit these
	// disk images. Not pretend to *BE* an Amiga!
	// 
	// Here's a use case:
	// 
	// You want to create an archive of your old
	// Amiga word processing documents. So you go
	// through a bunch of floppy disk images and
	// put them into an Amiga hard disk image
	// that can hold them all in an organized 
	// fashion. Well if you loose all the dates
	// then later when you go through things you
	// can't say "Hey, when did I write this?"
	// because you've lost that data. You'd have 
	// to go through your floppy disks or disk 
	// images and figure out where it is and 
	// get the date from that. But what's the point
	// of recording when you copied those files 
	// into your archive in the first place? 
	// The files haven't changed. Sure, if you edit
	// a file, then showing the last date that the
	// file's content actually changed makes sense. 
	// But there's nothing to be gained from being
	// able to say: "I copied this file on this date
	// but I have no idea when the last change was
	// actually made to the file in a meaningful way."
	//
	// Maybe at some point I'll add an option to 
	// enable or disable the preservation of 
	// file and folder dates. I might. But I don't
	// even want to! 
	//
	// I've made my case and I think it's a good one!
	// 
	// Thank you for coming to my TEDTalk.

	return TRUE;

	// End of copy function. 

}