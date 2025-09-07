/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file VolSelect.c
 *  \brief Volume Selection functions.
 *
 * VolSelect.c - routines to handle the volume selector (which appears when you
 *               open a device or dump file with more than one volume)
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "VolSelect.h"
#include "ListView.h"
#include "ADFlib.h"
#include <imagehlp.h>			// For MakeSureDirectoryPathExists().
#include "Help\AdfOpusHlp.h"
#include "ChildCommon.h"		// For access to buf for filename storage.


// For conversions.
#include "xDMS.h"
#include "BatchConvert.h"
#include "zLib.h"

extern int		volToOpen;
extern HANDLE	ghInstance;
extern char		gstrFileName[MAX_PATH * 2];
extern char		buf[MAX_PATH];



LRESULT CALLBACK VolSelectProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	int				i, count = 0;
	struct 	Device	*dev;
	char			tempStr[10];
	extern HWND		ghwndTB;
	FILE			*fileDisk;
	char			buffer[100];

	static DWORD aIds[] = { 
		IDC_VOLSELLIST,	IDH_VOLSEL_LIST,
		IDC_VOLSELOPEN,	IDH_VOLSEL_OPEN_BUTTON,
		IDC_VOLSELHELP,	IDH_VOLSEL_HELP_BUTTON,
		IDCANCEL,		IDH_VOLSEL_CANCEL_BUTTON,
		0,0 
	}; 	


	switch(msg) {
		case WM_INITDIALOG:
			volToOpen = -1;

			LVAddColumn(GetDlgItem(dlg, IDC_VOLSELLIST), "ID", 30, 0);
			LVAddColumn(GetDlgItem(dlg, IDC_VOLSELLIST), "Name", 100, 1);
			LVAddColumn(GetDlgItem(dlg, IDC_VOLSELLIST), "Start", 70, 2);
			LVAddColumn(GetDlgItem(dlg, IDC_VOLSELLIST), "End", 70, 3);

			// Store original filename for use in ChildCommon.c, ci->path.
			// Variable declared in ChildCommon.h.
			strcpy(buf, gstrFileName);		

			dfDisk = VolGetFormat(dlg);		// Check disk format. Unpack if not ADF.
			if(dfDisk == -1){				// File error, return.
				EndDialog(dlg, TRUE);
				return -1;
			}
			if(dfDisk != ADF){				// Get size of original compressed disk image.
				comp_size = 0;
				fileDisk = fopen(inBuf, "rb");
				while(!feof(fileDisk)){
					/* Attempt to read in 10 bytes: */
					count = fread(buffer, sizeof(char), 100, fileDisk);
					if(ferror(fileDisk)){
						MessageBox(dlg, "Read error", "Error", MB_OK|MB_ICONERROR);
						break;
					}
					/* Total up actual bytes read */
					comp_size += count;
				}
   				fclose(fileDisk);
			}

			dev = adfMountDev(gstrFileName, FALSE);
			if (dev == NULL) {
				EndDialog(dlg, TRUE);
				return -1;
			}

			if (dev->nVol < 2) {
				volToOpen = (dev->nVol == 1) ? 0 : -1;
				adfUnMountDev(dev);
				EndDialog(dlg, TRUE);
				return -1;
			}

			for (i = 0 ; i < dev->nVol ; i++) {
				itoa(i, tempStr, 10);
				LVAddItem(GetDlgItem(dlg, IDC_VOLSELLIST), tempStr, 1);
				if (dev->volList[i]->volName)
					strcpy(tempStr, dev->volList[i]->volName);
				else
					strcpy(tempStr, "-");

				LVAddSubItem(GetDlgItem(dlg, IDC_VOLSELLIST), tempStr, i, 1);
				itoa(dev->volList[i]->firstBlock, tempStr, 10);
				LVAddSubItem(GetDlgItem(dlg, IDC_VOLSELLIST), tempStr, i, 2);
				itoa(dev->volList[i]->lastBlock, tempStr, 10);
				LVAddSubItem(GetDlgItem(dlg, IDC_VOLSELLIST), tempStr, i, 3);
//				adfUnMountDev(dev);
			}
			//Moved from 2 lines following user bug report that only the first volume is displayed if
			// teh volume is unmounted in the FOR loop. Makes sense.
			adfUnMountDev(dev);

			return TRUE;
		case WM_COMMAND:
			switch(wp) {
				case IDCANCEL:
					volToOpen = -1;
					EndDialog(dlg, TRUE);
					return TRUE;
				case IDC_VOLSELOPEN:
					for(i = 0;i < ListView_GetItemCount(GetDlgItem(dlg, IDC_VOLSELLIST));i++)
						if (LVIsItemSelected(GetDlgItem(dlg, IDC_VOLSELLIST), i)) {
							volToOpen = i;
							EndDialog(dlg, TRUE);
							return TRUE;
						}

				case IDC_VOLSELHELP:
					// Implement help button.
					WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_VOLUME_SELECT);
					return TRUE;


			}
			break;
		case WM_CLOSE:
			volToOpen = -1;
			EndDialog(dlg, TRUE);
			return TRUE;
		case WM_NOTIFY:
			if (ListView_GetSelectedCount(GetDlgItem(dlg, IDC_VOLSELLIST)))
				EnableWindow(GetDlgItem(dlg, IDC_VOLSELOPEN), TRUE);
			else
				EnableWindow(GetDlgItem(dlg, IDC_VOLSELOPEN), FALSE);

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


enum DiskFormat	VolGetFormat(HWND dlg)
// Checks input file for ADZ or DMS types.
// Input:  receives a handle to the window on which error message boxes
//         should be displayed.
// Output: returns an enumerated disk type ADF, ADZ or DMS where ADF covers all
//         non-compressed types i.e. disk dumps, hardfiles etc.
{
	int		iLength, i;
	char	fileName[MAX_PATH];

	_splitpath(gstrFileName, NULL, NULL, fileName, NULL);	// Get filename.

	iLength = strlen(gstrFileName);							// Get name length.
	for(i = 0;i < iLength - 3;i++)							// Get name root.
		FileRoot[i] = gstrFileName[i];
	FileRoot[i] = '\0';

	FileSuf[0] = gstrFileName[iLength - 3];					// Get name suffix.
	FileSuf[1] = gstrFileName[iLength - 2];
	FileSuf[2] = gstrFileName[iLength - 1];
	FileSuf[3] = '\0';
		
	// Check for hardfile or dump. Return name unchanged if found.
	if(strcmp(FileSuf, "dmp") == 0 || strcmp(FileSuf, "DMP") == 0 ||
	   strcmp(FileSuf, "hdf") == 0 || strcmp(FileSuf, "HDF") == 0){
		
		return ADF;
	}

	// Copy input file name.
	strcpy(inBuf, gstrFileName);							// If ADF, leave original string to preserve case.
	
	// Open DMS.
	if(strcmp(FileSuf, "dms") == 0 || strcmp(FileSuf, "DMS") == 0 ){ 
		// Build temp file path name and copy back.
		strcpy(gstrFileName, dirTemp);
		MakeSureDirectoryPathExists(dirTemp);				// Create temp dir.
		strcat(gstrFileName, fileName);
		strcat(gstrFileName, ".adf");
		if(dmsUnpack(inBuf, gstrFileName) != NO_PROBLEM){
			MessageBox(dlg, "Couldn't unpack DMS", "Error", MB_OK|MB_ICONERROR);
			return -1;
		}
		return DMS;
	}

	// Open ADZ.	
	if(strcmp(FileSuf, "adz") == 0 || strcmp(FileSuf, "ADZ") == 0 ){ 
		// Build temp file path name and copy back.
		strcpy(gstrFileName, dirTemp);
		MakeSureDirectoryPathExists(dirTemp);				// Create temp dir.
		strcat(gstrFileName, fileName);
		strcat(gstrFileName, ".adf");
		if(!GZDecompress(NULL, inBuf, gstrFileName)){
			MessageBox(dlg, "Couldn't unpack ADZ", "Error", MB_OK|MB_ICONERROR);
			return -1;
		}
		return ADZ;
	}

	return ADF;
}
