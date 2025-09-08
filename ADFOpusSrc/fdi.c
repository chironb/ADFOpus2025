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
 * 
 * Chiron 2025 - The DISK2FDI functionality is being replaced with Greaseweazle functionality.
 * Feeling cute. Since FDI could mean Floppy Disk Interface,
*  I'm leaving the file name as fdi.c for now. Might change later
 * 
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

#include "ADFOpus.h"   // for 



#include "ChildCommon.h"   // for CHILDINFO



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

					

					if (RunGreaseweazle(dlg)) {
						EndDialog(dlg, TRUE); // We ran it just fine, the user didn't try to overwrite an existing file. 
						return TRUE;
					} else {
						// Stay in dialog, because the user tried to overwrite an existing file.
						return TRUE;
					}

					//EndDialog(dlg, TRUE);
					//return TRUE;

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








#include <windows.h>
#include <shlwapi.h>     // PathFindExtensionA
#include <commctrl.h>    // combo & MDI messages
#pragma comment(lib, "Shlwapi.lib")

extern HWND   ghwndMDIClient;    // your MDI client
extern char   gstrFileName[];

#include <shlwapi.h>     // for PathFindExtensionA
#pragma comment(lib, "Shlwapi.lib")

#include <windows.h>
#include <shlwapi.h>     // for PathFindExtensionA, PathCombineA, PathQuoteSpacesA
#pragma comment(lib, "Shlwapi.lib")

extern HWND ghwndMDIClient;     // your MDI client handle
extern char gstrFileName[];     // buffer holding the current ADF path











//-----------------------------------------------------------------------------
// callback for EnumChildWindows: adds every Amiga-ADFLister title to the combo
static BOOL CALLBACK _AddAmigaChildren(HWND hwndChild, LPARAM lParam)
{
	// 1) filter by your Amiga-list window class
	char cls[64];
	GetClassNameA(hwndChild, cls, sizeof(cls));
	if (strcmp(cls, "ADFOpusAmigaFileList") != 0)
		return TRUE;   // not an ADF lister window, skip it

	// 2) pull the title (full path) out
	char title[MAX_PATH] = { 0 };
	if (GetWindowTextA(hwndChild, title, MAX_PATH) > 0)
	{
		// 3) only .adf images
		const char* ext = PathFindExtensionA(title);
		if (ext && _stricmp(ext, ".adf") == 0)
		{
			HWND hCombo = (HWND)lParam;
			SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)title);
		}
	}
	return TRUE;  // continue enumeration
}




#include <windows.h>

// params we’ll pass as lParam to EnumChildWindows
typedef struct {
	HWND        dlg;       // dialog you want to update
	const char* path;      // full path to match
} ShowDensityParams;


//-----------------------------------------------------------------------------
// walks your MDI children, finds the one whose title == params.path,
// pops a MessageBox *and* updates IDC_GW_SHOWDENSITY in params.dlg
static BOOL CALLBACK _ShowAmigaDensityFromPath(HWND hwndChild, LPARAM lParam)
{
	ShowDensityParams* p = (ShowDensityParams*)lParam;

	// 1) only your ADF‐lister windows
	char cls[64];
	GetClassNameA(hwndChild, cls, sizeof(cls));
	if (strcmp(cls, "ADFOpusAmigaFileList") != 0)
		return TRUE;

	// 2) get the window’s title (full path)
	char title[MAX_PATH] = { 0 };
	if (GetWindowTextA(hwndChild, title, MAX_PATH) <= 0)
		return TRUE;

	// 3) skip until it matches the path we passed in
	if (strcmp(title, p->path) != 0)
		return TRUE;

	// 4) pull out your CHILDINFO* (stored at offset 0)
	CHILDINFO* ci = (CHILDINFO*)GetWindowLong(hwndChild, 0);
	if (!ci || !ci->dev)
		return FALSE;   // stop, but nothing to show

	// 5) map devType → human string
	char tempStr[64];
	switch (ci->dev->devType)
	{
	case DEVTYPE_FLOPDD:
		strcpy_s(tempStr, sizeof(tempStr),
			"Double Density Floppy (880KB ADF)");
		break;
	case DEVTYPE_FLOPHD:
		strcpy_s(tempStr, sizeof(tempStr),
			"High Density Floppy (1760KB ADF)");
		break;
	case DEVTYPE_HARDDISK:
		strcpy_s(tempStr, sizeof(tempStr),
			"Hard disk dump");
		break;
	case DEVTYPE_HARDFILE:
		strcpy_s(tempStr, sizeof(tempStr),
			"Hardfile");
		break;
	default:
		strcpy_s(tempStr, sizeof(tempStr),
			"Unknown density");
		break;
	}



	// 7) also update the static control in your dialog
	SetDlgItemTextA(
		p->dlg,
		IDC_GW_SHOWDENSITY,
		tempStr);

	// found it—stop enumeration
	return FALSE;
}

//-----------------------------------------------------------------------------
// Greaseweazle “Write” dialog proc
LRESULT CALLBACK GreaseweazleProcWrite(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{

	// Check for the existence of the file to write and warn if the user might overwrite it. 


	switch (msg)
	{
	case WM_INITDIALOG:
	{
		HWND hCombo = GetDlgItem(dlg, IDC_GW_COMBO_FILETOWRITE);
		// clear previous entries
		SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

		// walk every direct child of the MDI client
		EnumChildWindows(
			ghwndMDIClient,
			_AddAmigaChildren,
			(LPARAM)hCombo
		);


		// default‐select the first item, if any
		if (SendMessage(hCombo, CB_GETCOUNT, 0, 0) > 0)
			SendMessage(hCombo, CB_SETCURSEL, 0, 0);



		// 1) pull the selected path out of the combo
		char picked[MAX_PATH] = { 0 };
		// HWND hCombo = GetDlgItem(dlg, IDC_GW_COMBO_FILETOWRITE);
		int  sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
		if (sel != CB_ERR)
			SendMessageA(
				hCombo,
				CB_GETLBTEXT,
				(WPARAM)sel,
				(LPARAM)picked);

		// 2) build our param struct
		ShowDensityParams params = { dlg, picked };

		// 3) enumerate the MDI children—only the matching one will fire
		EnumChildWindows(
			ghwndMDIClient,
			_ShowAmigaDensityFromPath,
			(LPARAM)&params);



		// init Drive‐A/B radios
		CheckRadioButton(
			dlg,
			IDC_GW_RADIO_WRITE_DRIVE_A,
			IDC_GW_RADIO_WRITE_DRIVE_B,
			IDC_GW_RADIO_WRITE_DRIVE_A
		);

		return TRUE;
	}

	case WM_COMMAND:
	{
		WORD    id = LOWORD(wp);
		WORD    ev = HIWORD(wp);

		// 1) Catch the user changing the combo selection
		if (id == IDC_GW_COMBO_FILETOWRITE && ev == CBN_SELCHANGE)
		{
			// 2) Pull the newly selected path out of the combo
			char picked[MAX_PATH] = { 0 };
			HWND hCombo = GetDlgItem(dlg, IDC_GW_COMBO_FILETOWRITE);
			int  sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR)
			{
				SendMessageA(
					hCombo,
					CB_GETLBTEXT,
					(WPARAM)sel,
					(LPARAM)picked);

				// 3) Build the params and re‐run the EnumChildWindows lookup
				ShowDensityParams params = { dlg, picked };
				EnumChildWindows(
					ghwndMDIClient,
					_ShowAmigaDensityFromPath,
					(LPARAM)&params);
			}

			return TRUE;   // handled
		}

		// 4) Your existing Start/Cancel handling…
		switch (id)
		{
		case ID_GW_WRITE_CANCEL:
			EndDialog(dlg, FALSE);
			return TRUE;

		case ID_GW_WRITE_START:
			RunGreaseweazleWrite(dlg);
			EndDialog(dlg, TRUE);
			return TRUE;
		}
	}
	break;



	case WM_CLOSE:
		EndDialog(dlg, TRUE);
		return TRUE;
	}

	return FALSE;
}






//-----------------------------------------------------------------------------
// Builds the gw.exe command line and launches it
// This is what runs to write the ADF to a floppy drive.
void RunGreaseweazleWrite(HWND dlg)
{
	HWND hCombo = GetDlgItem(dlg, IDC_GW_COMBO_FILETOWRITE);
	int  idx = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
	if (idx == CB_ERR)
	{
		MessageBoxA(
			dlg,
			"Please select an Amiga disk image (.adf) to write.",
			"No File Selected",
			MB_OK | MB_ICONWARNING
		);
		return;
	}

	char picked[MAX_PATH] = { 0 };

	SendMessageA(
		hCombo,
		CB_GETLBTEXT,
		(WPARAM)idx,
		(LPARAM)picked
	);

	// copy to global if needed elsewhere
	strcpy_s(gstrFileName, MAX_PATH, picked);

	// locate gw.exe
	char gwDir[MAX_PATH] = "C:\\Program Files\\Greaseweazle";
	char gwExe[MAX_PATH] = "gw.exe";
	CHAR gwPath[MAX_PATH];
	if (!PathCombineA(gwPath, gwDir, gwExe))
	{
		MessageBoxA(
			dlg,
			"Failed to locate Greaseweazle executable.",
			"Error",
			MB_OK | MB_ICONERROR
		);
		return;
	}
	// PathQuoteSpacesA(gwPath);

	// drive arg
	char driveArg[32] = "--drive=A";
	if (SendDlgItemMessage(
		dlg, IDC_GW_RADIO_WRITE_DRIVE_B,
		BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		strcpy_s(driveArg, sizeof driveArg, "--drive=B");
	}





	char formatArg[32] = { 0 };
	char densityText[64] = { 0 };

	// pull the exact label text out of the dialog
	GetDlgItemTextA(
		dlg,
		IDC_GW_SHOWDENSITY,
		densityText,
		sizeof(densityText)
	);

	// compare for each exact string and set formatArg
	if (strcmp(densityText, "Double Density Floppy (880KB ADF)") == 0) {
		// double‐density selected
		strcpy_s(formatArg, sizeof(formatArg),
			"--format=amiga.amigados");
	}
	else if (strcmp(densityText, "High Density Floppy (1760KB ADF)") == 0) {
		// high‐density selected
		strcpy_s(formatArg, sizeof(formatArg),
			"--format=amiga.amigados_hd");
	}
	else {		
		// If we get here, something’s wrong. This means it's a weird format.
		MessageBoxA(dlg, "Error: Format unrecognized. Can't run Greaseweazle!", "Error:", MB_OK);
		
		EndDialog(dlg, IDABORT);
		return;
	}


	// .\gw.exe write --drive=A --format=amiga.amigados example_disk.adf
	// -------- ----- --------- ----------------------- ---------------- 
	
	// .\gw.exe write --drive=A --format=amiga.amigados_hd example_disk.adf
	// -------- ----- --------- -------------------------- ---------------- 
	
	// assemble command-line: write <drive> <format> "<picked>"
	CHAR cmdLine[256];
	sprintf_s(
		cmdLine, sizeof cmdLine,
		" write %s %s \"%s\"",
		driveArg, formatArg, picked
	);

	

	STARTUPINFOA        si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (!CreateProcess(
		gwPath, cmdLine,
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi))
	{

		MessageBoxA(dlg, "Can't access the Greaseweazle application! Check the path...", "Failed!:", MB_OK | MB_ICONERROR);
		MessageBoxA(dlg, gwPath, "Failed! Check gwPath:", MB_OK | MB_ICONERROR);

	}

	HANDLE hProc;
	hProc = OpenProcess(SYNCHRONIZE, FALSE, pi.dwProcessId);
	WaitForSingleObject(hProc, INFINITE);
	CloseHandle(hProc);
	EndDialog(dlg, TRUE);


	return;




}







// This is what probably should be called RunGreaseweazleRead 
// This is what runs to read the ADF to a new disk image .adf file.
BOOL RunGreaseweazle(HWND dlg)
{

	// Check if the file exists. If it does, warn the user and do not proceed.
	// That means we return FALSE meaning we did NOT run Greaseweazle 
	// and therefore the dialog should remain open for the user to change the filename.
	char test_if_exists_path[MAX_PATH] = { 0 };
	char imagePath[MAX_PATH] = "";
	char imageArg[MAX_PATH] = "";

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

	// Combine the path and filename into one string.
	PathCombineA(test_if_exists_path, imagePath, imageArg);

	// There is where I want to check is the file exists.
	if (PathFileExistsA(test_if_exists_path)) {
		MessageBoxA(dlg, "File already exists! Please choose another filename.", "Error", MB_OK | MB_ICONERROR);
		return FALSE; // We do NOT run Greaseweazle, so keep the dialog open.
	}

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

		return FALSE;
	}

	// batchFullPath is now wrapped in quotes if it contains spaces
	PathQuoteSpacesA(batchFullPath);

	// 4) Arguments you’ll pass to gw.exe
	// .\gw.exe read --drive=A --format=amiga.amigados example_disk.adf
	// -------- ---- --------- ----------------------- ---------------- 
	char rwArg[32]           = "read";
	char driveArg[32]        = "--drive=A";
	char formatArg[32]       = "--format=amiga.amigados";
	//char imagePath[MAX_PATH] = "";
	//char imageArg[MAX_PATH]  = "";

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
	return TRUE;


}


