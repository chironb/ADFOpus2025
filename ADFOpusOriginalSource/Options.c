/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file Options.c
 *  \brief Options dialogue functions.
 *
 * Options.c - routines to handle the options dialogue
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "Utils.h"
#include "Options.h"
#include "ADFlib.h"
#include "Help\AdfOpusHlp.h"
#include "ShellOpen.h"

/* local prototypes */
void WriteOptions();
void ApplyOptions(HWND);
void SetDefaultOptions();
void OptionsChanged(HWND);

extern HANDLE ghInstance;
extern struct OPTIONS Options;
extern HWND ghwndFrame;
extern struct Env adfEnv; /* this should not be here - Laurent's fault */

LRESULT CALLBACK OptionsProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static DWORD aIds[] = { 
		IDC_ODRIVELIST,	IDH_OPTIONS_WINLIST_DRIVE,
		IDC_OTHISDIR,	IDH_OPTIONS_WINLIST_DIR,	
		IDC_ODIR,		IDH_OPTIONS_DIR,	
		IDC_OLABEL,		IDH_OPTIONS_LABEL,
		IDC_ODELETE,	IDH_OPTIONS_CONFIRM_COMDEL,
		IDC_ODELDIR,	IDH_OPTIONS_CONFIRM_DELDIR,
		IDC_ODIRCACHE,	IDH_OPTIONS_MISC_DIRCACHE,
		IDC_OREGISTER,	IDH_OPTIONS_REGISTER_BUTTON,
		IDC_OOK,		IDH_OPTIONS_OK_BUTTON,
		IDC_OAPPLY,		IDH_OPTIONS_APPLY_BUTTON,
		IDC_OHELP,		IDH_OPTIONS_HELP_BUTTON,
		IDCANCEL,		IDH_OPTIONS_CANCEL_BUTTON,
		0,0 
	}; 	

	
	switch(msg) {
	case WM_INITDIALOG:
		SetDlgItemText(dlg, IDC_OLABEL, Options.defaultLabel);
		SetDlgItemText(dlg, IDC_ODIR, Options.defaultDir);
		SendMessage(GetDlgItem(dlg, IDC_ODIRCACHE), BM_SETCHECK, (Options.useDirCache ? BST_CHECKED : 0), 0l);
		SendMessage(GetDlgItem(dlg, IDC_ODELETE), BM_SETCHECK, (Options.confirmDelete ? BST_CHECKED : 0), 0l);
		SendMessage(GetDlgItem(dlg, IDC_ODELDIR), BM_SETCHECK, (Options.confirmDeleteDirs ? BST_CHECKED : 0), 0l);
		SendMessage(GetDlgItem(dlg, Options.defDriveList ? IDC_ODRIVELIST : IDC_OTHISDIR), BM_SETCHECK, BST_CHECKED, 0l);
		
		EnableWindow(GetDlgItem(dlg, IDC_ODIR), ! Options.defDriveList);
		return TRUE;
	case WM_COMMAND:
		switch((int)LOWORD(wp)) {
		case IDC_ODELETE:
		case IDC_ODELDIR:
		case IDC_ODIRCACHE:
		case IDC_ODIR:
		case IDC_OLABEL:
			OptionsChanged(dlg);
			return TRUE;
		case IDC_ODRIVELIST:
			EnableWindow(GetDlgItem(dlg, IDC_ODIR), FALSE);
			OptionsChanged(dlg);
			return TRUE;
		case IDC_OTHISDIR:
			EnableWindow(GetDlgItem(dlg, IDC_ODIR), TRUE);
			OptionsChanged(dlg);
			return TRUE;
		case IDC_OOK:
			ApplyOptions(dlg);
		case IDCANCEL:
			EndDialog(dlg, TRUE);
			return TRUE;
		case IDC_OAPPLY:
			ApplyOptions(dlg);
			SetDlgItemText(dlg, IDCANCEL, "Close");
			EnableWindow(GetDlgItem(dlg, IDC_OAPPLY), FALSE);
			return TRUE;

		case IDC_OREGISTER:
			RegisterFileTypes();		// Register .adf file types.
			return TRUE;

		case IDC_OHELP:
			// Implement help button.
			WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_OPTIONS);
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
        WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 	

	}
	return FALSE;
}

void SetDefaultOptions()
{
	Options.confirmDelete = TRUE;
	Options.confirmDeleteDirs = TRUE;
	strcpy(Options.defaultDir, "C:\\");
	strcpy(Options.defaultLabel, "ADF Opus Created Me!");
	Options.useDirCache = TRUE;
	Options.defDriveList = FALSE;
}

void ReadOptions()
{
	HKEY key;
	DWORD type, size;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\ADFOpus", 0, KEY_READ, &key)
		== ERROR_SUCCESS) {
		/* read into struct options */
		size = sizeof(Options);
		if (RegQueryValueEx(key, "Settings", 0L, &type, (void *)&Options, &size) != ERROR_SUCCESS) {
			MessageBox(NULL, "An error occured reading settings from the registry. Possibly the"
				" registry has been corrupted or modified. Using default settings.", "ADF Opus - Error", MB_OK |
				MB_ICONERROR);
			SetDefaultOptions();
		}
	} else {
		MessageBox(NULL, "It seems this is the first time you have run ADF Opus. Now is a good time"
			" to remind you that this program comes with ABSOLUTELY NO WARRANTY. See the Readme and"
			" license files for more information.\n\n"
			"I hope you enjoy this program, if you have any comments please email them to me (see"
			" Help | About for contact details.",
			"Welcome to ADF Opus", MB_OK | MB_ICONINFORMATION);
		SetDefaultOptions();
		RegisterFileTypes();		// Register Amiga disk file types.
	}
}

void WriteOptions()
{
	HKEY key;
	DWORD disp;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\ADFOpus", 0, "",
	REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disp) == ERROR_SUCCESS) {
		RegSetValueEx(key, "Settings", 0, REG_BINARY, (void *)&Options, sizeof(Options));
	} else {
		MessageBox(NULL, "An error occured writing user preferences to the registry.", "ADF Opus: Error",
			MB_OK | MB_ICONERROR);
	}
}

void ApplyOptions(HWND dlg)
{
	GetDlgItemText(dlg, IDC_OLABEL, Options.defaultLabel, sizeof(Options.defaultLabel));
	GetDlgItemText(dlg, IDC_ODIR, Options.defaultDir, sizeof(Options.defaultDir));
	if (Options.defaultDir[strlen(Options.defaultDir) - 1] != '\\')
		strcat (Options.defaultDir, "\\");


	Options.useDirCache = (SendMessage(GetDlgItem(dlg, IDC_ODIRCACHE), BM_GETCHECK, 0, 0l) == BST_CHECKED);
	Options.confirmDelete = (SendMessage(GetDlgItem(dlg, IDC_ODELETE), BM_GETCHECK, 0, 0l) == BST_CHECKED);
	Options.confirmDeleteDirs = (SendMessage(GetDlgItem(dlg, IDC_ODELDIR), BM_GETCHECK, 0, 0l) == BST_CHECKED);
	Options.defDriveList = (SendMessage(GetDlgItem(dlg, IDC_ODRIVELIST), BM_GETCHECK, 0, 0l) == BST_CHECKED);

	adfEnv.useDirCache = Options.useDirCache;
}

void OptionsChanged(HWND dlg)
{
	SetDlgItemText(dlg, IDCANCEL, "Cancel");
	EnableWindow(GetDlgItem(dlg, IDC_OAPPLY), TRUE);
}