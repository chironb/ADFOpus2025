/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file Bootblock.c
 *  \brief Bootblock display functions.
 *
 * Bootblock.c - routines to handle the display bootblock dialogue
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "ChildCommon.h"
#include "Help\AdfOpusHlp.h"
#include "Bootblock.h"

extern char gstrFileName[MAX_PATH * 2];

LRESULT CALLBACK DisplayBootblockProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static DWORD aIds[] = { 
		IDC_EDIT_BOOTBLOCK,	IDH_BOOTBLOCK_DISPLAY,
		IDC_BOOTBLOCK_HELP,	IDH_BOOTBLOCK_HELP_BUTTON,	
		IDOK,				IDH_BOOTBLOCK_OK_BUTTON,	
		0,0 
	}; 	


	char	lpBuffer[1024];
	FILE	*fp;

	switch(msg) {
	case WM_INITDIALOG:

		fp = fopen(gstrFileName, "rb");
		if(fread(lpBuffer, sizeof(byte), 1024, fp) != 1024){
			// Abort if can't read bootcode.
			MessageBox(dlg, strcat("Unable to read from ", gstrFileName), "ADF Opus Error", MB_OK|MB_ICONSTOP);
			fclose(fp);
			EndDialog(dlg, TRUE);
			return TRUE;
		}
		else{
			SetDlgItemText(dlg, IDC_EDIT_BOOTBLOCK, lpBuffer);
		}
		fclose(fp);
		return TRUE;

	case WM_COMMAND:
		switch((int)LOWORD(wp)) {

		case IDOK:
			EndDialog(dlg, TRUE);
			return TRUE;


		case IDC_BOOTBLOCK_HELP:
			// Implement help button.
			WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_BOOTBLOCK);
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


