/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * This code by Gary Harris.
 *
 */
/*! \file Properties.c
 *  \brief Properties dialogue functions.
 *
 * Properties.c - routines to handle the properties dialogue.
 */

// Code and output for setting access flags with ADFLib.
//
// RETCODE adfSetEntryAccess(struct Volume* vol, SECTNUM parSect, char* name, long newAcc)
//	
//	CODE
//	-----
//  adfSetEntryAccess(vol, vol->curDirPtr, "dir_5u", 0|ACCMASK_A|ACCMASK_E);
//  adfSetEntryAccess(vol, vol->curDirPtr, "file_1a", 0|ACCMASK_P|ACCMASK_W);
//
//  adfSetEntryAccess(vol, vol->curDirPtr, "dir_5u", 0x12 & !ACCMASK_A & !ACCMASK_E);
//  adfSetEntryAccess(vol, vol->curDirPtr, "file_1a", 0x24 & !ACCMASK_P & !ACCMASK_W );
//
//	OUTPUT
//	------
//	(Default flags)
//	dir_5u                          2    885 30/11/2000 18:25:43         ----rwed
//	file_1a                        -3    883 30/11/2000 18:25:43       1 ----rwed
//
//	(After first lines of code)
//	dir_5u                          2    885 30/11/2000 18:25:43         ---arw-d
//	file_1a                        -3    883 30/11/2000 18:25:43       1 --p-r-ed
//
//	(After second lines of code)
//	dir_5u                          2    885 30/11/2000 18:25:43         ----rwed
//	file_1a                        -3    883 30/11/2000 18:25:43       1 ----rwed

// Default flags are ----rwed : HSPA are OFF when bit = 0, RWED are ON when bit = 0 !!!!!!
// Therefore 00000000 = ----rwed

// I should just have to create a bit mask and let the lib deal with this.
// The mask has 1 where a bit is to be flipped. That is:
// 10000001 changes the flags to H---RWE-
// H and D are flipped. Do this with 0|ACCMASK_H|ACCMASK_D (F&!0ACCMASK_H&!ACCMASK_D).


#include "Properties.h"


extern HWND	ghwndFrame;
extern HWND	ghwndMDIClient;
extern HWND	ghwndTB;

LRESULT CALLBACK PropertiesProcWin(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
// Uses ci and buf declared in ChildCommon.h. 
{
	static DWORD aIds[] = { 
		IDC_PROPERTIES_FILENAME_WIN,	IDH_PROPERTIES_FILENAME_WIN,
		IDC_WIN_READONLY,				IDH_PROPERTIES_WIN_READONLY,	
		IDC_WIN_ARCHIVE,				IDH_PROPERTIES_WIN_ARCHIVE,	
		IDC_WIN_HIDDEN,					IDH_PROPERTIES_WIN_HIDDEN,
		IDC_WIN_SYSTEM,					IDH_PROPERTIES_WIN_SYSTEM,
		IDC_PROPERTIES_WIN_OK_BUTTON,	IDH_PROPERTIES_WIN_OK_BUTTON,
		IDC_PROPERTIES_WIN_HELP_BUTTON,	IDH_PROPERTIES_WIN_HELP_BUTTON,
		IDCANCEL,						IDH_PROPERTIES_WIN_CANCEL_BUTTON,
		0,0 
	}; 	

	
	DIRENTRY	*DirPtr = ci->content;
	HMENU		hMenu;

	switch(msg) {
	case WM_INITDIALOG:
		DirPtr = FindCIData(dlg, DirPtr);
		// Write the file name to the static text box.
		SetDlgItemText(dlg, IDC_PROPERTIES_FILENAME_WIN, DirPtr->name);
		
		// Fill the appropriate checkboxes.
		GetPropertiesWin(dlg, DirPtr);
		return TRUE;

	case WM_COMMAND:
		switch((int)LOWORD(wp)) {
		case IDC_PROPERTIES_WIN_OK_BUTTON:
				// Process flag changes.
				DirPtr = FindCIData(dlg, DirPtr);	
				SetPropertiesWin(dlg, DirPtr);

			case IDCANCEL:
				// Disable properties menu item and toolbar button.
				hMenu = GetMenu(ghwndFrame);
				EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
				EndDialog(dlg, TRUE);
				return TRUE;

			case IDC_PROPERTIES_WIN_HELP_BUTTON:
				// Implement help button.
				WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_PROPERTIES);
				return TRUE;

		}
		break;
	case WM_CLOSE:
		
		EndDialog(dlg, TRUE);
		return TRUE;

	// Context sensitive help.
    case WM_HELP: 
        WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", 
			HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
        break; 	

	}
	return FALSE;
}


LRESULT CALLBACK PropertiesProcAmi(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
// Uses ci and buf declared in ChildCommon.h. 
{
	static DWORD aIds[] = {
		IDC_PROPERTIES_FILENAME_AMI,	IDH_PROPERTIES_FILENAME_AMI,
		IDC_AMI_READABLE,				IDH_PROPERTIES_AMI_READABLE,
		IDC_AMI_WRITABLE,				IDH_PROPERTIES_AMI_WRITABLE,
		IDC_AMI_EXECUTABLE,				IDH_PROPERTIES_AMI_EXECUTABLE,
		IDC_AMI_DELETABLE,				IDH_PROPERTIES_AMI_DELETABLE,
		IDC_AMI_SCRIPT,					IDH_PROPERTIES_AMI_SCRIPT,
		IDC_AMI_ARCHIVE,				IDH_PROPERTIES_AMI_ARCHIVE,
		IDC_AMI_PURE,					IDH_PROPERTIES_AMI_PURE,
		IDC_AMI_HOLDBIT,				IDH_PROPERTIES_AMI_HOLDBIT,
		IDC_EDIT_COMMENT,				IDH_PROPERTIES_AMI_COMMENT,
		IDC_PROPERTIES_AMI_OK_BUTTON,	IDH_PROPERTIES_AMI_OK_BUTTON,
		IDC_PROPERTIES_AMI_HELP_BUTTON,	IDH_PROPERTIES_AMI_HELP_BUTTON,
		IDCANCEL,						IDH_PROPERTIES_AMI_CANCEL_BUTTON,
		0,0 
	}; 	

	
	DIRENTRY	*DirPtr = ci->content;
	HMENU		hMenu;

		switch(msg) {
	case WM_INITDIALOG:
		DirPtr = FindCIData(dlg, DirPtr);
		// Write the file name to the static text box.
		SetDlgItemText(dlg, IDC_PROPERTIES_FILENAME_AMI, DirPtr->name);
		
		// Fill the appropriate checkboxes.
		GetPropertiesAmi(dlg, DirPtr);

		return TRUE;

	case WM_COMMAND:
		switch((int)LOWORD(wp)) {
		case IDC_PROPERTIES_AMI_OK_BUTTON:
				// Process flag changes.
				DirPtr = FindCIData(dlg, DirPtr);	
				SetPropertiesAmi(dlg, DirPtr);

			case IDCANCEL:
				// Disable properties menu item and toolbar button.
				hMenu = GetMenu(ghwndFrame);
				EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_GRAYED);
				SendMessage(ghwndTB, TB_ENABLEBUTTON, ID_ACTION_PROPERTIES, MAKELONG(FALSE, 0));
				EndDialog(dlg, TRUE);
				return TRUE;

			case IDC_PROPERTIES_AMI_HELP_BUTTON:
				// Implement help button.
				WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_PROPERTIES);
				return TRUE;

		}
		break;
	case WM_CLOSE:
		
		EndDialog(dlg, TRUE);
		return TRUE;

	// Context sensitive help.
    case WM_HELP: 
        WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", 
			HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
        break; 	

	}
	return FALSE;
}


void GetPropertiesAmi(HWND dlg, DIRENTRY *DirPtr)
// Get flags from directory entry and set appropriate boxes.
// Properties in ADFLib are HSPARWED.
{
	struct File *amiFile;
	int i = 0;

	while(DirPtr->flags[i] != '\0'){
		if(DirPtr->flags[i] == 'R')
			SendDlgItemMessage(dlg, IDC_AMI_READABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'W')
			SendDlgItemMessage(dlg, IDC_AMI_WRITABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'E')
			SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'D')
			SendDlgItemMessage(dlg, IDC_AMI_DELETABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'S')
			SendDlgItemMessage(dlg, IDC_AMI_SCRIPT, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'A')
			SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'P')
			SendDlgItemMessage(dlg, IDC_AMI_PURE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'H')
			SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT, BM_SETCHECK, BST_CHECKED, 0l);
		i++;
	}


	amiFile = adfOpenFile(ci->vol, DirPtr->name, "r");
	SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)amiFile->fileHdr->comment);
    adfCloseFile(amiFile);

}

void GetPropertiesWin(HWND dlg, DIRENTRY *DirPtr)
// Get flags from directory entry and set appropriate boxes.
{
	int i = 0;

	while(DirPtr->flags[i] != '\0'){
		if(DirPtr->flags[i] == 'R')
			SendDlgItemMessage(dlg, IDC_WIN_READONLY, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'A')
			SendDlgItemMessage(dlg, IDC_WIN_ARCHIVE, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'H')
			SendDlgItemMessage(dlg, IDC_WIN_HIDDEN, BM_SETCHECK, BST_CHECKED, 0l);
		else if(DirPtr->flags[i] == 'S')
			SendDlgItemMessage(dlg, IDC_WIN_SYSTEM, BM_SETCHECK, BST_CHECKED, 0l);
		i++;
	}
}


void SetPropertiesAmi(HWND dlg, DIRENTRY *DirPtr)
// Get flags from checkboxes and set appropriate directory entry flags.
// Properties in ADFLib are HSPARWED. 
// Flags are 00000000 for ----RWED. 0 is off for first 4 and on for next 4.
// File property flags. See ADFLib/adf_blk.h
{

	long	longPropertyFlags = 15;	// Default flags 00001111 - all OFF!
	char	new_flags[9];
	char	szComment[MAXCMMTLEN + 1];			// Max comment length (79) defined in adf_blk.h + '\0'.
	int		i = 0;
	int		iNumChars;
	HWND	win;

	// Read the flags and create flag string and long.
	// Set RWED bits to 0 if flag set.
	if(SendDlgItemMessage(dlg, IDC_AMI_READABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags &= ~ACCMASK_R;
		new_flags[i] = 'R';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_AMI_WRITABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags &= ~ACCMASK_W;	// Current flags NAND flag mask.
		new_flags[i] = 'W';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags &= ~ACCMASK_E;
		new_flags[i] = 'E';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_AMI_DELETABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags &= ~ACCMASK_D;
		new_flags[i] = 'D';
		i++;
	}
	// Set RWED bits to 1 if flag set.
	if(SendDlgItemMessage(dlg, IDC_AMI_SCRIPT, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags |= ACCMASK_S;
		new_flags[i] = 'S';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags |= ACCMASK_A;
		new_flags[i] = 'A';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_AMI_PURE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags |= ACCMASK_P;
		new_flags[i] = 'P';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		longPropertyFlags |= ACCMASK_H;
		new_flags[i] = 'H';
		i++;
	}
 	new_flags[i] = '\0';
	
	// Update file properties.
	adfSetEntryAccess(ci->vol, ci->vol->curDirPtr, DirPtr->name, longPropertyFlags);

	// If file comment text has changed, update file comment.
	if(SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, EM_GETMODIFY, 0, 0)){
		iNumChars = SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, WM_GETTEXTLENGTH, 0, 0);
		SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, EM_GETLINE, iNumChars + 1, (LPARAM)szComment);
		szComment[iNumChars]= '\0';
		adfSetEntryComment(ci->vol, ci->vol->curDirPtr, DirPtr->name, szComment);
	}

	// Update MDI window childinfo data and listview.	
	win = GetParent(dlg);
 	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);
//	CheckForNDOS();


}



void SetPropertiesWin(HWND dlg, DIRENTRY *DirPtr)
// Get flags from checkboxes and set appropriate directory entry flags.
{
	char	new_flags[5];
	int		i = 0;
	char	lpFileName[MAX_PATH];	// Pointer to filename.
	DWORD	dwFileAttributes = 0;	// Attributes to set.
	HWND	win;


	// Read the flags and create flag string and attribute dword.
	if(SendDlgItemMessage(dlg, IDC_WIN_READONLY, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		new_flags[i] = 'R';
		dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_WIN_HIDDEN, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		new_flags[i] = 'H';
		dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_WIN_SYSTEM, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
		new_flags[i] = 'S';
		i++;
	}
	if(SendDlgItemMessage(dlg, IDC_WIN_ARCHIVE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
		dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
		new_flags[i] = 'A';
		i++;
	}
	new_flags[i] = '\0';

	// Update file properties.
	// Create file path.
	strcpy(lpFileName, ci->curDir);
	strcat(lpFileName, DirPtr->name);
	// Change file attributes.
	if(!SetFileAttributes(lpFileName, dwFileAttributes))
		MessageBox(dlg, "Couldn't update file properties.", "ADF Opus error", MB_OK);
	
	// Update MDI window childinfo data and listview.	
	win = GetParent(dlg);
 	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);
}



// STILL TO DO
// - Investigate "TODO: RDSK autodetection" in adfopus/ChildCommon.c. --- NT4 etc.

