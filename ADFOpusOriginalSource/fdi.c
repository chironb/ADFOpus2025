/*
 * ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 *
 * This code by Gary Harris.
 */
/*! \file fdi.c
 *  \brief Disk2FDI functions.
 *
 * FDI.c - routines to handle the Disk2FDI dialogue.
 */

    
//    Functions used to implement Disk2FDI functionality.
//


#include "Pch.h"

#include "ADFOpus.h"
#include "ChildCommon.h"
#include "Help\AdfOpusHlp.h"
#include "fdi.h"
#include <direct.h>

extern char gstrFileName[MAX_PATH * 2];
extern HWND	ghwndMDIClient;



LRESULT CALLBACK Disk2FDIProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{

	static DWORD aIds[] = { 
		IDC_EDIT_FILENAME,		IDH_DISK2FDI_FILENAME,
		IDC_CHECK_OPEN,			IDH_DISK2FDI_CHECK_OPEN,
		IDC_RADIO_FDI,			IDH_DISK2FDI_TYPE_FDI,	
		IDC_RADIO_ADF,			IDH_DISK2FDI_TYPE_ADF,	
		IDC_RADIO_ST,			IDH_DISK2FDI_TYPE_ST,
		IDC_RADIO_IMG,			IDH_DISK2FDI_TYPE_IMG,
		IDC_CHECK_USE_B_DRIVE,	IDH_DISK2FDI_USE_B,
		IDC_RADIO_SINGLE_SIDED,	IDH_DISK2FDI_1SIDED,
		IDC_RADIO_DOUBLE_SIDED,	IDH_DISK2FDI_2SIDED,
		IDC_CHECK_TRACKS,		IDH_DISK2FDI_NUM_TRACKS,
		IDC_EDIT_TRACKS,		IDH_DISK2FDI_NUM_TRACKS,
		IDC_CHECK_SECTORS,		IDH_DISK2FDI_NUM_SECTORS,
		IDC_EDIT_SECTORS,		IDH_DISK2FDI_NUM_SECTORS,
		IDSTART,				IDH_DISK2FDI_START_BUTTON,
		IDC_BUTTON_FDI_HELP,	IDH_DISK2FDI_HELP_BUTTON,
		IDCANCEL,				IDH_DISK2FDI_CANCEL_BUTTON,
		0,0 
	}; 	


	switch(msg) {
	case WM_INITDIALOG:

		SendDlgItemMessage(dlg, IDC_RADIO_ADF, BM_SETCHECK, BST_CHECKED, 0l);
		SendDlgItemMessage(dlg, IDC_RADIO_DOUBLE_SIDED, BM_SETCHECK, BST_CHECKED, 0l);
		
		// Set the spin control ranges.
		SendDlgItemMessage(dlg, IDC_SPIN_TRACKS, UDM_SETRANGE, 0L, MAKELONG(87, 1));	// 1 - 87 tracks.
		SendDlgItemMessage(dlg, IDC_SPIN_SECTORS, UDM_SETRANGE, 0L, MAKELONG(64, 1));	// 1 - 64 sectors.
		// Set the spin control default values.
		SendDlgItemMessage(dlg, IDC_SPIN_TRACKS, UDM_SETPOS, 0L, MAKELONG((SHORT)80, 0));	// Default to 80 tracks.
		SendDlgItemMessage(dlg, IDC_SPIN_SECTORS, UDM_SETPOS, 0L, MAKELONG((SHORT)22, 0));	// Default to 22 sectors.

		return TRUE;
	case WM_COMMAND:
		switch((int)LOWORD(wp)) {

			case IDC_RADIO_FDI:
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_SECTORS), FALSE);
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_TRACKS), TRUE);
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_OPEN), FALSE);		// Disable opening in Opus.
				return TRUE;

			case IDC_RADIO_ADF:
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_SECTORS), FALSE);
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_TRACKS), FALSE);
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_OPEN), TRUE);		// Enable opening in Opus.
				return TRUE;
			
			case IDC_RADIO_ST:
			case IDC_RADIO_IMG:
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_SECTORS), TRUE);
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_TRACKS), TRUE);
				EnableWindow(GetDlgItem(dlg, IDC_CHECK_OPEN), FALSE);		// Disable opening in Opus.
				return TRUE;
		
			case IDC_CHECK_TRACKS:
				if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
					EnableWindow(GetDlgItem(dlg, IDC_EDIT_TRACKS), TRUE);
					EnableWindow(GetDlgItem(dlg, IDC_SPIN_TRACKS), TRUE);
				}
				else{
					EnableWindow(GetDlgItem(dlg, IDC_EDIT_TRACKS), FALSE);
					EnableWindow(GetDlgItem(dlg, IDC_SPIN_TRACKS), FALSE);
				}
				return TRUE;

			case IDC_CHECK_SECTORS:
				if(SendDlgItemMessage(dlg, IDC_CHECK_SECTORS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
					EnableWindow(GetDlgItem(dlg, IDC_EDIT_SECTORS), TRUE);
					EnableWindow(GetDlgItem(dlg, IDC_SPIN_SECTORS), TRUE);
				}
				else{
					EnableWindow(GetDlgItem(dlg, IDC_EDIT_SECTORS), FALSE);
					EnableWindow(GetDlgItem(dlg, IDC_SPIN_SECTORS), FALSE);
				}
				return TRUE;

			case IDSTART:
				RunDisk2FDI(dlg);
				return TRUE;

			case IDCANCEL:
				EndDialog(dlg, TRUE);
				return TRUE;

			case IDC_BUTTON_FDI_HELP:
				// Implement help button.
				WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_DISK2FDI);
				return TRUE;

		}
		break;

	case WM_CLOSE:
		EndDialog(dlg, TRUE);
		return TRUE;

	// Context sensitive help.
    case WM_HELP: 
       WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
        break; 	

	}
	return FALSE;
}


void RunDisk2FDI(HWND dlg)
/// Creates a string containing the command line arguments and runs Disk2FDI with these arguments.
/// <BR>Input:  A handle to the calling dialogue.
/// <BR>Output: 
{
	char	*szCommand = "DISK2FDI.COM";			// Command.
	char	*szTracksArg = "/T";
	char	*szHeadsArg = "/H";
	char	*szSectorArg = "/S";
	char	*szDriveArg = "B:";
		
	char	szFileName[MAX_PATH];					// Dump name.
	char	szCommandLine[MAX_PATH + 26];			// Final command line - max name length + 26 for args.
	char	szCommandLineArgs[MAX_PATH + 26];		// Final command line - max name length + 26 for args.
	char	szNumTracks[3];
	char	szNumSectors[3];
	char	fileName[MAX_PATH];
	char	drive[MAX_PATH];
	BOOL	bIsADF = FALSE;
	STARTUPINFO s_info;
	PROCESS_INFORMATION p_info;
	HANDLE hProc;

	sprintf(szCommandLineArgs, " ");	// Print a space to avoid errors copying an unintialised string later.

	// Number of heads.
	if(SendDlgItemMessage(dlg, IDC_RADIO_SINGLE_SIDED, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
		sprintf(szCommandLineArgs, "%s %s1 ", szCommandLineArgs, szHeadsArg);
	}
	
	// Sector dump.
	if(SendDlgItemMessage(dlg, IDC_RADIO_ADF, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szSectorArg);
		bIsADF = TRUE;
	}
	else if(SendDlgItemMessage(dlg, IDC_RADIO_FDI, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
		if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
			SendDlgItemMessage(dlg, IDC_EDIT_TRACKS, WM_GETTEXT, 3, (LPARAM)szNumTracks);     // check for 2 digits????????
			sprintf(szCommandLineArgs, "%s %s%s ", szCommandLineArgs, szTracksArg, szNumTracks);
		}
	}
	else if(SendDlgItemMessage(dlg, IDC_RADIO_IMG, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
		if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
			SendDlgItemMessage(dlg, IDC_EDIT_TRACKS, WM_GETTEXT, 3, (LPARAM)szNumTracks);
			sprintf(szCommandLineArgs, "%s %s%s ", szCommandLineArgs, szTracksArg, szNumTracks);
		}
		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szSectorArg);
		if(SendDlgItemMessage(dlg, IDC_CHECK_SECTORS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
			SendDlgItemMessage(dlg, IDC_EDIT_SECTORS, WM_GETTEXT, 3, (LPARAM)szNumSectors);
			sprintf(szCommandLineArgs, "%s%s ", szCommandLineArgs, szNumSectors);
		}
	}
	else if(SendDlgItemMessage(dlg, IDC_RADIO_ST, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
		if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
			SendDlgItemMessage(dlg, IDC_EDIT_TRACKS, WM_GETTEXT, 3, (LPARAM)szNumTracks);
			sprintf(szCommandLineArgs, "%s %s%s ", szCommandLineArgs, szTracksArg, szNumTracks);
		}
		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szSectorArg);
		if(SendDlgItemMessage(dlg, IDC_CHECK_SECTORS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
			SendDlgItemMessage(dlg, IDC_EDIT_SECTORS, WM_GETTEXT, 3, (LPARAM)szNumSectors);
			sprintf(szCommandLineArgs, "%s%s ", szCommandLineArgs, szNumSectors);
		}

	}

	// Number of heads.
	if(SendDlgItemMessage(dlg, IDC_CHECK_USE_B_DRIVE, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szDriveArg);
	}
	
	SendDlgItemMessage(dlg, IDC_EDIT_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM)szFileName);	// Get filename.
	sprintf(szCommandLineArgs, "%s %s", szCommandLineArgs, szFileName);

	_splitpath(szFileName, drive, NULL, fileName, NULL);	// Get filename.
	if(strcmp(drive, "") == 0){
		_getcwd(drive, MAX_PATH);
		strcat(drive, "\\");
		strcat(fileName, ".adf");
		strcat(drive, fileName);
		strcpy(gstrFileName, drive);
	}

	sprintf(szCommandLine, "%s%s", szCommand, szCommandLineArgs);
	memset(&s_info, 0, sizeof(s_info));
	s_info.cb = sizeof(s_info);
	if(!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &s_info, &p_info)){
		MessageBox(dlg,
				   "Disk2FDI was not found in the command path. See help for further details.",
				   "ADF Opus Error",
				   MB_ICONSTOP);
		return;
	}

	// If opening after creation, wait for the process.
	if(SendDlgItemMessage(dlg, IDC_CHECK_OPEN, BM_GETCHECK, 0, 0L) == BST_CHECKED && bIsADF){
		hProc = OpenProcess(SYNCHRONIZE, FALSE, p_info.dwProcessId);
		WaitForSingleObject(hProc, INFINITE);
		CloseHandle(hProc);
		EndDialog(dlg, TRUE);
		CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
	}
	return;
}

