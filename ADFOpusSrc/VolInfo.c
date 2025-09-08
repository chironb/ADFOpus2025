/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file VolInfo.c
 *  \brief Volume Information dialogue functions.
 *
 * VolInfo.c - routines for the volume info tabbed dialogue
 */

#include "Pch.h"

#include "Utils.h"
#include "ADFOpus.h"
#include "ChildCommon.h"
#include "ADF_raw.h" /* we shouldn't really be using this */
#include "ADF_Bitm.h" /* or this */
#include "ADF_Nativ.h" /* or this */
#include "VolInfo.h"
#include "ListView.h"
#include "Help\AdfOpusHlp.h"

/* local prototypes */
void VolInfoInit(HWND, HWND);
LRESULT CALLBACK VolInfoDlgProc(HWND, UINT, WPARAM, LPARAM);
void VolInfoOnSelChanged(HWND);
void VolInfoKill(HWND, HWND);
LRESULT CALLBACK VolInfoChildDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp);

/* globals */
HWND tabControl;
int curPage = 0;
HWND pages[2];

extern HINSTANCE instance;
extern HWND ghwndMDIClient;
extern char gstrFileName[MAX_PATH * 2];

LRESULT CALLBACK VolInfoDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	LPNMHDR nmhdr;

	static DWORD aIds[] = {
		IDC_STATIC_PATH,		IDH_INFO_DEV_PATH,
		IDC_DEVPATH,			IDH_INFO_DEV_PATH,
		IDC_STATIC_SIZE,		IDH_INFO_DEV_SIZE,
		IDC_DEVSIZE,			IDH_INFO_DEV_SIZE,
		IDC_STATIC_TYPE,		IDH_INFO_DEV_TYPE,
		IDC_DEVTYPE,			IDH_INFO_DEV_TYPE,	
		IDC_STATIC_CYLINDERS,	IDH_INFO_DEV_GEOMETRY,
		IDC_DEVCYLINDERS,		IDH_INFO_DEV_GEOMETRY,
		IDC_STATIC_HEADS,		IDH_INFO_DEV_GEOMETRY,
		IDC_DEVHEADS,			IDH_INFO_DEV_GEOMETRY,
		IDC_STATIC_SECTORS,		IDH_INFO_DEV_GEOMETRY,
		IDC_DEVSECT,			IDH_INFO_DEV_GEOMETRY,
		IDC_DEVVOLLIST,			IDH_INFO_DEV_VOLUMES,
		IDC_VOLLABEL,			IDH_INFO_VOL_LABEL,
		IDC_VOLUSED,			IDH_INFO_VOL_USED,
		IDC_VOLFULLGAUGE,		IDH_INFO_VOL_USED,
		IDC_VOLFREE,			IDH_INFO_VOL_FREE,
		IDC_VOLTOTAL,			IDH_INFO_VOL_TOTAL,
		IDC_VOLFULL,			IDH_INFO_VOL_FULL,
		IDC_VOLFFS,				IDH_INFO_VOL_FFS,
		IDC_VOLDIRC,			IDH_INFO_VOL_DC,
		IDC_VOLINTL,			IDH_INFO_VOL_INTL,
		IDC_VOLINFO_HELP,		IDH_INFO_HELP_BUTTON,
		IDCANCEL,				IDH_INFO_CLOSE_BUTTON,
		IDC_UPDATELABEL,
		0,0 
	}; 	
	
	switch(msg) {
	case WM_INITDIALOG:
		tabControl = GetDlgItem(dlg, IDC_VOLINFOTABCTRL);		
		VolInfoInit(dlg, tabControl);
		return TRUE;
	case WM_COMMAND:
		switch (wp) {
		case IDCANCEL:
			VolInfoKill(dlg, tabControl);
			return TRUE;

		case IDC_VOLINFO_HELP:
			// Implement help button.
			//WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_INFORMATION_DIALOGUE_DEV);
			return TRUE;



		case IDC_UPDATELABEL:
			MessageBox(dlg, "IDC_UPDATELABEL", "DEBUG:", MB_OK | MB_ICONINFORMATION);
			return TRUE;

		}
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lp;
		switch(nmhdr->code) {
		case TCN_SELCHANGE:
			VolInfoOnSelChanged(dlg);
			return TRUE;
		}
		break;



	case WM_CLOSE:
		VolInfoKill(dlg, tabControl);
		return TRUE;

		// Context sensitive help.
    case WM_HELP: 
   //     WinHelp(((LPHELPINFO) lp)->hItemHandle, "adfopus.hlp", 
			//HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        //WinHelp((HWND) wp, "adfopus.hlp", HELP_CONTEXTMENU, 
        //    (DWORD) (LPVOID) aIds); 
        break; 	
	}
	return FALSE;
}

void VolInfoInit(HWND dlg, HWND tc)
/* set up the dialogue, create tabs and show the first page
 */
{
	TC_ITEM tie;
	RECT rTC;
	int i;
	HWND ac;
	CHILDINFO *ci;
	char tempStr[50], buf[50];
	struct bRootBlock root;
	int percent;
	struct nativeDevice *nd;

	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
	tie.pszText = "Device";
	TabCtrl_InsertItem(tc, 0, &tie);
	tie.pszText = "Volume";
	TabCtrl_InsertItem(tc, 1, &tie);

	pages[0] = CreateDialog(instance, MAKEINTRESOURCE(IDD_VOLINFOPAGE1), tc, VolInfoChildDlgProc);
	pages[1] = CreateDialog(instance, MAKEINTRESOURCE(IDD_VOLINFOPAGE2), tc, VolInfoChildDlgProc);

	GetClientRect(tc, &rTC);
	TabCtrl_AdjustRect(tc, FALSE, &rTC);

	for (i = 0 ; i < 2 ; i++) {
		MoveWindow(pages[i], rTC.left, rTC.top, rTC.right - rTC.left, rTC.bottom - rTC.top, FALSE);
	}

	ac = (HWND)SendMessage(ghwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM) NULL);
	ci = (CHILDINFO *)GetWindowLong(ac, 0);

	/* device info */
	nd = (struct nativeDevice *)ci->dev->nativeDev;

	SetDlgItemText(pages[0], IDC_DEVPATH, ci->orig_path);

	switch (ci->dev->devType) {
	case DEVTYPE_FLOPDD:
		strcpy(tempStr, "Double Density Floppy (880KB ADF)");
		break;
	case DEVTYPE_FLOPHD:
		strcpy(tempStr, "High Density Floppy (1760KB ADF)");
		break;
	case DEVTYPE_HARDDISK:
		strcpy(tempStr, "Hard disk dump");
		break;
	case DEVTYPE_HARDFILE:
		strcpy(tempStr, "Hardfile");
		break;
	default:
		strcpy(tempStr, "Unknown (!)");
	}	



	// Chiron 2025
	// 
	//// Prefix compression type to type string.
	//if(ci->dfDisk == ADZ){
	//	strcpy(buf, tempStr);
	//	strcpy(tempStr, "Compressed ");
	//	strcat(tempStr, buf);
	//}
	//	else if (ci->dfDisk == DMS) {
	//		strcpy(buf, tempStr);
	//		strcpy(tempStr, "Diskmashed ");
	//		strcat(tempStr, buf);
	//}
	//
	// Prefix compression type to type string.
	if (ci->dfDisk == (enum DiskFormat)ADZ) {
		strcpy(buf, tempStr);
		strcpy(tempStr, "Compressed ");
		strcat(tempStr, buf);
	}
	else if(ci->dfDisk == (enum DiskFormat)DMS){
		strcpy(buf, tempStr);
		strcpy(tempStr, "Diskmashed ");
		strcat(tempStr, buf);
	}
	



	SetDlgItemText(pages[0], IDC_DEVTYPE, tempStr);

	// Chiron 2025
	// 
	//if(ci->dfDisk == ADF){
	//	SizeToStr(ci->dev->size, tempStr);
	//	SetDlgItemText(pages[0], IDC_DEVSIZE, tempStr);		// ADF size.
	//}
	//
	if (ci->dfDisk == (enum DiskFormat)ADF) {
		SizeToStr(ci->dev->size, tempStr);
		SetDlgItemText(pages[0], IDC_DEVSIZE, tempStr);		// ADF size.
	}
	else{
		_itoa(ci->compSize/1024, tempStr, 10);				// Convert to string and KB.
		strcat(tempStr, "KB");
		SetDlgItemText(pages[0], IDC_DEVSIZE, tempStr);		// Compressed disk size.
	}
	
	itoa(ci->dev->cylinders, tempStr, 10);
	SetDlgItemText(pages[0], IDC_DEVCYLINDERS, tempStr);
	itoa(ci->dev->heads, tempStr, 10);
	SetDlgItemText(pages[0], IDC_DEVHEADS, tempStr);
	itoa(ci->dev->sectors, tempStr, 10);
	SetDlgItemText(pages[0], IDC_DEVSECT, tempStr);

	/* fill volume list */
	LVAddColumn(GetDlgItem(pages[0], IDC_DEVVOLLIST), "ID", 30, 0);
	LVAddColumn(GetDlgItem(pages[0], IDC_DEVVOLLIST), "Name", 100, 1);
	LVAddColumn(GetDlgItem(pages[0], IDC_DEVVOLLIST), "Start", 70, 2);
	LVAddColumn(GetDlgItem(pages[0], IDC_DEVVOLLIST), "End", 70, 3);

	for (i = 0 ; i < ci->dev->nVol ; i++) {
		itoa(i, tempStr, 10);
		LVAddItem(GetDlgItem(pages[0], IDC_DEVVOLLIST), tempStr, 1);
	        if (ci->dev->volList[i]->volName)
			strcpy(tempStr, ci->dev->volList[i]->volName);
		else
			strcpy(tempStr, "-");

		LVAddSubItem(GetDlgItem(pages[0], IDC_DEVVOLLIST), tempStr, i, 1);
		itoa(ci->dev->volList[i]->firstBlock, tempStr, 10);
		LVAddSubItem(GetDlgItem(pages[0], IDC_DEVVOLLIST), tempStr, i, 2);
		itoa(ci->dev->volList[i]->lastBlock, tempStr, 10);
		LVAddSubItem(GetDlgItem(pages[0], IDC_DEVVOLLIST), tempStr, i, 3);
	}

	/* volume info */
	adfReadRootBlock(ci->vol, ci->vol->rootBlock, &root);
	memset(tempStr, 0, sizeof(tempStr));
	memcpy(tempStr, root.diskName, root.nameLen);
	SetDlgItemText(pages[1], IDC_VOLLABEL, tempStr);

	SizeToStr((ci->vol->lastBlock + 1 - ci->vol->firstBlock) * LOGICAL_BLOCK_SIZE, tempStr);
	SetDlgItemText(pages[1], IDC_VOLTOTAL, tempStr);
	SizeToStr(((ci->vol->lastBlock + 1 - ci->vol->firstBlock) - adfCountFreeBlocks(ci->vol)) * LOGICAL_BLOCK_SIZE, tempStr);
	SetDlgItemText(pages[1], IDC_VOLUSED, tempStr);
	SizeToStr((adfCountFreeBlocks(ci->vol)) * LOGICAL_BLOCK_SIZE, tempStr);
	SetDlgItemText(pages[1], IDC_VOLFREE, tempStr);
	percent = (((ci->vol->lastBlock + 1 - ci->vol->firstBlock) -
		adfCountFreeBlocks(ci->vol)) * 100) /
		(ci->vol->lastBlock + 1 - ci->vol->firstBlock);
	itoa(percent, tempStr, 10);
	strcat(tempStr, "%");
	SetDlgItemText(pages[1], IDC_VOLFULL, tempStr);

	/* DOS type */
	SetDlgItemText(pages[1], IDC_VOLFFS, isFFS(ci->vol->dosType) ? "YES" : "NO");
	SetDlgItemText(pages[1], IDC_VOLINTL, isINTL(ci->vol->dosType) ? "YES" : "NO");
	SetDlgItemText(pages[1], IDC_VOLDIRC, isDIRCACHE(ci->vol->dosType) ? "YES" : "NO");

	SendMessage(GetDlgItem(pages[1], IDC_VOLFULLGAUGE), PBM_SETPOS, percent, 0l);

	curPage = 0;
	ShowWindow(pages[curPage], SW_SHOW);
}

void VolInfoOnSelChanged(HWND dlg)
/* hide the currently shown page and display a replacement
 */
{ 
	ShowWindow(pages[curPage], SW_HIDE);

	curPage = TabCtrl_GetCurSel(tabControl);

	ShowWindow(pages[curPage], SW_SHOW);
}

//LRESULT CALLBACK VolInfoChildDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
///* none of the child windows do anything, so we give them this token proc
// */
//{
//	return FALSE;
//}




//void VolUpdateLabel(HWND dlg) {
//
//	// 1) Grab the text from the label control
//	char labelText[256] = { 0 };
//	HWND hLabel = GetDlgItem(dlg, IDC_VOLLABEL);
//	if (hLabel)
//	{
//		// Pull up to 255 chars (leaving room for null)
//		GetWindowTextA(hLabel, labelText, sizeof(labelText));
//	}
//	else
//	{
//		strcpy_s(labelText, sizeof(labelText), "<no label>");
//	}
//
//
//	// 2) Show it in the message box
//	MessageBoxA(
//		dlg,
//		labelText,
//		"Current Volume Label",
//		MB_OK | MB_ICONINFORMATION
//	);
//
//}
#include <windows.h>
#include <string.h>     // for memset, memcpy
#include "adf_blk.h"    // struct bRootBlock, MAXNAMELEN
#include "adf_raw.h"    // adfReadRootBlock, adfWriteRootBlock, RC_OK
#include "ChildCommon.h"// CHILDINFO

extern HWND ghwndMDIClient;  // your MDI host
//----------------------------------------------------------------------------  
void VolUpdateLabel(HWND dlg)
{
	// 1) Pull new label text out of the edit control
	char labelText[256] = { 0 };
	GetDlgItemTextA(dlg, IDC_VOLLABEL, labelText, sizeof(labelText));

	// 2) Find the active child and its Volume*
	HWND hActive = (HWND)SendMessage(ghwndMDIClient,
		WM_MDIGETACTIVE, 0, 0);
	CHILDINFO* ci = (CHILDINFO*)GetWindowLong(hActive, 0);
	if (!ci || !ci->vol)
	{
		MessageBoxA(dlg,
			"Unable to locate the current volume.",
			"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// 3) Read the existing root block
	struct bRootBlock root;
	if (adfReadRootBlock(ci->vol,
		ci->vol->rootBlock,
		&root) != RC_OK)
	{
		MessageBoxA(dlg,
			"Failed to read the root block from the ADF file.",
			"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// 4) Copy our new name into the block (truncate if too long)
	size_t inLen = strlen(labelText);
	size_t maxLen = sizeof(root.diskName) - 1;
	if (inLen > maxLen) inLen = maxLen;

	root.nameLen = (char)inLen;
	memset(root.diskName, 0, sizeof(root.diskName));
	memcpy(root.diskName, labelText, inLen);

	// 5) Write it back (adfWriteRootBlock will rebuild the checksum)
	if (adfWriteRootBlock(ci->vol,
		ci->vol->rootBlock,
		&root) != RC_OK)
	{
		MessageBoxA(dlg,
			"Failed to write the updated root block to the ADF file.",
			"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// 6) Success!
	MessageBoxA(dlg,
		"Volume label updated successfully in the ADF file.",
		"Success", MB_OK | MB_ICONINFORMATION);
}


//-----------------------------------------------------------------------------
// This is the dialog‐proc for each of your tab pages.
// Controls on page1/page2 land here, so handle IDC_UPDATELABEL here.
LRESULT CALLBACK VolInfoChildDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_COMMAND:
	{
		int id = LOWORD(wp);
		int code = HIWORD(wp);

		if (id == IDC_UPDATELABEL && code == BN_CLICKED)
		{
			VolUpdateLabel(dlg);

			return TRUE;
		}
	}
	break;
	}

	return FALSE;
}


void VolInfoKill(HWND dlg, HWND tc)
/* destroy the child windows and quit
 */
{
	DestroyWindow(pages[0]);
	DestroyWindow(pages[1]);
	EndDialog(dlg, TRUE);
}
