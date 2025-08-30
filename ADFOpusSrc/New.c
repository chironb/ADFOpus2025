/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file New.c
 *  \brief New Volume dialogue functions.
 *
 * New.c - routines to handle the New Volume dialogue
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "ADFlib.h"
#include "New.h"
#include "Options.h"
#include "ChildCommon.h"
#include "Help\AdfOpusHlp.h"
#include "Install.h"

extern char gstrFileName[MAX_PATH * 2];
extern HINSTANCE instance;
extern HWND ghwndFrame;
extern HWND ghwndMDIClient;

/* globals */
extern int Done;
HWND NewProgress;
long Size;
extern int Percent;
extern UINT Timer;
extern struct OPTIONS Options;

/* prototypes */
void NewCreate(HWND);
void NewSelectFile(HWND);
LRESULT CALLBACK NewProgressProc(HWND, UINT, WPARAM, LPARAM);
void NewCreateFile(void *);

LRESULT CALLBACK NewDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	char size[6];
	int newPos;

	static DWORD aIds[] = { 
		IDC_NEWPATH,			IDH_CREATE_PATH_EDIT,
		IDC_NEWBROWSE,			IDH_CREATE_PATH_BROWSE,	
		IDC_NEWADF,				IDH_CREATE_TYPE_ADF,	
		IDC_NEWHD,				IDH_CREATE_TYPE_HD,
		IDC_NEWHARDFILE,		IDH_CREATE_TYPE_HARDFILE,
		IDC_NEWPRESET,			IDH_CREATE_TYPE_HARDFILE_PRESET,
		IDC_NEWPRESETSLIDER,	IDH_CREATE_TYPE_HARDFILE_PRESET_SIZES,
		IDC_NEWCUSTOM,			IDH_CREATE_TYPE_HARDFILE_CUSTOM,
		IDC_NEWCUSTOMSIZE,		IDH_CREATE_TYPE_HARDFILE_CUSTOM_SIZE,
		IDC_NEWFFS,				IDH_CREATE_FILESYSTEM_FFS,
		IDC_NEWDIRC,			IDH_CREATE_FILESYSTEM_DC,
		IDC_NEWINTL,			IDH_CREATE_FILESYSTEM_IFS,
		IDC_NEWBOOTABLE,		IDH_CREATE_FILESYSTEM_BOOT,
		IDC_NEWLABEL,			IDH_CREATE_LABEL,
		IDC_NEWOPEN,			IDH_CREATE_OPEN,
		IDC_NEWCREATE,			IDH_CREATE_CREATE_BUTTON,
		IDC_NEWHELP,			IDH_CREATE_HELP_BUTTON,
		IDCANCEL,				IDH_CREATE_CANCEL_BUTTON,
		0,0 
	}; 	

	
	switch(msg) {
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(dlg, IDC_NEWPRESETSLIDER), TBM_SETRANGE, TRUE, MAKELONG(0, 9));
		SendMessage(GetDlgItem(dlg, IDC_NEWADF), BM_SETCHECK, BST_CHECKED, 0l);
		SendMessage(GetDlgItem(dlg, IDC_NEWPRESET), BM_SETCHECK, BST_CHECKED, 0l);
		SendMessage(GetDlgItem(dlg, IDC_NEWPRESETSLIDER), BM_SETCHECK, BST_CHECKED, 0l);
		SendMessage(GetDlgItem(dlg, IDC_NEWPRESETSIZE), BM_SETCHECK, BST_CHECKED, 0l);
		SetDlgItemText(dlg, IDC_NEWPATH, "NEW.ADF");
		SetDlgItemText(dlg, IDC_NEWLABEL, Options.defaultLabel);
		return TRUE;
	case WM_COMMAND:
		switch(wp) {
		case IDC_NEWCREATE:
			NewCreate(dlg);
			return TRUE;
		case IDCANCEL:
			EndDialog(dlg, TRUE);
			return TRUE;
		case IDC_NEWBROWSE:
			NewSelectFile(dlg);
			return TRUE;
		case IDC_NEWADF:
			EnableWindow(GetDlgItem(dlg, IDC_NEWHD), TRUE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESET), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWCUSTOM), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWCUSTOMSIZE), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESETSIZE), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESETSLIDER), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWBOOTABLE), TRUE);
			return TRUE;
		case IDC_NEWHARDFILE:
			EnableWindow(GetDlgItem(dlg, IDC_NEWHD), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESET), TRUE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWCUSTOM), TRUE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWBOOTABLE), FALSE);
			if (SendMessage(GetDlgItem(dlg, IDC_NEWPRESET), BM_GETCHECK, 0, 0l) == BST_CHECKED)
				SendMessage(dlg, WM_COMMAND, IDC_NEWPRESET, 0l);
			else
				SendMessage(dlg, WM_COMMAND, IDC_NEWCUSTOM, 0l);
			return TRUE;
		case IDC_NEWPRESET:
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESETSIZE), TRUE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESETSLIDER), TRUE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWCUSTOMSIZE), FALSE);
			return TRUE;
		case IDC_NEWCUSTOM:
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESETSIZE), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWPRESETSLIDER), FALSE);
			EnableWindow(GetDlgItem(dlg, IDC_NEWCUSTOMSIZE), TRUE);
			return TRUE;
		case IDC_NEWDIRC:
			if (SendMessage(GetDlgItem(dlg, IDC_NEWDIRC), BM_GETCHECK, 0, 0l) == BST_CHECKED) {
				SendMessage(GetDlgItem(dlg, IDC_NEWINTL), BM_SETCHECK, TRUE, 0l);
				EnableWindow(GetDlgItem(dlg, IDC_NEWINTL), FALSE);
			}else
				EnableWindow(GetDlgItem(dlg, IDC_NEWINTL), TRUE);
			return TRUE;

		case IDC_NEWHELP:
			// Implement help button.
			WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_CREATE_DIALOGUE);
			return TRUE;

		}
		break;
	case WM_CLOSE:
		EndDialog(dlg, TRUE);
		return TRUE;
	case WM_HSCROLL:
	case TB_LINEDOWN:
	case TB_LINEUP:
		newPos = SendMessage(GetDlgItem(dlg, IDC_NEWPRESETSLIDER), TBM_GETPOS, 0, 0l);
		itoa(1 << newPos, size, 10);
		strcat(size, "MB");
		SetWindowText(GetDlgItem(dlg, IDC_NEWPRESETSIZE), size);
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

void NewCreate(HWND dlg)
{
	int iType;
	char ts[30];

	/* determine size of file to create (in SECTORS) */
	iType = SendMessage(GetDlgItem(dlg, IDC_NEWADF), BM_GETCHECK, 0, 0l);
	if (iType == BST_CHECKED) {
		/* it is a adf file */
		Size = (SendMessage(GetDlgItem(dlg, IDC_NEWHD), BM_GETCHECK, 0, 0l)
			== BST_CHECKED) ? 1760 << 1 : 1760;		
	} else {
		/* hardfile */
		if (SendMessage(GetDlgItem(dlg, IDC_NEWPRESET), BM_GETCHECK, 0, 0l)
			== BST_CHECKED) {
			/* one of the preset sizes */
			Size = (2 << SendMessage(GetDlgItem(dlg, IDC_NEWPRESETSLIDER),
				TBM_GETPOS, 0, 0l)) * 1024;
		} else {
			/* custom size */
			GetDlgItemText(dlg, IDC_NEWCUSTOMSIZE, ts, sizeof(ts));
			Size = atol(ts) << 1;
		}
	}

	if (! Size) {
		MessageBox(dlg, "The custom size you specified is not valid",
			"Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* create the file */
	GetDlgItemText(dlg, IDC_NEWPATH, gstrFileName, sizeof(gstrFileName));
	Done = FALSE;

	/* display the create progress dialogue */
	_beginthread(NewCreateFile, 0, dlg);
	DialogBox(instance, MAKEINTRESOURCE(IDD_PROGRESS1), dlg, (DLGPROC)NewProgressProc);
	SendMessage(dlg, WM_COMMAND, IDCANCEL, 0l);
	if (SendMessage(GetDlgItem(dlg, IDC_NEWOPEN), BM_GETCHECK, 0, 0l) == BST_CHECKED)
			CreateChildWin(ghwndMDIClient, CHILD_AMILISTER);
}

void NewSelectFile(HWND dlg)
{
	OPENFILENAME ofn;

	strcpy(gstrFileName, "");

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = dlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Amiga Disk File (*.adf)\0*.adf\0Hard Disk File (*.hdf)\0*.hdf\0Any file\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.lpstrFile = gstrFileName;
	ofn.nMaxFile = sizeof(gstrFileName);
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Save disk image";
	ofn.Flags = OFN_SHAREAWARE | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = NULL;

	if (GetSaveFileName(&ofn)) {
		SetDlgItemText(dlg, IDC_NEWPATH, gstrFileName);
	}
}

LRESULT CALLBACK NewProgressProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
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
		SendMessage(GetDlgItem(dlg, IDC_CREATEPROGRESS), PBM_SETPOS, Percent, 0l);
		return TRUE;
	}

	return FALSE;
}

void NewCreateFile(void *lpVoid)
{
	struct Device	*dev;
	char			tempStr[31];
	HWND			dlg = (HWND)lpVoid;
	int				type = 0;
	struct Volume	*vol;

	Percent = 0;

	if (Size == 1760) /* 880KB Floppy */
		dev = adfCreateDumpDevice(gstrFileName, 80, 2, 11);
	else
		if (Size == (1760 * 2)) /* HD Floppy */
			dev = adfCreateDumpDevice(gstrFileName, 80, 2, 22);
		else /* hardfile */
			dev = adfCreateDumpDevice(gstrFileName, Size, 1, 1);

	GetDlgItemText(dlg, IDC_NEWLABEL, tempStr, sizeof(tempStr));

	if (SendMessage(GetDlgItem(dlg, IDC_NEWFFS), BM_GETCHECK, 0, 0l) == BST_CHECKED)
		type += FSMASK_FFS;
	if (SendMessage(GetDlgItem(dlg, IDC_NEWINTL), BM_GETCHECK, 0, 0l) == BST_CHECKED)
		type += FSMASK_INTL;
	if (SendMessage(GetDlgItem(dlg, IDC_NEWDIRC), BM_GETCHECK, 0, 0l) == BST_CHECKED)
		type += FSMASK_DIRCACHE;
	
	if ((Size == 1760) || (Size == 1760 * 2)){
		adfCreateFlop(dev, tempStr, type);
		// Install bootblock if "Bootable" selected.
		if(SendMessage(GetDlgItem(dlg, IDC_NEWBOOTABLE), BM_GETCHECK, 0, 0l) == BST_CHECKED){
			vol = adfMount(dev, 0, FALSE);
			InstallBootBlock(dlg, vol, TRUE);
		}
	}
	else
		adfCreateHdFile(dev, tempStr, type);

	adfUnMountDev(dev);

	Done = TRUE;
}
