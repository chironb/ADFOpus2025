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

#include "ADFLib.h"
#include "Help\AdfOpusHlp.h"

extern char* adfGetVersionNumber(); /* this shouldn't be here */

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


int PASCAL WinMain(HINSTANCE inst, HINSTANCE prevInst, LPSTR cmdLine, int show)
/* windows entry point function - initialises everything and enters the
 * message loop
 */
{
	MSG msg;

	instance = inst;
	if (! RegisterAppClass(inst))
		return 1;

	/* load the common control dll */
	InitCommonControls();

	/* load cursors */
	ghcurNormal = LoadCursor(NULL, IDC_ARROW);
	ghcurNo = LoadCursor(NULL, IDC_NO);
	ghcurDrag = LoadCursor(instance, MAKEINTRESOURCE(IDC_POINTER_COPY));

	/* get user options */
	ReadOptions();

	/* initialise ADFLib */
	adfEnvInitDefault();
	adfSetEnvFct(ADFError, ADFWarning, ADFVerbose);

	adfEnv.rwhAccess = ADFAccess;
	adfEnv.progressBar = ADFProgress;
	adfEnv.useRWAccess = TRUE;
	adfEnv.useProgressBar = TRUE;
	adfEnv.useDirCache = Options.useDirCache;

	// Create path to temp directory in Opus root directory.
	getcwd(dirTemp, 100);
	strcpy(dirOpus, dirTemp);
	strcat(dirTemp, "\\opustemp\\");

	/* the main loop */
	msg.wParam = 1;

	// Store command line arguments for processing in WM_CREATE.
	strcpy(gstrCmdLineArgs, cmdLine);

	ghwndFrame = CreateAppWindow(inst);
	if (ghwndFrame)
	{
		ShowWindow(ghwndFrame, show);
		UpdateWindow(ghwndFrame);

		/* standard message loop for MDI apps */
		while(GetMessage(&msg, NULL, 0, 0))
		{
			if(! TranslateMDISysAccel(ghwndMDIClient, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	/* clean up ADFLib */
	adfEnvCleanUp();

	/* save options */
	WriteOptions();

	/* unregister all the classes we registered (not really needed but...) */
	UnregisterAllClasses(inst);

	return msg.wParam;
}

LRESULT CALLBACK MainWinProc(HWND hwndFrame, UINT wMsg, WPARAM wParam, LPARAM lParam)
/* handles all messages for the main window and passes messages on to the
 * MDI child windows where applicable
 */
{
	HANDLE			hDrop;
	int				i, iCount;
	NMHDR			*nmhdr = (NMHDR *) lParam;
	LPTOOLTIPTEXT	lpTTT;
	DWORD			dwVersion;							// OS version.


	switch(wMsg)
	{
	case WM_CREATE:
		CreateProc(hwndFrame);
		
		// Open files given on the command line.
		if(strcmp(gstrCmdLineArgs, "") != 0){

			char sep[] = " ";
			char *token;
			char ArgArray[20][MAX_PATH * 2];
		
			bCmdLineArgs = TRUE;

			// Get argument sub-strings as disk file names.
			i = 0;
			token = strtok(gstrCmdLineArgs, sep);   
			while(token != NULL){
				/* While there are tokens in "string" */      
				strcpy(ArgArray[i], token);
				/* Get next token: */      
				token = strtok(NULL, sep);
				i++;
			}

			/* open an Amiga lister child window for each file */
			for(iCount = 0;iCount < i;iCount++){
				strcpy(gstrFileName, ArgArray[iCount]);
				CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
			}
		}
		break;

	case WM_PAINT:

		// Push the Windows lister to the back and cascade the open windows if file names have been given on the
		// command line. Reset flag afterwards as this is a once-only event.
		if(bCmdLineArgs){
			HWND hwndActiveChild;
			hwndActiveChild = (HWND) SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
			SendMessage(ghwndMDIClient, WM_MDINEXT, (WPARAM)hwndActiveChild, 0l);
			SendMessage(ghwndMDIClient, WM_MDICASCADE, 0, 0l);
			bCmdLineArgs = FALSE;
		}

		PaintProc(hwndFrame);
		break;
	case WM_DESTROY:
		DestroyProc(hwndFrame);
		break;
	case WM_QUIT:
		_chdir(dirOpus);
		_rmdir(dirTemp);
		break;

	case WM_COMMAND:
		if (! CommandProc(hwndFrame, wParam, lParam))
			return(DefFrameProc(hwndFrame, ghwndMDIClient, wMsg, wParam, lParam));
		break;
	case WM_DROPFILES:
		/* file(s) dropped from explorer */
		hDrop = (HANDLE) wParam;

		/* find out how many files were dropped */
		iCount = DragQueryFile(hDrop, 0xFFFFFFFF, gstrFileName, sizeof(gstrFileName));

		/* open an Amiga lister child window for each file */
		for (i = 0 ; i < iCount ; i++) {
			DragQueryFile(hDrop, i, gstrFileName, sizeof(gstrFileName));
			CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);			
		}
		break;
	case WM_SIZE:
		/* resize toolbar and status bar */
		SendMessage(ghwndTB, WM_SIZE, 0, 0);
		SendMessage(ghwndSB, WM_SIZE, 0, 0);

		ResizeMDIClientWin(hwndFrame, ghwndMDIClient);

		if (! gbFirstTime) {
			CreateChildWin(ghwndMDIClient, CHILD_WINLISTER);
			gbFirstTime = TRUE;
		}
		break;
	case WM_INITMENU:
		ghmenuMain = (HMENU)wParam;
		UpdateMenuItems(ghmenuMain);
		CheckMenuItem(ghmenuMain, ID_VIEW_TOOL_BAR, gbToolbarVisible ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(ghmenuMain, ID_VIEW_THE_STATUSBAR, gbStatusBarVisible ?MF_CHECKED : MF_UNCHECKED);

		// Disable the Disk2FDI interface for NT, 2K and XP.
		dwVersion = GetVersion();
		// High order bit is 1 (0x80000000) for Windows NT, 2K or XP. If less, we have 95, 98 or ME.
		if(dwVersion < 0x80000000)                
			EnableMenuItem(ghmenuMain, ID_TOOLS_DISK2FDI, MF_GRAYED);

		SetMenuBitmaps(instance, ghmenuMain);
		break;
	case WM_LBUTTONUP:
		if (gbIsDragging)
			MainWinOnDrop();
		break;
	case WM_MOUSEMOVE:
		if (gbIsDragging)
			MainWinOnDragOver(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		if (gbIsDragging)
			MainWinOnDrop();
		break;
	case WM_NOTIFY:
		switch (nmhdr->code) {
		case TTN_NEEDTEXT:
			lpTTT = (LPTOOLTIPTEXT)lParam;
			GetTooltipText(lpTTT->szText, wParam);
		}
		break;
	default:
		return(DefFrameProc(hwndFrame, ghwndMDIClient, wMsg, wParam, lParam));
	}
	return(0l);
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
		if (OpenDlg(win))
			CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
		break;
	case ID_FIL_OPENDEVICE:
		strcpy(gstrFileName, "|H1"); //// TODO: RDSK autodetection
		CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
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

	case ID_TOOLS_DISK2FDI:
		DialogBox(instance, MAKEINTRESOURCE(IDD_DISK2FDI), win, (DLGPROC)Disk2FDIProc);
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
		DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUT), win, (DLGPROC) AboutDlgProc);
		break;

	// Implement help menu items.
	case ID_HELP_HELPTOPICS:
		WinHelp(win, "ADFOpus.hlp>Opus_win", HELP_FINDER, 0L);
		return TRUE;
	
	case ID_HELP_MAIN:
		WinHelp(win, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_MAIN);
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

	_chdir(dirOpus);
	_rmdir(dirTemp);										// Remove "Temp" directory.
	
	WinHelp(hwndFrame, "ADFOpus.hlp", HELP_QUIT, 0L);		// Free help resources.
//	DeleteObject(bmpMenu);									// Delete menu bitmap.

	PostQuitMessage(0);

	return;
}

/* the callback functions for ADFLib - simply redirects the messages to a
 * standard Windows message box
 */
VOID ADFError(char *strMessage)
{
	MessageBox(ghwndFrame, strMessage, "ADFLib Error", MB_OK | MB_ICONERROR);
}

VOID ADFWarning(char *strMessage)
{
	MessageBox(ghwndFrame, strMessage, "ADFLib Warning", MB_OK | MB_ICONWARNING);
}

VOID ADFVerbose(char *strMessage)
{
	MessageBox(ghwndFrame, strMessage, "ADFLib Message", MB_OK | MB_ICONINFORMATION);
}

void ADFAccess(SECTNUM physical, SECTNUM logical, BOOL write)
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

void CopyAmi2Win(char *fileName, char *destPath, struct Volume *vol, long fileSize)
{
	struct File *amiFile;
	long act;
	DWORD dwActual;
	unsigned char buf[600];
	HANDLE winFile;
	long bread = 0l;

	// Prevent divide by zero and other errors.
	if(fileSize <= 0){
		MessageBox(ghwndFrame, "Can't copy zero byte file", "ADF Opus Error", MB_OK | MB_ICONERROR);
		return;
	}

	winFile = CreateFile(destPath, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (winFile == INVALID_HANDLE_VALUE) {
		MessageBox(ghwndFrame, "Couldn't create destination file.",
		"Error", MB_OK | MB_ICONERROR);
		return;
	}

	amiFile = adfOpenFile(vol, fileName, "r");
	if (! amiFile) {
		CloseHandle(winFile);
		MessageBox(ghwndFrame, "Error opening source file (probably"
			" a bug)", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* copy data */
	while (! adfEndOfFile(amiFile)) {
		act = adfReadFile(amiFile, sizeof(buf), buf);
		bread += sizeof(buf);
		if (! WriteFile(winFile, buf, act, &dwActual, NULL)) {
			CloseHandle(winFile);
			MessageBox(ghwndFrame, "Error writing destination"
			" file (disk full, maybe?)", "Error", MB_OK | MB_ICONERROR);
			return;
		}
		Percent = (100 * bread) / fileSize;
	}
	adfCloseFile(amiFile);
	CloseHandle(winFile);
}

void CopyWin2Ami(char *fileName, char *srcPath, struct Volume *vol, long fileSize)
{
	struct File *amiFile;
	HANDLE winFile;
	unsigned char buf[600];
	DWORD act;
	char errMess[200];
	long bread = 0, bn;

	// Prevent divide by zero and other errors.
	if(fileSize <= 0){
		MessageBox(ghwndFrame, "Can't copy zero byte file", "ADF Opus Error", MB_OK | MB_ICONERROR);
		return;
	}

	bn = adfFileRealSize(fileSize, LOGICAL_BLOCK_SIZE, NULL, NULL);
	if (adfCountFreeBlocks(vol) < bn) {
		MessageBox(ghwndFrame, "Could not copy file. There is insufficient "
			"free space on the destination volume.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* open source file */
	winFile = CreateFile(srcPath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		0, NULL);
	if (winFile == INVALID_HANDLE_VALUE) {
		MessageBox(ghwndFrame, "Couldn't open source file.",
		"Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* open dest file */
	amiFile = adfOpenFile(vol, fileName, "w");

	if (! amiFile) {
		CloseHandle(winFile);
		MessageBox(ghwndFrame, "Error opening destination file (volume"
			" full perhaps?)", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* write the file */
	act = 1;
	while (act > 0) {
		ReadFile(winFile, buf, sizeof(buf), &act, NULL);
		bread += sizeof(buf);
		if (adfWriteFile(amiFile, act, buf) != (long)act) {
			CloseHandle(winFile);
			adfCloseFile(amiFile);
			sprintf(errMess, "Could not write file '%s'.  Not enough free space on volume.", fileName);
			MessageBox(ghwndFrame, errMess, "Error", MB_OK | MB_ICONERROR);
			return;
		}
		Percent = (100 * bread) / fileSize;
	}

	adfCloseFile(amiFile);
	CloseHandle(winFile);
}

void CopyWin2Win(char *srcPath, char *destPath)
{
	HANDLE srcFile, destFile;
	unsigned char buf[600];
	DWORD read, written, crap;
	long fileSize, bread = 0;

	/* open source file */
	srcFile = CreateFile(srcPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (srcFile == INVALID_HANDLE_VALUE) {
		MessageBox(ghwndFrame, "Couldn't open source file.",
		"Error", MB_OK | MB_ICONERROR);
		return;
	}

	fileSize = GetFileSize(srcFile, &crap);

	/* open dest file */
	destFile = CreateFile(destPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		0, NULL);
	if (destFile == INVALID_HANDLE_VALUE) {
		CloseHandle(srcFile);
		MessageBox(ghwndFrame, "Couldn't create destination file.",
		"Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* copy data */
	read = 1;
	while (read) {
		ReadFile(srcFile, buf, sizeof(buf), &read, NULL);
		bread += sizeof(buf);
		WriteFile(destFile, buf, read, &written, NULL);
		if (written != read) {
			MessageBox(ghwndFrame, "Error writing destination "
				"file.  Maybe disk full?", "Error", MB_OK |
				MB_ICONERROR);
			CloseHandle(srcFile);
			CloseHandle(destFile);
			return;
		}
		Percent = (100 * bread) / fileSize;
	}

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


	// Prevent divide by zero and other errors.
	if(fileSize <= 0){
		MessageBox(ghwndFrame, "Can't copy zero byte file", "ADF Opus Error", MB_OK | MB_ICONERROR);
		return;
	}

	bn = adfFileRealSize(fileSize, LOGICAL_BLOCK_SIZE, NULL, NULL);
	if (adfCountFreeBlocks(destVol) < bn) {
		MessageBox(ghwndFrame, "Could not copy file. There is insufficient "
			"free space on the destination volume.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* open source file */
	srcFile = adfOpenFile(srcVol, fileName, "r");

	if (! srcFile) {
		MessageBox(ghwndFrame, "Error opening source file (probably"
			" a bug).", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* open dest file */
	destFile = adfOpenFile(destVol, fileName, "w");

	if (! destFile) {
		adfCloseFile(srcFile);
		MessageBox(ghwndFrame, "Error opening destination file"
			" (probably a bug).", "Error", MB_OK | MB_ICONERROR);
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
			MessageBox(ghwndFrame, "Error writing destination file (volume full?).",
				"ADF Opus Error", MB_OK | MB_ICONERROR);
		}
		Percent = (100 * bread) / fileSize;
	}

	adfCloseFile(srcFile);
	adfCloseFile(destFile);
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
	case ID_TOOLS_BATCHCONVERTER:
		strcpy(buf, "Batch Converter");
		break;
	case ID_TOOLS_DISK2FDI:
		strcpy(buf, "Disk2FDI");
		break;
	case ID_TOOLS_INSTALL:
		strcpy(buf, "Install Bootblock");
		break;
	case ID_TOOLS_DISPLAYBOOTBLOCK:
		strcpy(buf, "Display Bootblock");
		break;
	case ID_TOOLS_OPTIONS:
		strcpy(buf, "Options");
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

BOOL CopyAmiDir2Ami(char *dirName, struct Volume *src, struct Volume *dest)
{
	struct List *list;
	struct Entry *ent;

	adfCreateDir(dest, dest->curDirPtr, dirName);

	adfChangeDir(src, dirName);
	adfChangeDir(dest, dirName);

	list = adfGetDirEnt(src, src->curDirPtr);

	while (list) {
		ent = (struct Entry *)list->content;
		if (ent->type == ST_DIR) {
			/* it's a dir - recurse into it */
			CopyAmiDir2Ami(ent->name, src, dest);
		} else {
			/* it's a file or a link, just copy it */
			CopyAmi2Ami(ent->name, src, dest, ent->size);
		}
		adfFreeEntry(list->content);
		list = list->next;
	}
	freeList(list);

	adfParentDir(dest);
	adfParentDir(src);

	return TRUE;
}
