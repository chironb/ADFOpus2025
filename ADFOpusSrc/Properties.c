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




/* Chiron 2025: TODO: Remove duplicates. I mean... it's fine... but it's messy! */
#include <windows.h>
#include <shlwapi.h>    // for StrFormatByteSize
#pragma comment(lib, "Shlwapi.lib")
#include "Properties.h"
#include "adf_util.h"
#include "adf_dir.h"

extern HWND	ghwndFrame;
extern HWND	ghwndMDIClient;
extern HWND	ghwndTB;
extern char gstrFileName[MAX_PATH * 2];   // your global ADF path // Chiron 2025: DEBUG: This is for debugging. 

extern RETCODE adfSetEntryDate( struct Volume* vol, SECTNUM parSect, char* name, long newDays, long newMins, long newTicks);
extern void adfTime2AmigaTime(struct DateTime dt, long* day, long* min, long* ticks);







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

		// Fill the date and time field.
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (GetFileAttributesEx(DirPtr->name, GetFileExInfoStandard, &fad))
		{
			// Convert to local SYSTEMTIME
			FILETIME ftLocal;
			FileTimeToLocalFileTime(&fad.ftLastWriteTime, &ftLocal);

			SYSTEMTIME st;
			FileTimeToSystemTime(&ftLocal, &st);

			// Format date as "MMMM d, yyyy" => "September 8, 2025"
			TCHAR dateBuf[64];
			GetDateFormat(
				LOCALE_USER_DEFAULT,
				0,
				&st,
				TEXT("MMMM d, yyyy"),
				dateBuf,
				ARRAYSIZE(dateBuf)
			);

			// Format time as "h:mm:ss tt" => "4:03:23 AM"
			TCHAR timeBuf[64];
			GetTimeFormat(
				LOCALE_USER_DEFAULT,
				0,
				&st,
				TEXT("h:mm:ss tt"),
				timeBuf,
				ARRAYSIZE(timeBuf)
			);

			// Combine and dump into your static control
			TCHAR dateTimeBuf[128];
			wsprintf(dateTimeBuf, TEXT("%s, %s"), dateBuf, timeBuf);
			SetDlgItemText(dlg, IDC_PROPERTIES_DATE_WIN, dateTimeBuf);
		};/*end-if*/

		// 4) New: retrieve & format the file size
		{
			// Combine High/Low dwords into a 64-bit size
			ULONGLONG fileSize = ((ULONGLONG)fad.nFileSizeHigh << 32)
				| fad.nFileSizeLow;

			// Convert to a friendly string: "1.2 MB", "512 bytes", etc.
			TCHAR sizeBuf[64];
			StrFormatByteSize(fileSize, sizeBuf, ARRAYSIZE(sizeBuf)); // TODO: ABOUT THAT WARNING *** 'function': conversion from 'ULONGLONG' to 'DWORD', possible loss of data

			// And dump into your static control
			SetDlgItemText(dlg, IDC_PROPERTIES_SIZE_WIN, sizeBuf);
		}


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
				//WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_PROPERTIES);
				return TRUE;

		}
		break;
	case WM_CLOSE:
		
		EndDialog(dlg, TRUE);
		return TRUE;

	// Context sensitive help.
    case WM_HELP: 
   //     WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", 
			//HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        //WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
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
				/*WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_PROPERTIES);*/
				return TRUE;
		}
		break;
	case WM_CLOSE:
		
		EndDialog(dlg, TRUE);
		return TRUE;

	// Context sensitive help.
    case WM_HELP: 
		// WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", 
		// HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        //WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
        break; 	
	}
	return FALSE;
}






void GetPropertiesAmi(HWND dlg, DIRENTRY* DirPtr)
// Get flags from directory entry and set appropriate boxes.
// Properties in ADFLib are HSPARWED.
{
	int i = 0;

	while (DirPtr->flags[i] != '\0') {
		if      (DirPtr->flags[i] == 'r') SendDlgItemMessage(dlg, IDC_AMI_READABLE,   BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'w') SendDlgItemMessage(dlg, IDC_AMI_WRITABLE,   BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'e') SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'd') SendDlgItemMessage(dlg, IDC_AMI_DELETABLE,  BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 's') SendDlgItemMessage(dlg, IDC_AMI_SCRIPT,     BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'a') SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE,    BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'p') SendDlgItemMessage(dlg, IDC_AMI_PURE,       BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'h') SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT,    BM_SETCHECK, BST_CHECKED, 0l);
		i++;
	}

	struct File* amiFile;

	amiFile = adfOpenFile(ci->vol, DirPtr->name, "r");

	// 1) Write comment to dialog.
	SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)amiFile->fileHdr->comment);

	///////////////////////////////////////////////////////////////////////////////////////
	// DATE STARTS HERE ///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////

	//// Scratch buffers for date/time formatting
	TCHAR        dateBuf[64], timeBuf[64], dateTimeBuf[128];
	SYSTEMTIME   st;

	//// back into FILETIME → local SYSTEMTIME
	//FILETIME    ftTarget; // , ftLocal; // No longer used? 
	SYSTEMTIME  stTarget = { 0 };

	// Chiron 2025: This is ugly. Come clean this up!
	// 2) Convert raw Amiga days/minutes/ticks → local SYSTEMTIME
	{/*begin-chunk*/

		// pull out the raw fields
		LONG days = amiFile->fileHdr->days;   // days since 1978-01-01
		LONG mins = amiFile->fileHdr->mins;   // minutes since midnight
		LONG ticks_ = amiFile->fileHdr->ticks;  // 1/50ths of a second

		// build FILETIME for the base date 1978-01-01 00:00:00 UTC
		SYSTEMTIME stBase = { 0 };
		stBase.wYear = 1978;
		stBase.wMonth = 1;
		stBase.wDay = 1;
		FILETIME ftBase;
		SystemTimeToFileTime(&stBase, &ftBase);

		// promote to 64-bit and add days, mins, ticks in 100 ns units
		ULARGE_INTEGER uli = { .LowPart = ftBase.dwLowDateTime,
								.HighPart = ftBase.dwHighDateTime };
		const ULONGLONG DAY_100NS = 86400ULL * 10000000ULL;
		const ULONGLONG MIN_100NS = 60ULL * 10000000ULL;
		const ULONGLONG TICK_100NS = 10000000ULL / 50ULL;

		uli.QuadPart += (ULONGLONG)days * DAY_100NS;
		uli.QuadPart += (ULONGLONG)mins * MIN_100NS;
		uli.QuadPart += (ULONGLONG)ticks_ * TICK_100NS;

		// back into FILETIME → local SYSTEMTIME
		FILETIME    ftTarget; //  , ftLocal;  // No longer used? 
 		SYSTEMTIME  stTarget = { 0 };

		ftTarget.dwLowDateTime = uli.LowPart;
		ftTarget.dwHighDateTime = uli.HighPart;

		//FileTimeToLocalFileTime(&ftTarget, &ftLocal);
		//FileTimeToSystemTime(&ftLocal, &stTarget);
		FileTimeToSystemTime(&ftTarget, &stTarget);

		// stash it in 'st' for formatting
		st = stTarget;
	};/*end-chunk*/

	// 3) Format exactly as your Properties dialog does
	GetDateFormat(
		LOCALE_USER_DEFAULT,
		0,
		&st,
		TEXT("MMM/dd/yyyy"),
		dateBuf,
		ARRAYSIZE(dateBuf)
	);
	GetTimeFormat(
		LOCALE_USER_DEFAULT,
		0,
		&st,
		TEXT("h:mm:ss tt"),
		timeBuf,
		ARRAYSIZE(timeBuf)
	);

	wsprintf(dateTimeBuf, TEXT("%s %s"), dateBuf, timeBuf);

	// 7) Update the Date and Time Picker controls with stTarget
	// Requires #include <commctrl.h> and InitCommonControlsEx for DTP controls

	SendDlgItemMessageA(
		dlg,
		IDC_PROPERTIES_DATEPICKER_AMI,    // your date-picker control
		DTM_SETSYSTEMTIME,                // message to set a SYSTEMTIME
		GDT_VALID,                        // indicate the time is valid
		(LPARAM)&st                 // pointer to your SYSTEMTIME
	);

	SendDlgItemMessageA(
		dlg,
		IDC_PROPERTIES_TIMEPICKER_AMI,    // your time-picker control
		DTM_SETSYSTEMTIME,
		GDT_VALID,
		(LPARAM)&st
	);

	///////////////////////////////////////////////////////////////////////////////////////
	// DATE FINISH HERE ///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	
	unsigned long bytes = amiFile->fileHdr->byteSize;
	char buf[64];

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
		wsprintfA(buf, "%lu.%lu KB (%lu bytes)", whole, frac, bytes);
	}
	else if (bytes < GB) {
		// 1 MB–1 GB: show MB with one decimal
		unsigned long whole = bytes / MB;
		unsigned long frac = (bytes % MB) * 10 / MB;
		wsprintfA(buf, "%lu.%lu MB (%lu bytes)", whole, frac, bytes);
	}
	else {
		// 1 GB+: show GB with one decimal
		unsigned long whole = bytes / GB;
		unsigned long frac = (bytes % GB) * 10 / GB;
		wsprintfA(buf, "%lu.%lu GB (%lu bytes)", whole, frac, bytes);
	}

	SendDlgItemMessageA(
		dlg,
		IDC_PROPERTIES_SIZE_AMI,
		WM_SETTEXT,
		0,
		(LPARAM)buf
	);
	
    adfCloseFile(amiFile);

}




void SetPropertiesAmi(HWND dlg, DIRENTRY* DirPtr)
{
	long longPropertyFlags = 15;  // Default flags 00001111 - all OFF!
	char new_flags[9];
	char szComment[MAXCMMTLEN + 1];
	int  i = 0;
	int  iNumChars;
	HWND win;

	// 1) Read checkboxes and build flag mask/string
	 if (SendDlgItemMessage(dlg, IDC_AMI_READABLE,   BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_R; new_flags[i++] = 'r'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_WRITABLE,   BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_W; new_flags[i++] = 'w'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_E; new_flags[i++] = 'e'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_DELETABLE,  BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_D; new_flags[i++] = 'd'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_SCRIPT,     BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_S;  new_flags[i++] = 's'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE,    BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_A;  new_flags[i++] = 'a'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_PURE,       BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_P;  new_flags[i++] = 'p'; }
     if (SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT,    BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_H;  new_flags[i++] = 'h'; }
	new_flags[i] = '\0';

	// 2) Apply flags
	adfSetEntryAccess(ci->vol, ci->vol->curDirPtr, DirPtr->name, longPropertyFlags);

	// 3) Update comment if modified
	if (SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, EM_GETMODIFY, 0, 0)) {
		iNumChars = SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, WM_GETTEXTLENGTH, 0, 0);
		GetDlgItemTextA(dlg, IDC_EDIT_COMMENT, szComment, ARRAYSIZE(szComment));
		szComment[iNumChars] = '\0';
		adfSetEntryComment(ci->vol, ci->vol->curDirPtr, DirPtr->name, szComment);
	}

	// Chiron 2025: This is ugly. Come clean this up!
	// 4) Read date & time from the picker controls, convert to Amiga date, and set it
	{/*begin-chunk*/

		// Chiron 2025: This is ugly. Come clean this up!
		// 4) Read date & time from the pickers, pack into DateTime, and call adfTime2AmigaTime
		{/*begin-chunk*/

			SYSTEMTIME stDate = { 0 };
			SYSTEMTIME stTime = { 0 };
			SYSTEMTIME stTarget = { 0 };

			// fetch date and time
			SendDlgItemMessageA(
				dlg,
				IDC_PROPERTIES_DATEPICKER_AMI,
				DTM_GETSYSTEMTIME,
				0,
				(LPARAM)&stDate
			);
			SendDlgItemMessageA(
				dlg,
				IDC_PROPERTIES_TIMEPICKER_AMI,
				DTM_GETSYSTEMTIME,
				0,
				(LPARAM)&stTime
			);

			// merge into one SYSTEMTIME
			stTarget.wYear = stDate.wYear;
			stTarget.wMonth = stDate.wMonth;
			stTarget.wDay = stDate.wDay;
			stTarget.wHour = stTime.wHour;
			stTarget.wMinute = stTime.wMinute;
			stTarget.wSecond = stTime.wSecond;
			stTarget.wMilliseconds = stTime.wMilliseconds;

			// build your struct DateTime (tm_year since 1900, mon 1-12, day 1-31)
			struct DateTime dt;
			dt.year = stTarget.wYear - 1900;
			dt.mon = stTarget.wMonth;
			dt.day = stTarget.wDay;
			dt.hour = stTarget.wHour;
			dt.min = stTarget.wMinute;
			dt.sec = stTarget.wSecond;

			// convert *literally*—no timezone or DST adjustments
			long days, mins, ticks;
			adfTime2AmigaTime(dt, &days, &mins, &ticks);

			// write back into the Amiga header
			adfSetEntryDate(
				ci->vol,
				ci->vol->curDirPtr,
				DirPtr->name,
				days,
				mins,
				ticks
			);
		};/*end-chunk*/

	};/*end-chunk*/

	// 5) Refresh the view
	win = GetParent(dlg);
	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);

}    





//void GetPropertiesWin(HWND dlg, DIRENTRY *DirPtr)
//// Get flags from directory entry and set appropriate boxes.
//{
//	int i = 0;
//
//	while(DirPtr->flags[i] != '\0'){
//		if(DirPtr->flags[i] == 'R')
//			SendDlgItemMessage(dlg, IDC_WIN_READONLY, BM_SETCHECK, BST_CHECKED, 0l);
//		else if(DirPtr->flags[i] == 'A')
//			SendDlgItemMessage(dlg, IDC_WIN_ARCHIVE, BM_SETCHECK, BST_CHECKED, 0l);
//		else if(DirPtr->flags[i] == 'H')
//			SendDlgItemMessage(dlg, IDC_WIN_HIDDEN, BM_SETCHECK, BST_CHECKED, 0l);
//		else if(DirPtr->flags[i] == 'S')
//			SendDlgItemMessage(dlg, IDC_WIN_SYSTEM, BM_SETCHECK, BST_CHECKED, 0l);
//		i++;
//	}
//}
//

//------------------------------------------------------------------------------
// GetPropertiesWin: populate checkboxes and date/time pickers
//------------------------------------------------------------------------------
#include <windows.h>
#include <commctrl.h>    // for DTM_SETSYSTEMTIME

extern CHILDINFO* ci;    // ci->curDir ends with path, ci->content->name is filename

void GetPropertiesWin(HWND dlg, DIRENTRY* DirPtr)
{
	CHAR                         fullPath[MAX_PATH];
	WIN32_FILE_ATTRIBUTE_DATA    fad;
	FILETIME                     ftLocal;
	SYSTEMTIME                   st;

	// 1) Clear all four checkboxes, then set those in DirPtr->flags
	SendDlgItemMessageA(dlg, IDC_WIN_READONLY, BM_SETCHECK, BST_UNCHECKED, 0);
	SendDlgItemMessageA(dlg, IDC_WIN_HIDDEN, BM_SETCHECK, BST_UNCHECKED, 0);
	SendDlgItemMessageA(dlg, IDC_WIN_SYSTEM, BM_SETCHECK, BST_UNCHECKED, 0);
	SendDlgItemMessageA(dlg, IDC_WIN_ARCHIVE, BM_SETCHECK, BST_UNCHECKED, 0);

	for (int i = 0; DirPtr->flags[i] != '\0'; ++i) {
		switch (DirPtr->flags[i]) {
		case 'R':
			SendDlgItemMessageA(dlg, IDC_WIN_READONLY, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 'H':
			SendDlgItemMessageA(dlg, IDC_WIN_HIDDEN, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 'S':
			SendDlgItemMessageA(dlg, IDC_WIN_SYSTEM, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 'A':
			SendDlgItemMessageA(dlg, IDC_WIN_ARCHIVE, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	}

	// 2) Build the full path: ci->curDir + DirPtr->name
	lstrcpyA(fullPath, ci->curDir);
	lstrcatA(fullPath, DirPtr->name);

	// 3) Grab the file’s timestamps
	if (GetFileAttributesExA(fullPath, GetFileExInfoStandard, &fad)) {
		// Creation time → local SYSTEMTIME → pickers
		FileTimeToLocalFileTime(&fad.ftCreationTime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		SendDlgItemMessageA(dlg, IDC_PROPERTIES_DATEPICKER_WIN_CREATED,
			DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
		SendDlgItemMessageA(dlg, IDC_PROPERTIES_TIMEPICKER_WIN_CREATED,
			DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);

		// Modified time
		FileTimeToLocalFileTime(&fad.ftLastWriteTime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		SendDlgItemMessageA(dlg, IDC_PROPERTIES_DATEPICKER_WIN_MODIFIED,
			DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
		SendDlgItemMessageA(dlg, IDC_PROPERTIES_TIMEPICKER_WIN_MODIFIED,
			DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);

		// Access time
		FileTimeToLocalFileTime(&fad.ftLastAccessTime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		SendDlgItemMessageA(dlg, IDC_PROPERTIES_DATEPICKER_WIN_ACCESSED,
			DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
		SendDlgItemMessageA(dlg, IDC_PROPERTIES_TIMEPICKER_WIN_ACCESSED,
			DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
	}
}


//void SetPropertiesWin(HWND dlg, DIRENTRY *DirPtr)
//// Get flags from checkboxes and set appropriate directory entry flags.
//{
//	char	new_flags[5];
//	int		i = 0;
//	char	lpFileName[MAX_PATH];	// Pointer to filename.
//	DWORD	dwFileAttributes = 0;	// Attributes to set.
//	HWND	win;
//
//	// Read the flags and create flag string and attribute dword.
//	if(SendDlgItemMessage(dlg, IDC_WIN_READONLY, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		new_flags[i] = 'R';
//		dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_WIN_HIDDEN, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		new_flags[i] = 'H';
//		dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_WIN_SYSTEM, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
//		new_flags[i] = 'S';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_WIN_ARCHIVE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
//		new_flags[i] = 'A';
//		i++;
//	}
//	new_flags[i] = '\0';
//
//	// Update file properties.
//	// Create file path.
//	strcpy(lpFileName, ci->curDir);
//	strcat(lpFileName, DirPtr->name);
//	// Change file attributes.
//	if(!SetFileAttributes(lpFileName, dwFileAttributes))
//		MessageBox(dlg, "Couldn't update file properties.", "ADF Opus error", MB_OK);
//	
//	// Update MDI window childinfo data and listview.	
//	win = GetParent(dlg);
// 	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);
//}


//------------------------------------------------------------------------------
// SetPropertiesWin: read checkboxes + date/time pickers, apply them,
//                   update DirPtr->flags, and refresh the UI.
//------------------------------------------------------------------------------
#include <windows.h>
#include <commctrl.h>    // for DTM_GETSYSTEMTIME

extern CHILDINFO* ci;    // same as above

void SetPropertiesWin(HWND dlg, DIRENTRY* DirPtr)
{
	CHAR      fullPath[MAX_PATH];
	DWORD     dwFileAttributes = 0;
	char      new_flags[5];
	int       idx = 0;

	// 1) Read checkboxes into new_flags[] and dwFileAttributes mask
	if (SendDlgItemMessageA(dlg, IDC_WIN_READONLY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		new_flags[idx++] = 'R';
		dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
	}
	if (SendDlgItemMessageA(dlg, IDC_WIN_HIDDEN, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		new_flags[idx++] = 'H';
		dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
	}
	if (SendDlgItemMessageA(dlg, IDC_WIN_SYSTEM, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		new_flags[idx++] = 'S';
		dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
	}
	if (SendDlgItemMessageA(dlg, IDC_WIN_ARCHIVE, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		new_flags[idx++] = 'A';
		dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
	}
	new_flags[idx] = '\0';

	// 2) Build the full path
	lstrcpyA(fullPath, ci->curDir);
	lstrcatA(fullPath, DirPtr->name);

	// 3) Apply Windows file attributes
	SetFileAttributesA(fullPath, dwFileAttributes);

	// 4) Read each date+time picker into a SYSTEMTIME, convert to UTC FILETIME
	SYSTEMTIME sd = { 0 }, st = { 0 };
	FILETIME   ftLocal, ftUTC_C, ftUTC_A, ftUTC_M;

	// Creation
	SendDlgItemMessageA(dlg,
		IDC_PROPERTIES_DATEPICKER_WIN_CREATED,
		DTM_GETSYSTEMTIME, 0, (LPARAM)&sd);
	SendDlgItemMessageA(dlg,
		IDC_PROPERTIES_TIMEPICKER_WIN_CREATED,
		DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
	sd.wHour = st.wHour;
	sd.wMinute = st.wMinute;
	sd.wSecond = st.wSecond;
	sd.wMilliseconds = st.wMilliseconds;
	SystemTimeToFileTime(&sd, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, &ftUTC_C);

	// Access
	SendDlgItemMessageA(dlg,
		IDC_PROPERTIES_DATEPICKER_WIN_ACCESSED,
		DTM_GETSYSTEMTIME, 0, (LPARAM)&sd);
	SendDlgItemMessageA(dlg,
		IDC_PROPERTIES_TIMEPICKER_WIN_ACCESSED,
		DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
	sd.wHour = st.wHour;
	sd.wMinute = st.wMinute;
	sd.wSecond = st.wSecond;
	sd.wMilliseconds = st.wMilliseconds;
	SystemTimeToFileTime(&sd, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, &ftUTC_A);

	// Modified
	SendDlgItemMessageA(dlg,
		IDC_PROPERTIES_DATEPICKER_WIN_MODIFIED,
		DTM_GETSYSTEMTIME, 0, (LPARAM)&sd);
	SendDlgItemMessageA(dlg,
		IDC_PROPERTIES_TIMEPICKER_WIN_MODIFIED,
		DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
	sd.wHour = st.wHour;
	sd.wMinute = st.wMinute;
	sd.wSecond = st.wSecond;
	sd.wMilliseconds = st.wMilliseconds;
	SystemTimeToFileTime(&sd, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, &ftUTC_M);

	// 5) Stamp the file (creation, access, modified)
	HANDLE hFile = CreateFileA(
		fullPath,
		FILE_WRITE_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		SetFileTime(hFile, &ftUTC_C, &ftUTC_A, &ftUTC_M);
		CloseHandle(hFile);
	}

	// 6) Save flags back into your DIRENTRY and refresh the view
	lstrcpyA(DirPtr->flags, new_flags);
	SendMessage(GetParent(dlg), WM_COMMAND, ID_VIEW_REFRESH, 0);
}



// Chiron 2025: TODO: I'd like to investigate this. It's from the original codeabase.
// STILL TO DO
// - Investigate "TODO: RDSK autodetection" in adfopus/ChildCommon.c. --- NT4 etc.