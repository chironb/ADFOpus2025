/* ADF Opus Copyright 1998-2003 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file TextViewer.c
 *  \brief Bootblock display functions.
 *
 * TextViewer.c - routines to handle the display bootblock dialogue
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "TextViewer.h"
#include "ChildCommon.h"
#include "Utils.h"
#include "ListView.h"
#include "Help\AdfOpusHlp.h"
#include <direct.h>



LRESULT CALLBACK TextViewerProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static DWORD aIds[] = { 
		IDC_EDIT_TEXT,			IDH_TEXTVIEWER_EDIT,
		IDC_TEXTVIEWER_HELP,	IDH_TEXTVIEWER_BUTTON_HELP,
		IDOK,					IDH_TEXTVIEWER_BUTTON_OK,
		0,0 
	}; 	


	char			*lpBuffer, szError[MAX_PATH];
	FILE			*fp;
//	DIRENTRY		*DirPtr = ci->content;
	char			lpFileName[MAX_PATH];	// Pointer to filename.
	int				i, iIndex, iLength;
	char			ch;
	HWND			hEdit, hButtonOK, hButtonHelp;
	int				nWidth, nHeight;
	RECT			lpRectButton, lpRectEdit, lpRectMarginOK, lpRectMarginHelp, lpRectMarginEdit;
	static HBRUSH	hBrush;


	hEdit = GetDlgItem (dlg, IDC_EDIT_TEXT);
	switch(msg){
	case WM_INITDIALOG:

		// Create file path.
		iIndex = LVGetItemFocused(ci->lv);
		LVGetItemCaption(ci->lv, buf, sizeof(buf), iIndex);
		switch (LVGetItemImageIndex(ci->lv, iIndex)) {
		case ICO_WINFILE:
			strcpy(lpFileName, ci->curDir);
			strcat(lpFileName, buf);
//			ChildUpdate(win);
			break;
		
		case ICO_AMIFILE:
			// Get the file and copy it to the temp dir.
			if(_chdir(dirTemp) == -1){						// Change to temp dir.
				_mkdir(dirTemp);							// Create if doesn't yet exist.
				_chdir(dirTemp);
			}
			if(GetFileFromADF(ci->vol, buf) < 0){
				sprintf(szError, "Error extracting %s from %s.", buf, ci->orig_path);
				MessageBox(dlg, szError, "ADF Opus Error", MB_ICONSTOP);
				_chdir(dirOpus);							// Change back to Opus dir.
				EndDialog(dlg, TRUE);
				return TRUE;
			}
			strcpy(lpFileName, dirTemp);
			strcat(lpFileName, buf);
			_chdir(dirOpus);								// Change back to Opus dir.
			break;
		default:
//			sprintf(szError, "You must select a file.");
//			MessageBox(dlg, szError, "ADF Opus Error", MB_ICONSTOP);
			EndDialog(dlg, TRUE);
			return TRUE;
		}
		iIndex = 0;
	
		// Display the file.
		fp = fopen(lpFileName, "r");

//		retval = fscanf(fp, "%s", lpBuffer);
//		while(retval != EOF){
//			SetFocus(hEdit);
//			SendMessage(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
//			SendMessage(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
//			SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM) ((LPSTR) lpBuffer));
//			retval = fscanf(fp, "%s", lpBuffer);
//		}

		fseek(fp, 0, SEEK_END);
		iLength = ftell(fp);
		rewind(fp);	
		if((lpBuffer = malloc(iLength)) == NULL)
			return FALSE;
//		fread(lpBuffer, 1, iLength, fp);
//		lpBuffer[iLength] = '\0';
//		SetDlgItemText(dlg, IDC_EDIT_TEXT, lpBuffer);
		
		ch = fgetc(fp);
		for(i = 0;feof(fp) == 0;i++){
			lpBuffer[i] = ch;
			if(ch == '\n'){
				/* Add null to end string */
				lpBuffer[i + 1] = '\0';
				iIndex = GetWindowTextLength (hEdit);
				SetFocus(hEdit);
				SendMessage(hEdit, EM_SETSEL, (WPARAM)iIndex, (LPARAM)iIndex);
				SendMessage(hEdit, EM_SETSEL, (WPARAM)iIndex, (LPARAM)iIndex);
				SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM) ((LPSTR) lpBuffer));
				// Reset counter. For loop will increment it to 0.
				i = -1;
			}
			ch = fgetc(fp);
		}

////		else
////			MessageBox(dlg, strcat("Unable to read from ", buf), "ADF Opus Error", MB_OK|MB_ICONSTOP);
		fclose(fp);

        // Create a blue brush to be used for the edit control's background color.
        hBrush = CreateSolidBrush(RGB(0, 0, 255));
		return TRUE;

	case WM_SIZE:
		nWidth = LOWORD(lp);								// Width of client area.
		nHeight = HIWORD(lp);								// Height of client area.

		hButtonOK = GetDlgItem (dlg, IDOK);
		hButtonHelp = GetDlgItem (dlg, IDC_TEXTVIEWER_HELP);

		lpRectMarginOK.right = 7;							// OK button right margin.
		lpRectMarginOK.bottom = 6;							// OK button bottom margin.
		lpRectMarginOK.top = 0;
		lpRectMarginOK.left = 0;
		MapDialogRect(dlg, &lpRectMarginOK);
		lpRectMarginHelp.right = 63;						// Help button right margin.
		lpRectMarginHelp.bottom = 6;						// Help button bottom margin.
		lpRectMarginHelp.top = 0;
		lpRectMarginHelp.left = 0;
		MapDialogRect(dlg, &lpRectMarginHelp);
		lpRectMarginEdit.right = 7;							// OK button right margin.
		lpRectMarginEdit.bottom = 28;						// OK button bottom margin.
		lpRectMarginEdit.top = 0;
		lpRectMarginEdit.left = 0;
		MapDialogRect(dlg, &lpRectMarginEdit);
		lpRectButton.right = 50;							// Button sizes.
		lpRectButton.bottom = 14;
		lpRectButton.top = 0;
		lpRectButton.left = 0;
		MapDialogRect(dlg, &lpRectButton);
		lpRectEdit.top = 7;;								// Edit control sizes.
		lpRectEdit.left = 7;
		lpRectEdit.right = 0;
		lpRectEdit.bottom = 0;
		MapDialogRect(dlg, &lpRectEdit);

		// Move the windows.
		MoveWindow(hButtonOK,
					nWidth - lpRectButton.right - lpRectMarginOK.right,
					nHeight - lpRectButton.bottom - lpRectMarginOK.bottom,
					lpRectButton.right,
					lpRectButton.bottom,
					TRUE);
		MoveWindow(hButtonHelp,
					nWidth - lpRectButton.right - lpRectMarginHelp.right,
					nHeight - lpRectButton.bottom - lpRectMarginHelp.bottom,
					lpRectButton.right,
					lpRectButton.bottom,
					TRUE);
		MoveWindow(hEdit,
					lpRectEdit.left,
					lpRectEdit.top,
					nWidth - lpRectMarginEdit.right - lpRectEdit.left,
					nHeight - lpRectMarginEdit.bottom - lpRectEdit.top,
					TRUE);
		return TRUE;

	case WM_COMMAND:
		switch((int)LOWORD(wp)) {

		case IDOK:
			EndDialog(dlg, TRUE);
			return TRUE;


		case IDC_TEXTVIEWER_HELP:
			// Implement help button.
			WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_TEXTVIEWER);
			return TRUE;

		}
		break;

	case WM_CLOSE:
		EndDialog(dlg, TRUE);
        DeleteObject(hBrush);
		return TRUE;

	// Context sensitive help.
    case WM_HELP:
        WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
		return TRUE;
 
    case WM_CONTEXTMENU: 
        WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, (DWORD) (LPVOID) aIds); 
		return TRUE;

	}
	return FALSE;
}

