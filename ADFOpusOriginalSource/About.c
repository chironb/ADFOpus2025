/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file About.c
 *  \brief About box functions.
 *
 * About.c - routines to handle about box messages
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "About.h"
#include "ADFlib.h"
#include "zlib.h"

/* local prototypes */
LRESULT CALLBACK AboutChildDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutChildDlgProc(HWND, UINT, WPARAM, LPARAM);
void InitAboutBox(HWND);
void AboutOnSelChanged(HWND);
void KillAboutBox(HWND);

/* external globals */
extern HINSTANCE instance;

LRESULT CALLBACK AboutDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
/*! Callback function for the about dialogue.		<BR>
 *	Input:											<BR>	
 *		dlg		- Handle to parent window.			<BR>
 *		msg		- the Windows message being sent.	<BR>
 *		wp		- message parameter					<BR>
 *		lp		- message parameter					<BR>
 *	Output:											<BR>
 *		LRESULT - Success or failure.
 */
{
	LPNMHDR nmhdr;
	aboutInfo *ai;

	switch(msg) {
		case WM_INITDIALOG:
			ai = malloc(sizeof(aboutInfo));
			SetWindowLong(dlg, GWL_USERDATA, (long)ai);
			ai->tabControl = GetDlgItem(dlg, IDC_ABOUTTABCTRL);		
			InitAboutBox(dlg);
			return TRUE;
		case WM_COMMAND:
			switch(wp) {
				case IDCANCEL:
					EndDialog(dlg, TRUE);
					return TRUE;
			}
			break;
		case WM_CLOSE:
			KillAboutBox(dlg);
			EndDialog(dlg, TRUE);
			return TRUE;
		case WM_NOTIFY:
			nmhdr = (NMHDR *)lp;
			switch(nmhdr->code) {
				case TCN_SELCHANGE:
					AboutOnSelChanged(dlg);
			return TRUE;
		}
	}
	return FALSE;
}

LRESULT CALLBACK AboutChildDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
/*! shared callback function for the child dialogues (pages)	<BR>
 *	Input:														<BR>
 *		dlg		- Handle to parent window.						<BR>
 *		msg		- the Windows message being sent.				<BR>
 *		wp		- message parameter								<BR>
 *		lp		- message parameter								<BR>
 *	Output:														<BR>	
 *		LRESULT - Success or failure.							<BR>
 */
{
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case IDC_WEBPAGEBTN:
			ShellExecute(dlg, "open", "http://adfopus.sourceforge.net/",
				NULL, NULL, 0);
			break;
		case IDC_EMAILBTN:
			ShellExecute(dlg, "open", "mailto:gharris@zip.com.au",
				NULL, NULL, 0);
			break;
		case IDC_ADFLIBBTN:
			ShellExecute(dlg, "open", "http://perso.club-internet.fr/lclevy/adflib/",
				NULL, NULL, 0);
			break;
		case IDC_ZLIBBTN:
			ShellExecute(dlg, "open", "http://www.cdrom.com/pub/infozip/zlib/",
				NULL, NULL, 0);
			break;
		}
	}
	return FALSE;
}

void InitAboutBox(HWND dlg)
/// Initialise the about property sheet.		<BR>
///	Input:										<BR>
///		dlg - Handle to parent window.			<BR>
///	Output:										<BR>
///		void.
{
	TC_ITEM tie;
	RECT rec;
	int i;
	aboutInfo *ai = (aboutInfo *)GetWindowLong(dlg, GWL_USERDATA);
	char tempStr[1024];

	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
	tie.pszText = "About";
	TabCtrl_InsertItem(ai->tabControl, 0, &tie);
	tie.pszText = "ADFlib";
	TabCtrl_InsertItem(ai->tabControl, 1, &tie);
	tie.pszText = "xDMS";
	TabCtrl_InsertItem(ai->tabControl, 2, &tie);
	tie.pszText = "ZLib";
	TabCtrl_InsertItem(ai->tabControl, 3, &tie);
	tie.pszText = "Credits";
	TabCtrl_InsertItem(ai->tabControl, 4, &tie);
	tie.pszText = "Greets";
	TabCtrl_InsertItem(ai->tabControl, 5, &tie);

	ai->pages[0] = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUTPAGE1), ai->tabControl, AboutChildDlgProc);
	ai->pages[1] = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUTPAGE4), ai->tabControl, AboutChildDlgProc);
	ai->pages[2] = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUTPAGE6), ai->tabControl, AboutChildDlgProc);
	ai->pages[3] = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUTPAGE5), ai->tabControl, AboutChildDlgProc);
	ai->pages[4] = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUTPAGE2), ai->tabControl, AboutChildDlgProc);
	ai->pages[5] = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUTPAGE3), ai->tabControl, AboutChildDlgProc);

	// Get Opus version number and write to page 1 of About box.
	LoadString(instance, IDS_VERSION, tempStr, sizeof(tempStr));
	SetDlgItemText(ai->pages[0], IDC_OPUS_VERSION, tempStr);
	
	// ADFLib, ZLib version and date.
	sprintf(tempStr, "This copy of ADF Opus is using ADFlib version %s.", adfGetVersionNumber());
	SetDlgItemText(ai->pages[1], IDC_ADFLIBVER, tempStr);
	sprintf(tempStr, "%s", adfGetVersionDate());
	SetDlgItemText(ai->pages[1], IDC_ADFLIBDATE, tempStr);
	sprintf(tempStr, "ADF Opus uses ZLib %s for its GZip compression support.", zlibVersion());
	SetDlgItemText(ai->pages[3], IDC_ZLIBVER, tempStr);

	GetClientRect(ai->tabControl, &rec);
	TabCtrl_AdjustRect(ai->tabControl, FALSE, &rec);

	for (i = 0 ; i < 6 ; i++) {
		MoveWindow(ai->pages[i], rec.left, rec.top, rec.right - rec.left, rec.bottom - rec.top, FALSE);
	}

	ai->curPage = 0;
	ShowWindow(ai->pages[ai->curPage], SW_SHOW);
}

void AboutOnSelChanged(HWND dlg)
/// Select a new property page.				<BR>
///	Input:									<BR>
///		dlg - Handle to parent window.		<BR>
///	Output:									<BR>
///		void.
{ 
	aboutInfo *ai = (aboutInfo *)GetWindowLong(dlg, GWL_USERDATA);

	ShowWindow(ai->pages[ai->curPage], SW_HIDE);
	ai->curPage = TabCtrl_GetCurSel(ai->tabControl);
	ShowWindow(ai->pages[ai->curPage], SW_SHOW);
}

void KillAboutBox(HWND dlg)
/// Close the about property sheet.			<BR>
///	Input:									<BR>
///		dlg - Handle to parent window.		<BR>
///	Output:									<BR>
///		void.
{
	int i;

	aboutInfo *ai = (aboutInfo *)GetWindowLong(dlg, GWL_USERDATA);

	for (i = 0 ; i < 4 ; i++)
		DestroyWindow(ai->pages[i]);
}
