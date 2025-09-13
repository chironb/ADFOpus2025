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

#include <windows.h>
#include <shlwapi.h>    // for StrFormatByteSize
#pragma comment(lib, "Shlwapi.lib")


#include "Properties.h"


extern HWND	ghwndFrame;
extern HWND	ghwndMDIClient;
extern HWND	ghwndTB;

// Chiron 2025: DEBUG: This is for debugging. 
extern char gstrFileName[MAX_PATH * 2];   // your global ADF path


#include "adf_dir.h"

extern RETCODE adfSetEntryDate(
	struct Volume* vol,
	SECTNUM        parSect,
	char* name,
	long           newDays,
	long           newMins,
	long           newTicks);


#include "adf_util.h"
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
		}

		// 4) New: retrieve & format the file size

		{
			// Combine High/Low dwords into a 64-bit size
			ULONGLONG fileSize = ((ULONGLONG)fad.nFileSizeHigh << 32)
				| fad.nFileSizeLow;

			// Convert to a friendly string: "1.2 MB", "512 bytes", etc.
			TCHAR sizeBuf[64];
			StrFormatByteSize(fileSize, sizeBuf, ARRAYSIZE(sizeBuf));

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
   //     WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", 
			//HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
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
		if (DirPtr->flags[i] == 'R')
			SendDlgItemMessage(dlg, IDC_AMI_READABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'W')
			SendDlgItemMessage(dlg, IDC_AMI_WRITABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'E')
			SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'D')
			SendDlgItemMessage(dlg, IDC_AMI_DELETABLE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'S')
			SendDlgItemMessage(dlg, IDC_AMI_SCRIPT, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'A')
			SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'P')
			SendDlgItemMessage(dlg, IDC_AMI_PURE, BM_SETCHECK, BST_CHECKED, 0l);
		else if (DirPtr->flags[i] == 'H')
			SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT, BM_SETCHECK, BST_CHECKED, 0l);
		i++;
	}


	struct File* amiFile;

	amiFile = adfOpenFile(ci->vol, DirPtr->name, "r");

	// 1) Write comment to dialog.
	SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)amiFile->fileHdr->comment);


	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// DATE STARTS HERE ///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////







	//// 2) Convert Amiga date/time to local Windows date/time and write to dialog.
	//
	//	// 1) Grab the raw Amiga date/time fields
	//	LONG days = amiFile->fileHdr->days;   // days since 1978-01-01
	//	LONG mins = amiFile->fileHdr->mins;   // minutes since midnight
	//	LONG ticks = amiFile->fileHdr->ticks;  // 1/50ths of a second

	//	// 2) Build a FILETIME for the base date 1978-01-01 00:00:00
	//	SYSTEMTIME stBase = { 0 };
	//	stBase.wYear = 1978;
	//	stBase.wMonth = 1;
	//	stBase.wDay = 1;
	//	// wHour, wMinute, wSecond, wMilliseconds all zero

	//	FILETIME ftBase;
	//	SystemTimeToFileTime(&stBase, &ftBase);

	//	// 3) Promote to 64-bit and add days, mins, ticks in 100ns units
	//	ULARGE_INTEGER uli;
	//	uli.LowPart = ftBase.dwLowDateTime;
	//	uli.HighPart = ftBase.dwHighDateTime;

	//	const ULONGLONG DAY_100NS = 86400ULL * 10000000ULL;
	//	const ULONGLONG MIN_100NS = 60ULL * 10000000ULL;
	//	const ULONGLONG TICK_100NS = 10000000ULL / 50ULL;

	//	uli.QuadPart += (ULONGLONG)days * DAY_100NS;
	//	uli.QuadPart += (ULONGLONG)mins * MIN_100NS;
	//	uli.QuadPart += (ULONGLONG)ticks * TICK_100NS;

	//	// 4) Convert back to FILETIME
	//	FILETIME ftTarget;
	//	ftTarget.dwLowDateTime = uli.LowPart;
	//	ftTarget.dwHighDateTime = uli.HighPart;

	//	// 5) Convert to local SYSTEMTIME
	//	FILETIME ftLocal;
	//	SYSTEMTIME stTarget;
	//	// … after you’ve built ftTarget by adding days/mins/ticks to your base …
	//	//
	//	// OLD: this applies DST/TZ and will shift you in or out of summer time
	//	//    FileTimeToLocalFileTime(&ftTarget, &ftLocal);
	//	//    FileTimeToSystemTime  (&ftLocal, &stTarget);
	//	//
	//	// NEW: drop the local‐convert; ftTarget is *your* raw 1978‐based offset,
	//	//      so just turn that 100ns‐value directly into a SYSTEMTIME
	//	FileTimeToSystemTime(&ftTarget, &stTarget);

	//	// 6) Format as "YYYY-MM-DD HH:MM:SS"
	//	char dateBuf[64];
	//	wsprintfA(
	//		dateBuf,
	//		"%04u-%02u-%02u %02u:%02u:%02u",
	//		stTarget.wYear,
	//		stTarget.wMonth,
	//		stTarget.wDay,
	//		stTarget.wHour,
	//		stTarget.wMinute,
	//		stTarget.wSecond
	//	);

	//	// … after you’ve obtained `stTarget` from FileTimeToSystemTime …

	//	
	//		//char dateBuf[64];
	//		char datePart[32];
	//		char timePart[32];

	//		// Format “August 2, 2025”
	//		// pattern “MMMM d, yyyy” → full month, day(no leading 0), 4-digit year
	//		GetDateFormatA(
	//			LOCALE_USER_DEFAULT,
	//			0,
	//			&stTarget,
	//			"MMMM d, yyyy",
	//			datePart,
	//			ARRAYSIZE(datePart)
	//		);

	//		// Format “10:52:18 PM”
	//		// pattern “h:mm:ss tt” → 12-hour, no leading 0 on hour, seconds, AM/PM
	//		GetTimeFormatA(
	//			LOCALE_USER_DEFAULT,
	//			0,
	//			&stTarget,
	//			"h:mm:ss tt",
	//			timePart,
	//			ARRAYSIZE(timePart)
	//		);

	//		// Combine with a comma
	//		wsprintfA(
	//			dateBuf,
	//			"%s, %s",
	//			datePart,
	//			timePart
	//		);


	//		MessageBoxA(NULL, dateBuf, "DEBUG:dateBuf", MB_OK | MB_ICONERROR);

	//		//SendDlgItemMessageA(
	//		//	dlg,
	//		//	IDC_PROPERTIES_DATE_AMI,
	//		//	WM_SETTEXT,
	//		//	0,
	//		//	(LPARAM)dateBuf
	//		//);
		



//DIRENTRY* ce;
////char         strBuf[20];
//int          pos;
//
//BOOL         bAmi = FALSE;
//
//// Scratch buffers for date/time formatting
TCHAR        dateBuf[64], timeBuf[64], dateTimeBuf[128];
SYSTEMTIME   st;
//FILETIME     ftLocal;
//WIN32_FILE_ATTRIBUTE_DATA fad;
//char         fullPath[MAX_PATH];
//
//// back into FILETIME → local SYSTEMTIME
FILETIME    ftTarget, ftLocal;
SYSTEMTIME  stTarget = { 0 };

			// 2) Convert raw Amiga days/minutes/ticks → local SYSTEMTIME
			{
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
				FILETIME    ftTarget, ftLocal;
				SYSTEMTIME  stTarget = { 0 };

				ftTarget.dwLowDateTime = uli.LowPart;
				ftTarget.dwHighDateTime = uli.HighPart;

				//FileTimeToLocalFileTime(&ftTarget, &ftLocal);
				//FileTimeToSystemTime(&ftLocal, &stTarget);
				FileTimeToSystemTime(&ftTarget, &stTarget);


				// stash it in 'st' for formatting
				st = stTarget;
			}

			//adfCloseFile(amiFile);

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

			//// 4) Insert into ListView
			//LVAddSubItem(ci->lv, dateTimeBuf, pos, 3);


			//MessageBoxA(NULL, dateTimeBuf, "DEBUG:dateTimeBuf", MB_OK | MB_ICONERROR);




		// 7) Update the Date and Time Picker controls with stTarget
//    Requires #include <commctrl.h> and InitCommonControlsEx for DTP controls

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





		// DEBUGING AND TESTING - WRITE RAW DATE INFO TO THE DIALOG! 

		//char days_text[255], 
		//	 mins_text[255], 
		//	ticks_text[255];
		//
		//wsprintfA(days_text, "%d", (LPARAM)amiFile->fileHdr->days);
		//wsprintfA(mins_text, "%d", (LPARAM)amiFile->fileHdr->mins);
		//wsprintfA(ticks_text, "%d", (LPARAM)amiFile->fileHdr->ticks);

		//SendDlgItemMessage(dlg, IDC_PROPERTIES_DAYS_AMI, WM_SETTEXT, 0, days_text);
		//SendDlgItemMessage(dlg, IDC_PROPERTIES_MINS_AMI, WM_SETTEXT, 0, mins_text);
		//SendDlgItemMessage(dlg, IDC_PROPERTIES_TICKS_AMI, WM_SETTEXT, 0, ticks_text);





	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// DATE FINISH HERE ///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
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


//void SetPropertiesAmi(HWND dlg, DIRENTRY *DirPtr)
//// Get flags from checkboxes and set appropriate directory entry flags.
//// Properties in ADFLib are HSPARWED. 
//// Flags are 00000000 for ----RWED. 0 is off for first 4 and on for next 4.
//// File property flags. See ADFLib/adf_blk.h
//{
//	long	longPropertyFlags = 15;	// Default flags 00001111 - all OFF!
//	char	new_flags[9];
//	char	szComment[MAXCMMTLEN + 1];			// Max comment length (79) defined in adf_blk.h + '\0'.
//	int		i = 0;
//	int		iNumChars;
//	HWND	win;
//
//	// Read the flags and create flag string and long.
//	// Set RWED bits to 0 if flag set.
//	if(SendDlgItemMessage(dlg, IDC_AMI_READABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags &= ~ACCMASK_R;
//		new_flags[i] = 'R';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_AMI_WRITABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags &= ~ACCMASK_W;	// Current flags NAND flag mask.
//		new_flags[i] = 'W';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags &= ~ACCMASK_E;
//		new_flags[i] = 'E';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_AMI_DELETABLE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags &= ~ACCMASK_D;
//		new_flags[i] = 'D';
//		i++;
//	}
//	// Set RWED bits to 1 if flag set.
//	if(SendDlgItemMessage(dlg, IDC_AMI_SCRIPT, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags |= ACCMASK_S;
//		new_flags[i] = 'S';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags |= ACCMASK_A;
//		new_flags[i] = 'A';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_AMI_PURE, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags |= ACCMASK_P;
//		new_flags[i] = 'P';
//		i++;
//	}
//	if(SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT, BM_GETCHECK, 0l, 0l) == BST_CHECKED){
//		longPropertyFlags |= ACCMASK_H;
//		new_flags[i] = 'H';
//		i++;
//	}
// 	new_flags[i] = '\0';
//	
//	// Update file properties.
//	adfSetEntryAccess(ci->vol, ci->vol->curDirPtr, DirPtr->name, longPropertyFlags);
//
//	// If file comment text has changed, update file comment.
//	if(SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, EM_GETMODIFY, 0, 0)){
//		
//		iNumChars = SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, WM_GETTEXTLENGTH, 0, 0);
//		// SendDlgItemMessage(dlg, IDC_EDIT_COMMENT, EM_GETLINE, iNumChars + 1, (LPARAM)szComment); // This was a bug in the original code I think. 
//		GetDlgItemTextA(dlg, IDC_EDIT_COMMENT, szComment, ARRAYSIZE(szComment));
//		szComment[iNumChars]= '\0';
//
//		// MessageBoxA(dlg, szComment, "DEBUG:szComment", MB_OK | MB_ICONERROR);
//		
//		adfSetEntryComment(ci->vol, ci->vol->curDirPtr, DirPtr->name, szComment);
//	
//	}
//
//	// DEBUGING AND TESTING - WRITE RAW DATE INFO TO THE DIALOG! 
//	char days_text[255],
//		mins_text[255],
//		ticks_text[255];
//
//	long days_long,
//		 mins_long,
//		ticks_long;
//
//	char* end;
//
//	GetDlgItemTextA(dlg, IDC_PROPERTIES_DAYS_AMI,   days_text, ARRAYSIZE(days_text));
//	GetDlgItemTextA(dlg, IDC_PROPERTIES_MINS_AMI,   mins_text, ARRAYSIZE(mins_text));
//	GetDlgItemTextA(dlg, IDC_PROPERTIES_TICKS_AMI, ticks_text, ARRAYSIZE(ticks_text));
//
//	days_long  = strtol(days_text,  &end, 10);  // base 10 for decimal
//	mins_long  = strtol(mins_text,  &end, 10);   
//	ticks_long = strtol(ticks_text, &end, 10);   
//
//	adfSetEntryDate(ci->vol, ci->vol->curDirPtr, DirPtr->name, days_long, mins_long, ticks_long);
//
//	// Update MDI window childinfo data and listview.	
//	win = GetParent(dlg);
// 	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);
//
//}




void SetPropertiesAmi(HWND dlg, DIRENTRY* DirPtr)
{
	long longPropertyFlags = 15;  // Default flags 00001111 - all OFF!
	char new_flags[9];
	char szComment[MAXCMMTLEN + 1];
	int  i = 0;
	int  iNumChars;
	HWND win;

	// 1) Read checkboxes and build flag mask/string
	if (SendDlgItemMessage(dlg, IDC_AMI_READABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_R; new_flags[i++] = 'R'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_WRITABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_W; new_flags[i++] = 'W'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_EXECUTABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_E; new_flags[i++] = 'E'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_DELETABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags &= ~ACCMASK_D; new_flags[i++] = 'D'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_SCRIPT, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_S; new_flags[i++] = 'S'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_ARCHIVE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_A; new_flags[i++] = 'A'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_PURE, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_P; new_flags[i++] = 'P'; }
	if (SendDlgItemMessage(dlg, IDC_AMI_HOLDBIT, BM_GETCHECK, 0, 0) == BST_CHECKED) { longPropertyFlags |= ACCMASK_H; new_flags[i++] = 'H'; }
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

	// 4) Read date & time from the picker controls, convert to Amiga date, and set it
	{
		//SYSTEMTIME stDate = { 0 };
		//SYSTEMTIME stTime = { 0 };
		//SYSTEMTIME stTarget = { 0 };

		//// Fetch date and time separately
		//SendDlgItemMessageA(dlg,
		//	IDC_PROPERTIES_DATEPICKER_AMI,
		//	DTM_GETSYSTEMTIME,
		//	0,
		//	(LPARAM)&stDate);

		//SendDlgItemMessageA(dlg,
		//	IDC_PROPERTIES_TIMEPICKER_AMI,
		//	DTM_GETSYSTEMTIME,
		//	0,
		//	(LPARAM)&stTime);

		//// Combine into one SYSTEMTIME
		//stTarget.wYear = stDate.wYear;
		//stTarget.wMonth = stDate.wMonth;
		//stTarget.wDay = stDate.wDay;
		//stTarget.wHour = stTime.wHour;
		//stTarget.wMinute = stTime.wMinute;
		//stTarget.wSecond = stTime.wSecond;
		//stTarget.wMilliseconds = stTime.wMilliseconds;

		//// Convert local SYSTEMTIME to UTC FILETIME
		//SYSTEMTIME stUTC;
		//TzSpecificLocalTimeToSystemTime(NULL, &stTarget, &stUTC);

		//FILETIME ftUTC;
		//SystemTimeToFileTime(&stUTC, &ftUTC);



		// 4) Read date & time from the pickers, pack into DateTime, and call adfTime2AmigaTime
		{
			SYSTEMTIME stDate   = { 0 };
			SYSTEMTIME stTime   = { 0 };
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
			dt.mon  = stTarget.wMonth;
			dt.day  = stTarget.wDay;
			dt.hour = stTarget.wHour;
			dt.min  = stTarget.wMinute;
			dt.sec  = stTarget.wSecond;





			//TCHAR dateBuf[64], timeBuf[64], dateTimeBuf[128];
			//GetDateFormat( LOCALE_USER_DEFAULT, 0, &stTarget, TEXT("MMM/dd/yyyy"), dateBuf, ARRAYSIZE(dateBuf) );
			//GetTimeFormat( LOCALE_USER_DEFAULT, 0, &stTarget, TEXT("h:mm:ss tt"),  timeBuf, ARRAYSIZE(timeBuf) );
			//wsprintf(dateTimeBuf, TEXT("%s %s"), dateBuf, timeBuf);
			//MessageBoxA(NULL, dateTimeBuf, "DEBUG:dateTimeBuf", MB_OK | MB_ICONERROR);



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
		}

	}

	// 5) Refresh the view
	win = GetParent(dlg);
	SendMessage(win, WM_COMMAND, ID_VIEW_REFRESH, 0l);
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

