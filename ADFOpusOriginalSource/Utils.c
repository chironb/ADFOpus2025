/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file Utils.c
 *  \brief Utility functions.
 *
 * Utils.c - general utility functions
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "ADF_raw.h" /* we shouldn't really be using this */
#include "ADF_Util.h" /* or this */
#include "Utils.h"

extern char gstrFileName[MAX_PATH * 2];
extern HWND ghwndFrame;
extern HWND ghwndMDIClient;
extern BOOL gbToolbarVisible;
extern BOOL gbStatusBarVisible;
extern HWND ghwndTB;
extern HWND ghwndSB;
extern BOOL ReadOnly;
extern BOOL	bUndeleting;


void SizeToParent(HWND hwndParent, HWND hwndControl)
/*! resizes a control to fit the client area of its parent window 
*/
{
	RECT rec;

	GetClientRect(hwndParent, &rec);
	MoveWindow(hwndControl, 0, 0, rec.right, rec.bottom, TRUE);
}

BOOL OpenDlg(HWND hwndOwner)
{
	OPENFILENAME ofn;
	BOOL res;

	strcpy(gstrFileName, "");

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndOwner;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "All supported types\0*.adf;*.adz;*.dms;*.hdf;*.dmp\0Amiga Disk Files (*.adf)\0*.adf\0Compressed Disk Files (*.adz)\0*.adz\0Diskmasher Files (*.dms)\0*.dms\0Hard Disk Files (*.hdf)\0*.hdf\0Device Dump Files (*.dmp)\0*.dmp\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.lpstrFile = gstrFileName;
	ofn.nMaxFile = sizeof(gstrFileName);
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Open disk image";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_SHAREAWARE;
	ofn.lpstrDefExt = NULL;

	res = GetOpenFileName(&ofn);
	ReadOnly = (ofn.Flags & OFN_READONLY);
	return res;
}

void SizeToStr(const long Size, char *strBuf)
/* converts a number to a string containing size in KB */
{
	char strTemp[40];
	long lTemp = Size / 1024;

	lTemp = lTemp > 0 ? lTemp : lTemp + 1;
	itoa(lTemp, strTemp, 10);
	AddCommas(strTemp);
	strcat(strTemp, "KB");
	strcpy(strBuf, strTemp);
	return;
}

void AddCommas(char *strBuf)
/* adds commas to a numeric string.  this is old, lame code but it works */
{
	char TempBuf[40] = "";
	int Pos = 0;
	int i;

	if (strlen(strBuf) <= 3) return;

	for (i = strlen(strBuf) - 1 ; i >= 0 ; i--) {
		TempBuf[Pos] = strBuf[i];
		Pos++;
		if (((strlen(strBuf) - i) % 3 == 0) && i > 1) {
			TempBuf[Pos] = ',';
			Pos++;
		}
	}
	TempBuf[Pos] = 0;

	Pos = 0;
	for (i = strlen(TempBuf) - 1 ; i >= 0 ; i--) {
		strBuf[Pos] = TempBuf[i];
		Pos++;
	}
	strBuf[Pos] = 0;
}

void ResizeMDIClientWin()
/* resizes the MDI Client window to fit the frame window, including space for
 * the toolbar and status bar where appropriate */
{
	RECT rFrm, rTB, rSB;
	long lNewHeight;

	/* get dimensions of frame window */
	GetClientRect(ghwndFrame, &rFrm);
	lNewHeight = rFrm.bottom - rFrm.top;

	/* move top down by size of toolbar, if it is visible */
	if (gbToolbarVisible) {
		GetWindowRect(ghwndTB, &rTB);
		rFrm.top += (rTB.bottom - rTB.top);
		lNewHeight -= (rTB.bottom - rTB.top);
	}

	if (gbStatusBarVisible) {
		GetWindowRect(ghwndSB, &rSB);
		lNewHeight -= (rSB.bottom - rSB.top);
	}

	MoveWindow(ghwndMDIClient, rFrm.left, rFrm.top, rFrm.right - rFrm.left, lNewHeight, TRUE);
}


void SetMenuBitmaps(HINSTANCE hInst, HMENU hmenuMain)
// Set the menu item bitmaps.
{
	HBITMAP	bmpMenu;
	HMENU	hsubmenuFile, hsubmenuView, hsubmenuAction, hsubmenuTools, hsubmenuWindow, hsubmenuHelp;
	int		iBitmap;
	
	hsubmenuFile	= GetSubMenu(hmenuMain, 0);
	hsubmenuView	= GetSubMenu(hmenuMain, 2);
	hsubmenuAction	= GetSubMenu(hmenuMain, 3);
	hsubmenuTools	= GetSubMenu(hmenuMain, 4);
	hsubmenuWindow	= GetSubMenu(hmenuMain, 5);
	hsubmenuHelp	= GetSubMenu(hmenuMain, 6);

	// Load menu bitmaps. Bitmap IDs range from IDB_NEW (206) to IDB_TEXTVIEWER (227).
	// Cycling to IDB_TEXTVIEWER + 1 prevents final bitmap being corrupted when bmpMenu
	// is released.
	for(iBitmap = IDB_NEW;iBitmap <= IDB_TEXTVIEWER + 1;iBitmap++){

		bmpMenu =  (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(iBitmap), IMAGE_BITMAP, 0, 0, 
									  LR_DEFAULTSIZE|LR_DEFAULTCOLOR);//LR_LOADTRANSPARENT);

		switch(iBitmap){
		// File menu.
		case IDB_NEW:
			SetMenuItemBitmaps(hsubmenuFile, 0, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_OPEN:
			SetMenuItemBitmaps(hsubmenuFile, 1, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_CLOSE:
			SetMenuItemBitmaps(hsubmenuFile, 3, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_INFO:
			SetMenuItemBitmaps(hsubmenuFile, 5, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		// View menu.
		case IDB_SHOWUNDELETABLE:
			SetMenuItemBitmaps(hsubmenuView, 1, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		// Action menu.
		case IDB_UPONELEVEL:
			SetMenuItemBitmaps(hsubmenuAction, 0, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_CREATEDIR:
			SetMenuItemBitmaps(hsubmenuAction, 1, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_DELETE:
			SetMenuItemBitmaps(hsubmenuAction, 3, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_UNDELETE:
			SetMenuItemBitmaps(hsubmenuAction, 4, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_RENAME:
			SetMenuItemBitmaps(hsubmenuAction, 5, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_PROPERTIES:
			SetMenuItemBitmaps(hsubmenuAction, 6, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		// Tools menu.
		case IDB_TEXTVIEWER:
			SetMenuItemBitmaps(hsubmenuTools, 0, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_BATCH:
			SetMenuItemBitmaps(hsubmenuTools, 2, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_DISK2FDI:
			SetMenuItemBitmaps(hsubmenuTools, 3, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_INSTALL:
			SetMenuItemBitmaps(hsubmenuTools, 5, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_DISPLAY:
			SetMenuItemBitmaps(hsubmenuTools, 6, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_OPTIONS:
			SetMenuItemBitmaps(hsubmenuTools, 8, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		// Window menu.
		case IDB_CASCADE:
			SetMenuItemBitmaps(hsubmenuWindow, 0, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_TILEHOR:
			SetMenuItemBitmaps(hsubmenuWindow, 1, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_TILEVER:
			SetMenuItemBitmaps(hsubmenuWindow, 2, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		// Help menu.
		case IDB_HELP:
			SetMenuItemBitmaps(hsubmenuHelp, 0, MF_BYPOSITION, bmpMenu, bmpMenu);
			SetMenuItemBitmaps(hsubmenuHelp, 1, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		case IDB_ABOUT:
			SetMenuItemBitmaps(hsubmenuHelp, 3, MF_BYPOSITION, bmpMenu, bmpMenu);
			break;
		}
		
	}
	DeleteObject(bmpMenu);
}


void UpdateMenuItems(HMENU menu)
{
	/* set new state to enable if there are any child windows open,
           otherwise set it to grayed */
	BOOL		isActive = ((HWND)SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, 0 ) != NULL);
	int			iNewState = isActive ? MF_ENABLED : MF_GRAYED;
	int			tempState;
	HWND		ac = (HWND)SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, 0);
	CHILDINFO	*ci = NULL;
	

	if(isActive)
		ci = (CHILDINFO *)GetWindowLong(ac, 0);

	/* file menu */
	EnableMenuItem(menu, ID_FIL_CLOSE, iNewState);
	if(ci == NULL)		// Prevent error if no MDI window open.
		EnableMenuItem(menu, ID_FIL_INFORMATION, MF_GRAYED);
	else if(ci->isAmi)										
		EnableMenuItem(menu, ID_FIL_INFORMATION, MF_ENABLED);
	else if(ci != NULL)
		EnableMenuItem(menu, ID_FIL_INFORMATION, MF_GRAYED);
	
	/* edit menu */
	EnableMenuItem(menu, ID_EDIT_SELECTALL, iNewState);
	EnableMenuItem(menu, ID_EDIT_SELECTNONE, iNewState);
	EnableMenuItem(menu, ID_EDIT_INVERTSELECTION, iNewState);

	/* view menu */
	EnableMenuItem(menu, ID_VIEW_REFRESH, iNewState);
//	EnableMenuItem(menu, ID_VIEW_REFRESHALL, iNewState);
//	EnableMenuItem(menu, ID_VIEW_CLONECURRENTWINDOW, MF_GRAYED);
	if(ci != NULL && ci->isAmi && !bUndeleting)
		EnableMenuItem(hMenu, ID_VIEW_SHOWUNDELETABLEFILES, MF_ENABLED);

	/* action menu */
//	EnableMenuItem(menu, ID_ACTION_UPONELEVEL, iNewState);
	if(isActive)
		EnableMenuItem(menu, ID_ACTION_UPONELEVEL, ci->atRoot == TRUE?MF_GRAYED:MF_ENABLED);
	else
		EnableMenuItem(menu, ID_ACTION_UPONELEVEL, MF_GRAYED);

	// If it's a DMS disable "Rename", "Delete" and "New Directory".
	// Checking for isActive avoids a failed access of unitialised ci->dfDisk.
	if(isActive && ci->dfDisk == DMS){
		tempState = iNewState;
		iNewState = MF_GRAYED;
	}
	
	// If we're in the drives window, disable the Delete, Rename and Make New Directory menu items.
	if(ci == NULL || strcmp(ci->curDir, "") == 0){
		EnableMenuItem(menu, ID_ACTION_DELETE, MF_GRAYED);
		EnableMenuItem(menu, ID_ACTION_RENAME, MF_GRAYED);
		EnableMenuItem(menu, ID_ACTION_NEWDIRECTORY, MF_GRAYED);
	}
	// If not an undeletable file, set as per iNewState.
	else if(!bUndeleting){
		EnableMenuItem(menu, ID_ACTION_NEWDIRECTORY, iNewState);
		EnableMenuItem(menu, ID_ACTION_DELETE, iNewState);
		EnableMenuItem(menu, ID_ACTION_RENAME, iNewState);
	}
	// Restore iNewState state.
	if(isActive && ci->dfDisk == DMS)
		iNewState = tempState;
	
	/* tools menu */
	EnableMenuItem(menu, ID_TOOLS_OPTIONS, MF_ENABLED);
	EnableMenuItem(menu, ID_TOOLS_INSTALL, MF_GRAYED);
	EnableMenuItem(menu, ID_TOOLS_DISPLAYBOOTBLOCK, MF_GRAYED);

	// Check bootblock and enable "Install" menu item if not already bootable.
	if(ci != NULL && ci->isAmi && (ci->vol->bootCode != 1))
		EnableMenuItem(menu, ID_TOOLS_INSTALL, MF_ENABLED);
	// Enable bootblock display if Ami lister.
	if(ci != NULL && ci->isAmi)
		EnableMenuItem(menu, ID_TOOLS_DISPLAYBOOTBLOCK, MF_ENABLED);


	/* window menu */ 
	EnableMenuItem(menu, ID_WIN_CASCADE, iNewState);
	EnableMenuItem(menu, ID_WIN_TILEVERTICAL, iNewState);
	EnableMenuItem(menu, ID_WIN_TILEHORIZONTAL, iNewState);
	EnableMenuItem(menu, ID_WIN_CLOSEALL, iNewState);
	EnableMenuItem(menu, ID_WIN_ARRANGEICONS, iNewState);
}

void UpdateToolbar()
/* wait wait don't tell me...
 */
{
	HWND		ac = (HWND)SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, 0);
	BOOL		isActive = (ac != NULL);
	BOOL		tempActive = FALSE;
	CHILDINFO	*ci = NULL;

	if (isActive)
		ci = (CHILDINFO *)GetWindowLong(ac, 0);

	/* enable/disable buttons that depend on whether a child window is active */	
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_FIL_CLOSE, MAKELONG(isActive, 0));
	
	// If its a DMS disable "Rename", "Delete" and "New Directory".
	// Checking for isActive avoids a failed access of unitialised ci->dfDisk.
	if(isActive && ci->dfDisk == DMS){
		tempActive = isActive;
		isActive = FALSE;
	}

	// If we're in the drives window, disable the Delete, Rename and Make New Directory toolbar items.
	if(ci == NULL || strcmp(ci->curDir, "") == 0){
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(FALSE, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(FALSE, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_NEWDIRECTORY, MAKELONG(FALSE, 0));
	}
	else{
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_RENAME, MAKELONG(isActive, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_DELETE, MAKELONG(isActive, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_NEWDIRECTORY, MAKELONG(isActive, 0));
	}
	// Restore isActive state.
	if(tempActive)
		isActive = tempActive;

	if(isActive) {
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UPONELEVEL, MAKELONG(! ci->atRoot, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_FIL_INFORMATION, MAKELONG(GetWindowLong(ac, GWL_USERDATA) == CHILD_AMILISTER, 0));
	}else{
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UPONELEVEL, MAKELONG(FALSE, 0));
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_FIL_INFORMATION, MAKELONG(isActive, 0));
	}

	// View items.
	if(ci != NULL && ci->isAmi && !bUndeleting)
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_VIEW_SHOWUNDELETABLEFILES, MAKELONG(TRUE, 0));
	else
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_VIEW_SHOWUNDELETABLEFILES, MAKELONG(FALSE, 0));

	// Action items.
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_UNDELETE, MAKELONG(FALSE, 0));

	// Tools items.
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_INSTALL, MAKELONG(FALSE, 0));
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_DISPLAYBOOTBLOCK, MAKELONG(FALSE, 0));
	SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_TEXT_VIEWER, MAKELONG(FALSE, 0));

	// Check bootblock and enable "Install" menu item if not already bootable.
	if(ci != NULL && ci->isAmi)// && (ci->vol->bootCode != 1))
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_INSTALL, MAKELONG(isActive, 0));
	// Enable bootblock display if Ami lister.
	if(ci != NULL && ci->isAmi)
		SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_TOOLS_DISPLAYBOOTBLOCK, MAKELONG(isActive, 0));

}

int CountOFNFiles(char *lst, int siz)
/* counts no of files in OFN filelist lst.  returns 1 if 1 file or no of files + 1
 * if > 1 file
 */
{
	int i, count = 0;

	for (i = 0 ; i < siz ; i++) {
		if (lst[i] == 0) {
			count++;
			if (lst[i + 1] == 0)
				return count;
		}
	}
	return count;
}

void GetOFNFile(char *lst, int no, char *fil)
/* gets the noth file from an OFN filelist (path\0file1\0file2\0....)
 */
{
	int i = 0, count = 0;

	while (count < no) {
		if (lst[i] == 0)
			count++;
		i++;
	}

	strcpy(fil, lst);
	strcat(fil, "\\");
	strcat(fil, lst + i);
}

DIRENTRY *FindCIData(HWND dlg, DIRENTRY *DirPtr)
// Find the correct entry in the CHILDINFO struct.
// The "content" member of CHILDINFO and all "next" sub-members are of type DIRENTRY.
// This includes files. Both are defined in ChildCommon.h.
{
	while(strcmp(DirPtr->name, buf) != 0){
		if(DirPtr->next == NULL){
			MessageBox(dlg, "Couldn't find filename in CHILDINFO struct.", 
					"ADF Opus error", MB_OK);
			return(NULL);
		}
		DirPtr = DirPtr->next;
	}
	return(DirPtr);
}


#ifdef DEBUG_INFO
// Debug info dialogue.
// Activate  DEBUG_INFO in ADFOpus.h.

void adfVolumeInfoWin(HWND hWnd, struct Volume *vol)
// Windows version of adfVolumeInfo().
// Input:  Receives a handle to the window on which to display the dialogue and a pointer
//         to a ADFLib Volume structure.
// Output: Nil. Displays a windows dialogue containing the disk file data.
{
	char	szTemp[50], szAdfInfo[500];		// Info string.
	
	struct	bRootBlock root;
	char	diskName[35];
	int		days,month,year;
	
	if (adfReadRootBlock(vol, vol->rootBlock, &root)!=RC_OK)
		return;
	
	memset(diskName, 0, 35);
	memcpy(diskName, root.diskName, root.nameLen);
	
	sprintf(szAdfInfo, "Name : %-30s\n", vol->volName);
	strcat(szAdfInfo, "Type : ");
	switch(vol->dev->devType) {
		case DEVTYPE_FLOPDD:
			strcat(szAdfInfo, "Floppy Double Density : 880 KBytes\n");
			break;
		case DEVTYPE_FLOPHD:
			strcat(szAdfInfo, "Floppy High Density : 1760 KBytes\n");
			break;
		case DEVTYPE_HARDDISK:
			sprintf(szTemp, "Hard Disk partition : %3.1f KBytes\n", 
				(vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
			strcat(szAdfInfo, szTemp);
			break;
		case DEVTYPE_HARDFILE:
			sprintf(szTemp, "HardFile : %3.1f KBytes\n", 
				(vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
			strcat(szAdfInfo, szTemp);
			break;
		default:
			strcat(szAdfInfo, "Unknown devType!\n");
	}
	strcat(szAdfInfo, "Filesystem : ");
	sprintf(szTemp, "%s ",isFFS(vol->dosType) ? "FFS" : "OFS");
	strcat(szAdfInfo, szTemp);
	if (isINTL(vol->dosType))
		strcat(szAdfInfo, "INTL ");
	if (isDIRCACHE(vol->dosType))
		strcat(szAdfInfo, "DIRCACHE ");
	strcat(szAdfInfo, "\n");

    sprintf(szTemp, "Free blocks = %ld\n", adfCountFreeBlocks(vol));
	strcat(szAdfInfo, szTemp);
	if (vol->readOnly)
		strcat(szAdfInfo, "Read only\n");
    else
		strcat(szAdfInfo, "Read/Write\n");
 	
    /* created */
	adfDays2Date(root.coDays, &year, &month, &days);
	sprintf(szTemp, "created %d/%02d/%02d %ld:%02ld:%02ld\n",days,month,year,
	    root.coMins/60,root.coMins%60,root.coTicks/50);
	strcat(szAdfInfo, szTemp);
	adfDays2Date(root.days, &year, &month, &days);
	sprintf(szTemp, "last access %d/%02d/%02d %ld:%02ld:%02ld,   ",days,month,year,
	    root.mins/60,root.mins%60,root.ticks/50);
	strcat(szAdfInfo, szTemp);
	adfDays2Date(root.cDays, &year, &month, &days);
	sprintf(szTemp, "%d/%02d/%02d %ld:%02ld:%02ld\n",days,month,year,
	    root.cMins/60,root.cMins%60,root.cTicks/50);
	strcat(szAdfInfo, szTemp);

	MessageBox(hWnd, szAdfInfo, "Adf Info", MB_OK);
}

#endif
