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

#include <windows.h>
#include <shlwapi.h>    // for PathCombineA
#pragma comment(lib, "Shlwapi.lib")

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <commdlg.h>

extern char gstrFileName[MAX_PATH * 2];
extern HWND	ghwndMDIClient;


const char* defaultGreaseweazleFilename = "adfopus_greaseweazle_default.adf";


#include <windows.h>
#include <shobjidl.h>   // for IFileOpenDialog, IID_IFileOpenDialog
#pragma comment(lib, "Ole32.lib")

void OnBrowseFolder(HWND dlg)
{
	HRESULT hr;
	IFileOpenDialog* pDlg = NULL;
	// Initialize COM for this thread (if not already)
	OleInitialize(NULL);

	// Create the FileOpenDialog object
	hr = CoCreateInstance(&CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		&IID_IFileOpenDialog,
		(void**)&pDlg);
	if (SUCCEEDED(hr))
	{
		DWORD opts;
		pDlg->lpVtbl->GetOptions(pDlg, &opts);
		// add the "pick folders" option
		pDlg->lpVtbl->SetOptions(pDlg, opts | FOS_PICKFOLDERS);

		// show dialog
		hr = pDlg->lpVtbl->Show(pDlg, dlg);
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem;
			hr = pDlg->lpVtbl->GetResult(pDlg, &pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR wpath;
				hr = pItem->lpVtbl->GetDisplayName(pItem,
					SIGDN_FILESYSPATH,
					&wpath);
				if (SUCCEEDED(hr))
				{
					// convert wide‐char to ANSI
					char path[MAX_PATH];
					WideCharToMultiByte(
						CP_ACP, 0, wpath, -1,
						path, MAX_PATH, NULL, NULL);

					SetDlgItemTextA(dlg, IDC_GW_EDIT_PATH, path);
					CoTaskMemFree(wpath);
				}
				pItem->lpVtbl->Release(pItem);
			}
		}
		pDlg->lpVtbl->Release(pDlg);
	}

	OleUninitialize();
}


LRESULT CALLBACK GreaseweazleProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	/* Placeholder until I can create the Greaseweazle functionality to replace DISK2FDI. */

	static DWORD aIds[] = {
		IDCANCEL,				IDCANCEL,
		0,0
	};

	switch (msg) {

		case WM_INITDIALOG:


			// optionally pre-populate IDC_GW_EDIT_PATH here
			// I'm thinking like the current working directory of ADF Opus!
			// pre-populate the “Browse…” edit with your default folder
			SetDlgItemTextA(
				dlg,
				IDC_GW_EDIT_PATH,      // your edit control’s ID
				g_defaultLocalPath     // e.g. "C:\\Users\\micro\\…\\"
			);



			// Filename - Set Default Filename for Greaseweazle Imaged Disk
			SetDlgItemText(
				dlg,                   // dialog HWND
				IDC_GW_EDIT_FILENAME,          // your control’s ID
				//TEXT("ADFOpus_Greaseweazle.adf")
				defaultGreaseweazleFilename
			);

		

			// Open in ADF Opus after creation
			CheckDlgButton(
				dlg,                // HWND of the dialog
				IDC_GW_CHECK_OPEN,    // control ID
				BST_CHECKED         // set it checked
			);


			// Radio Pair #1 - Image Type
			CheckRadioButton(dlg,
				IDC_GW_RADIO_ADF,  // first ID in group
				IDC_GW_RADIO_ADZ,  // last ID in group
				IDC_GW_RADIO_ADF   // ID to select
			);

			// Radio Pair #2 - Source Drive
			CheckRadioButton(dlg,
				IDC_GW_RADIO_DRIVE_0,  // first ID in group
				IDC_GW_RADIO_DRIVE_1,  // last ID in group
				IDC_GW_RADIO_DRIVE_0   // ID to select
			);

			// Radio Pair #3 - Disk Density
			CheckRadioButton(dlg,
				IDC_GW_RADIO_DOUBLEDENSITY,  // first ID in group
				IDC_GW_RADIO_HIGHDENSITY,  // last ID in group
				IDC_GW_RADIO_DOUBLEDENSITY   // ID to select
			);

			// Radio Pair #4 - Read / Write Image
			CheckRadioButton(dlg,
				IDC_GW_RADIO_READ_IMAGE,  // first ID in group
				IDC_GW_RADIO_WRITE_IMAGE,  // last ID in group
				IDC_GW_RADIO_READ_IMAGE   // ID to select
			);

			return TRUE;

		break;

		case WM_COMMAND:
			switch ((int)LOWORD(wp)) {


				case IDC_GW_BTN_BROWSE:
					OnBrowseFolder(dlg);
					return TRUE;
				break;


				case IDCANCEL:
					EndDialog(dlg, TRUE);
					return TRUE;
				break;

				case IDGWSTART:

					//MessageBox(
					//	dlg,
					//	TEXT("This feature, launching Greaseweazle from within\n")
					//	TEXT("ADF Opus 2025, is not yet implemented.\n\n")
					//	TEXT("Thanks for your patience!"),
					//	TEXT("Not Implemented"),
					//	MB_OK | MB_ICONINFORMATION
					//);

					//MessageBox(
					//	dlg,
					//	TEXT("We are about to try launching the command line!"),
					//	TEXT("Testing..."),
					//	MB_OK | MB_ICONINFORMATION
					//);

					RunGreaseweazle(dlg);

					EndDialog(dlg, TRUE);
					return TRUE;

				break;

			}/*end-switch*/
		break;
		
		//IDGWSTART


		case WM_CLOSE:
			EndDialog(dlg, TRUE);
			return TRUE;
		break;

	}/*end-switch*/

	return FALSE;

}		


 


/**
 * Escape all backslashes in 'in' by doubling them into 'out'.
 * 'outSize' must be at least (2 * strlen(in) + 1).
 */
static void EscapeBackslashes(
	const char* in,
	char* out,
	size_t      outSize
) {
	size_t i = 0, j = 0;
	while (in[i] != '\0' && j + 1 < outSize)
	{
		if (in[i] == '\\')
		{
			// write two backslashes
			if (j + 2 >= outSize)
				break;
			out[j++] = '\\';
			out[j++] = '\\';
		}
		else
		{
			out[j++] = in[i];
		}
		i++;
	}
	out[j] = '\0';
}














void RunGreaseweazle(HWND dlg)
{
	// 1) Configurable at runtime (load from your options later)

	char greaseweazlePATH[MAX_PATH] = "C:\\Program Files\\Greaseweazle";
	char greaseweazleEXE[MAX_PATH] = "gw.exe";

	// 3) Build the on‐disk path to the Greaseweazle .exe
	CHAR batchFullPath[MAX_PATH];
	if (!PathCombineA(batchFullPath, greaseweazlePATH, greaseweazleEXE))
	{
		MessageBoxA(
			dlg,
			"Failed to combine Greaseweazle path.\n"
			"Check your greaseweazlePATH and greaseweazleEXE settings.",
			"Error",
			MB_OK | MB_ICONERROR
		);

		return;
	}

	// batchFullPath is now wrapped in quotes if it contains spaces
	PathQuoteSpacesA(batchFullPath);

	// 4) Arguments you’ll pass to gw.exe
	// .\gw.exe read --drive=A --format=amiga.amigados example_disk.adf
	// -------- ---- --------- ----------------------- ---------------- 
	char rwArg[32]           = "read";
	char driveArg[32]        = "--drive=A";
	char formatArg[32]       = "--format=amiga.amigados";
	char imagePath[MAX_PATH] = "";
	char imageArg[MAX_PATH]  = "";

	// Drive A Selection
	if (SendDlgItemMessage(dlg, IDC_GW_RADIO_DRIVE_A, BM_GETCHECK, 0L, 0L) == BST_CHECKED) {
		strcpy(driveArg, "--drive=A");
	}

	// Drive B Selection
	if (SendDlgItemMessage(dlg, IDC_GW_RADIO_DRIVE_B, BM_GETCHECK, 0L, 0L) == BST_CHECKED) {
		strcpy(driveArg, "--drive=B");
	}

	// Drive Double Density Selection
	if (SendDlgItemMessage(dlg, IDC_GW_RADIO_DOUBLEDENSITY, BM_GETCHECK, 0L, 0L) == BST_CHECKED) {
		strcpy(formatArg, "--format=amiga.amigados");
	}

	// Drive High Density Selection
	if (SendDlgItemMessage(dlg, IDC_GW_RADIO_HIGHDENSITY, BM_GETCHECK, 0L, 0L) == BST_CHECKED) {
		strcpy(formatArg, "--format=amiga.amigados_hd");
	}

	// Disk Image Path for Greaseweazle
	GetDlgItemTextA(
		dlg,
		IDC_GW_EDIT_PATH,
		imagePath,
		MAX_PATH
	);

	// Disk Image Path for Greaseweazle
	SendDlgItemMessageA(
		dlg,
		IDC_GW_EDIT_FILENAME,  // control ID
		WM_GETTEXT,            // the message to get text
		(WPARAM)(MAX_PATH),    // max chars including NULL
		(LPARAM)imageArg       // your buffer
	);

	// now check imageArg[0] or strlen(imageArg)
	if (imageArg[0] == '\0') {
		strcpy(imageArg, defaultGreaseweazleFilename);
	}

	// This will append “\” only if one isn’t already there.
	// Returns a pointer to the backslash in the new string.
	PathAddBackslashA(imagePath);

	// 5) Build the gw.exe argument string
	// assumes that the end of hte path has a \ backslash
	// .\gw.exe read --drive=A --format=amiga.amigados example_disk.adf
	// -------- ---- --------- ----------------------- ----------------
	// also there needs to be an extra space before the args that get appened after gw.exe
	CHAR gwArgs[256] = "";
	sprintf_s(gwArgs, sizeof gwArgs,
		" %s %s %s \"%s%s\"",
		rwArg,
		driveArg,
		formatArg,
		imagePath,
		imageArg
	);

	// 1) Combine the dir + exe
	//CHAR batchFullPath[MAX_PATH];
	PathCombineA(batchFullPath, greaseweazlePATH, greaseweazleEXE);

	char TESTexePath[MAX_PATH] = "C:\\Program Files\\Greaseweazle\\gw.exe";
	char TESTcmdArgs[256] = " read --drive=A --format=amiga.amigados example_disk.adf";

	// 8) Launch cmd.exe in a new console so you can watch output
	STARTUPINFOA        si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
	HANDLE hProc;

	//if ( !CreateProcessA(batchFullPath, gwArgs, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) ) {
	if ( !CreateProcess(batchFullPath, gwArgs, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) ) {

		MessageBoxA( dlg, "Can't access the Greaseweazle application! Check the path...", "Failed!:", MB_OK | MB_ICONERROR);
		MessageBoxA( dlg, batchFullPath, "Failed! Check batchFullPath:", MB_OK | MB_ICONERROR );
	
	}
	//else
	//{
	//	// close handles if you don't need to wait
	//	CloseHandle(pi.hProcess);
	//	CloseHandle(pi.hThread);
	//}

	//sprintf(szCommandLine, "%s%s", szCommand, szCommandLineArgs);
	//memset(&s_info, 0, sizeof(s_info));
	//s_info.cb = sizeof(s_info);

	//if ( !CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &s_info, &p_info)) {
	//	
	//	MessageBox(dlg,
	//		"Disk2FDI was not found in the command path. See help for further details.",
	//		"ADF Opus Error",
	//		MB_ICONSTOP);
	//	return;
	//}

	// Prep full path and filename so we can open it right away in ADF Opus 2025!
	CHAR imageFullPath[256] = "";
	sprintf_s(imageFullPath, sizeof imageFullPath,
		"%s%s",
		imagePath,
		imageArg
	);

	strcpy(gstrFileName, imageFullPath);

	// If opening after creation, wait for the process.
	if (SendDlgItemMessage(dlg, IDC_GW_CHECK_OPEN, BM_GETCHECK, 0, 0L) == BST_CHECKED) {
		hProc = OpenProcess(SYNCHRONIZE, FALSE, pi.dwProcessId);
		WaitForSingleObject(hProc, INFINITE);
		CloseHandle(hProc);
		EndDialog(dlg, TRUE);
		CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
	}
	return;


}



//MessageBoxA(
//	dlg,
//	greaseweazlePATH,
//	"DEBUG:greaseweazlePATH",
//	MB_OK | MB_ICONERROR
//);






// THIS WORKS EVEN IF TEH PROGRAM IS IN A FOLDER PATH WITH SPACE FUCK YEAH!
//// must be mutable buffers, not literals
//char TESTexePath[MAX_PATH] = "C:\\Program Files\\Greaseweazle\\gw.exe";
//char TESTcmdArgs[256] = " read --drive=A --format=amiga.amigados example_disk.adf";
//// you need the space so the program name is separated from its args
//
//STARTUPINFOA TESTsi = { sizeof(si) };
//PROCESS_INFORMATION TESTpi;
//
//// pass exePath in lpApplicationName, args only in lpCommandLine
//if (!CreateProcessA(
//	TESTexePath,        // launches gw.exe directly
//	TESTcmdArgs,        // its argv[1]…argv[n]
//	NULL, NULL,
//	FALSE,
//	CREATE_NEW_CONSOLE,
//	NULL, NULL,
//	&TESTsi, &TESTpi
//))
//{
//	MessageBoxA(dlg,
//		"Launch failed",
//		"Error", MB_OK | MB_ICONERROR);
//}
//else
//{
//	CloseHandle(TESTpi.hProcess);
//	CloseHandle(TESTpi.hThread);
//}












//LRESULT CALLBACK Disk2FDIProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
//{
//
//	static DWORD aIds[] = { 
//		IDC_EDIT_FILENAME,		IDH_DISK2FDI_FILENAME,
//		IDC_CHECK_OPEN,			IDH_DISK2FDI_CHECK_OPEN,
//		IDC_RADIO_FDI,			IDH_DISK2FDI_TYPE_FDI,	
//		IDC_RADIO_ADF,			IDH_DISK2FDI_TYPE_ADF,	
//		IDC_RADIO_ST,			IDH_DISK2FDI_TYPE_ST,
//		IDC_RADIO_IMG,			IDH_DISK2FDI_TYPE_IMG,
//		IDC_CHECK_USE_B_DRIVE,	IDH_DISK2FDI_USE_B,
//		IDC_RADIO_SINGLE_SIDED,	IDH_DISK2FDI_1SIDED,
//		IDC_RADIO_DOUBLE_SIDED,	IDH_DISK2FDI_2SIDED,
//		IDC_CHECK_TRACKS,		IDH_DISK2FDI_NUM_TRACKS,
//		IDC_EDIT_TRACKS,		IDH_DISK2FDI_NUM_TRACKS,
//		IDC_CHECK_SECTORS,		IDH_DISK2FDI_NUM_SECTORS,
//		IDC_EDIT_SECTORS,		IDH_DISK2FDI_NUM_SECTORS,
//		IDSTART,				IDH_DISK2FDI_START_BUTTON,
//		IDC_BUTTON_FDI_HELP,	IDH_DISK2FDI_HELP_BUTTON,
//		IDCANCEL,				IDH_DISK2FDI_CANCEL_BUTTON,
//		0,0 
//	}; 	
//
//
//	switch(msg) {
//	case WM_INITDIALOG:
//
//		SendDlgItemMessage(dlg, IDC_RADIO_ADF, BM_SETCHECK, BST_CHECKED, 0l);
//		SendDlgItemMessage(dlg, IDC_RADIO_DOUBLE_SIDED, BM_SETCHECK, BST_CHECKED, 0l);
//		
//		// Set the spin control ranges.
//		SendDlgItemMessage(dlg, IDC_SPIN_TRACKS, UDM_SETRANGE, 0L, MAKELONG(87, 1));	// 1 - 87 tracks.
//		SendDlgItemMessage(dlg, IDC_SPIN_SECTORS, UDM_SETRANGE, 0L, MAKELONG(64, 1));	// 1 - 64 sectors.
//		// Set the spin control default values.
//		SendDlgItemMessage(dlg, IDC_SPIN_TRACKS, UDM_SETPOS, 0L, MAKELONG((SHORT)80, 0));	// Default to 80 tracks.
//		SendDlgItemMessage(dlg, IDC_SPIN_SECTORS, UDM_SETPOS, 0L, MAKELONG((SHORT)22, 0));	// Default to 22 sectors.
//
//		return TRUE;
//	case WM_COMMAND:
//		switch((int)LOWORD(wp)) {
//
//			case IDC_RADIO_FDI:
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_SECTORS), FALSE);
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_TRACKS), TRUE);
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_OPEN), FALSE);		// Disable opening in Opus.
//				return TRUE;
//
//			case IDC_RADIO_ADF:
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_SECTORS), FALSE);
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_TRACKS), FALSE);
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_OPEN), TRUE);		// Enable opening in Opus.
//				return TRUE;
//			
//			case IDC_RADIO_ST:
//			case IDC_RADIO_IMG:
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_SECTORS), TRUE);
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_TRACKS), TRUE);
//				EnableWindow(GetDlgItem(dlg, IDC_CHECK_OPEN), FALSE);		// Disable opening in Opus.
//				return TRUE;
//		
//			case IDC_CHECK_TRACKS:
//				if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//					EnableWindow(GetDlgItem(dlg, IDC_EDIT_TRACKS), TRUE);
//					EnableWindow(GetDlgItem(dlg, IDC_SPIN_TRACKS), TRUE);
//				}
//				else{
//					EnableWindow(GetDlgItem(dlg, IDC_EDIT_TRACKS), FALSE);
//					EnableWindow(GetDlgItem(dlg, IDC_SPIN_TRACKS), FALSE);
//				}
//				return TRUE;
//
//			case IDC_CHECK_SECTORS:
//				if(SendDlgItemMessage(dlg, IDC_CHECK_SECTORS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//					EnableWindow(GetDlgItem(dlg, IDC_EDIT_SECTORS), TRUE);
//					EnableWindow(GetDlgItem(dlg, IDC_SPIN_SECTORS), TRUE);
//				}
//				else{
//					EnableWindow(GetDlgItem(dlg, IDC_EDIT_SECTORS), FALSE);
//					EnableWindow(GetDlgItem(dlg, IDC_SPIN_SECTORS), FALSE);
//				}
//				return TRUE;
//
//			case IDSTART:
//				RunDisk2FDI(dlg);
//				return TRUE;
//
//			case IDCANCEL:
//				EndDialog(dlg, TRUE);
//				return TRUE;
//
//			case IDC_BUTTON_FDI_HELP:
//				// Implement help button.
//				WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_DISK2FDI);
//				return TRUE;
//
//		}
//		break;
//
//	case WM_CLOSE:
//		EndDialog(dlg, TRUE);
//		return TRUE;
//
//	// Context sensitive help.
//    case WM_HELP: 
//       WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
//        break; 
// 
//    case WM_CONTEXTMENU: 
//        WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
//        break; 	
//
//	}
//	return FALSE;
//}


//void RunDisk2FDI(HWND dlg)
///// Creates a string containing the command line arguments and runs Disk2FDI with these arguments.
///// <BR>Input:  A handle to the calling dialogue.
///// <BR>Output: 
//{
//	char	*szCommand = "DISK2FDI.COM";			// Command.
//	char	*szTracksArg = "/T";
//	char	*szHeadsArg = "/H";
//	char	*szSectorArg = "/S";
//	char	*szDriveArg = "B:";
//		
//	char	szFileName[MAX_PATH];					// Dump name.
//	char	szCommandLine[MAX_PATH + 26];			// Final command line - max name length + 26 for args.
//	char	szCommandLineArgs[MAX_PATH + 26];		// Final command line - max name length + 26 for args.
//	char	szNumTracks[3];
//	char	szNumSectors[3];
//	char	fileName[MAX_PATH];
//	char	drive[MAX_PATH];
//	BOOL	bIsADF = FALSE;
//	STARTUPINFO s_info;
//	PROCESS_INFORMATION p_info;
//	HANDLE hProc;
//
//	sprintf(szCommandLineArgs, " ");	// Print a space to avoid errors copying an unintialised string later.
//
//	// Number of heads.
//	if(SendDlgItemMessage(dlg, IDC_RADIO_SINGLE_SIDED, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//		sprintf(szCommandLineArgs, "%s %s1 ", szCommandLineArgs, szHeadsArg);
//	}
//	
//	// Sector dump.
//	if(SendDlgItemMessage(dlg, IDC_RADIO_ADF, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szSectorArg);
//		bIsADF = TRUE;
//	}
//	else if(SendDlgItemMessage(dlg, IDC_RADIO_FDI, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//		if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//			SendDlgItemMessage(dlg, IDC_EDIT_TRACKS, WM_GETTEXT, 3, (LPARAM)szNumTracks);     // check for 2 digits????????
//			sprintf(szCommandLineArgs, "%s %s%s ", szCommandLineArgs, szTracksArg, szNumTracks);
//		}
//	}
//	else if(SendDlgItemMessage(dlg, IDC_RADIO_IMG, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//		if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//			SendDlgItemMessage(dlg, IDC_EDIT_TRACKS, WM_GETTEXT, 3, (LPARAM)szNumTracks);
//			sprintf(szCommandLineArgs, "%s %s%s ", szCommandLineArgs, szTracksArg, szNumTracks);
//		}
//		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szSectorArg);
//		if(SendDlgItemMessage(dlg, IDC_CHECK_SECTORS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//			SendDlgItemMessage(dlg, IDC_EDIT_SECTORS, WM_GETTEXT, 3, (LPARAM)szNumSectors);
//			sprintf(szCommandLineArgs, "%s%s ", szCommandLineArgs, szNumSectors);
//		}
//	}
//	else if(SendDlgItemMessage(dlg, IDC_RADIO_ST, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//		if(SendDlgItemMessage(dlg, IDC_CHECK_TRACKS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//			SendDlgItemMessage(dlg, IDC_EDIT_TRACKS, WM_GETTEXT, 3, (LPARAM)szNumTracks);
//			sprintf(szCommandLineArgs, "%s %s%s ", szCommandLineArgs, szTracksArg, szNumTracks);
//		}
//		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szSectorArg);
//		if(SendDlgItemMessage(dlg, IDC_CHECK_SECTORS, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//			SendDlgItemMessage(dlg, IDC_EDIT_SECTORS, WM_GETTEXT, 3, (LPARAM)szNumSectors);
//			sprintf(szCommandLineArgs, "%s%s ", szCommandLineArgs, szNumSectors);
//		}
//
//	}
//
//	// Number of heads.
//	if(SendDlgItemMessage(dlg, IDC_CHECK_USE_B_DRIVE, BM_GETCHECK, 0L, 0L) == BST_CHECKED ){
//		sprintf(szCommandLineArgs, "%s %s ", szCommandLineArgs, szDriveArg);
//	}
//	
//	SendDlgItemMessage(dlg, IDC_EDIT_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM)szFileName);	// Get filename.
//	sprintf(szCommandLineArgs, "%s %s", szCommandLineArgs, szFileName);
//
//	_splitpath(szFileName, drive, NULL, fileName, NULL);	// Get filename.
//	if(strcmp(drive, "") == 0){
//		(void)_getcwd(drive, MAX_PATH);
//		strcat(drive, "\\");
//		strcat(fileName, ".adf");
//		strcat(drive, fileName);
//		strcpy(gstrFileName, drive);
//	}
//
//	sprintf(szCommandLine, "%s%s", szCommand, szCommandLineArgs);
//	memset(&s_info, 0, sizeof(s_info));
//	s_info.cb = sizeof(s_info);
//	if(!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &s_info, &p_info)){
//		MessageBox(dlg,
//				   "Disk2FDI was not found in the command path. See help for further details.",
//				   "ADF Opus Error",
//				   MB_ICONSTOP);
//		return;
//	}
//
//	// If opening after creation, wait for the process.
//	if(SendDlgItemMessage(dlg, IDC_CHECK_OPEN, BM_GETCHECK, 0, 0L) == BST_CHECKED && bIsADF){
//		hProc = OpenProcess(SYNCHRONIZE, FALSE, p_info.dwProcessId);
//		WaitForSingleObject(hProc, INFINITE);
//		CloseHandle(hProc);
//		EndDialog(dlg, TRUE);
//		CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
//	}
//	return;
//}