/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 *	This code by Gary Harris.
 *
 */
/*! \file Install.c
 *  \brief Bootblock installation functions.
 *
 * Install.c - Bootblock installation routines.
 */

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include "adflib.h"    // for RC_OK, adfWriteBlock, etc.

 /* externs from your codebase */
extern char dirOpus[MAX_PATH];


//#include "Install.h"
//
//    
//extern char	gstrFileName[MAX_PATH * 2];
//extern char	dirOpus[MAX_PATH];
//
//InstallBootBlock(HWND win, struct Volume *vol, BOOL bNewAdf)
//{
//    FILE* boot;
//    unsigned char	bootcode[1024], szBootPath[MAX_PATH];
//	char			*szWarning = "ADFLib is unable to detect whether or not existing boot code is valid. "
//								"You should display the bootblock to get some idea of whether or not it"
//								"contains reasonable looking code and test bootblock installation on a "
//								"copy of the ADF unless you are sure you want to continue. Overwriting "
//								"many non-standard or proprietry bootblocks will invalidate the ADF."
//								"\n\nDo you still want to install the new bootblock?";
// 
//	char			*szBBMissing = "ADF Opus can't open the bootblock file. Please make sure that the "
//								"bootblock (\"stdboot3.bbk\") is located in a sub-directory called \"Boot\" "
//								"in the directory where ADF Opus is installed. This should automatically be "
//								"the case if you use any of the automated installation methods.";
//
//
//
//	if(bNewAdf || MessageBox(win, szWarning, "ADF Opus Warning!", MB_YESNO|MB_ICONWARNING) == IDYES){
//		strcpy(szBootPath, dirOpus);
//		strcat(szBootPath, "\\Boot\\stdboot3.bbk");
//		// MessageBox(win, szBootPath, "Bootblock Path", MB_OK | MB_ICONINFORMATION); // for debugging
//		boot = fopen(szBootPath, "rb");
//		if (!boot) {
//   			MessageBox(win, szBBMissing, "ADF Opus error", MB_OK|MB_ICONERROR);
//			return(1);
//		}
//		fread(bootcode, sizeof(unsigned char), 1024, boot);
//		fclose(boot);
//		
//		if(adfInstallBootBlock(vol,  bootcode) != RC_OK)
//			return 1;
//	}
//	return 0;
//}












#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include "Install.h"

extern char  gstrFileName[MAX_PATH * 2];
extern char  dirOpus[MAX_PATH];

/*
  Prompts the user for a bootblock file, reads 1024 bytes,
  and installs it into the given volume.
*/
int InstallBootBlock(HWND win, struct Volume* vol, BOOL bNewAdf)
{
    static const char* szWarning =
        "ADFLib is unable to detect whether or not existing boot code is valid. "
        "You should display the bootblock to get some idea of whether or not it "
        "contains reasonable looking code and test bootblock installation on a "
        "copy of the ADF unless you are sure you want to continue. Overwriting "
        "many non-standard or proprietary bootblocks will invalidate the ADF.\n\n"
        "Do you still want to install the new bootblock?";
    static const char* szBBMissing =
        "ADF Opus can't open the selected bootblock file. Please pick a valid file.";

    unsigned char  bootcode[1024];
    char           initialDir[MAX_PATH];
    char           fileName[MAX_PATH] = "";
    OPENFILENAMEA  ofn = { 0 };
    FILE* fp;
    // DWORD          read;


    HINSTANCE hInst = GetModuleHandle(NULL);
    // hInst is the HINSTANCE passed into WinMain <-- This works here too!
    PlaySound(
        MAKEINTRESOURCE(IDR_NOTIFICATION_WAVE_1),
        hInst,
        SND_RESOURCE | SND_ASYNC
    );

    // If this is a new ADF or user confirms overwrite...
    //if (bNewAdf || MessageBoxA(win, szWarning, "ADF Opus Warning!", MB_YESNO | MB_ICONWARNING) == IDYES)
    if (bNewAdf || MessageBoxA(win, szWarning, "ADF Opus Warning!", MB_YESNO ) == IDYES)
    {
        // Build default "Boot" subfolder path
        lstrcpyA(initialDir, dirOpus);
        lstrcatA(initialDir, "\\Boot");

        // Configure Open-File dialog
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = win;
        ofn.lpstrFilter = "Bootblock Files (*.bbk)\0*.bbk\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrInitialDir = initialDir;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        // Show dialog; if user cancels, abort
        if (!GetOpenFileNameA(&ofn))
            return 1;

        // Open the chosen file
        fp = fopen(fileName, "rb");
        if (!fp) {
            MessageBoxA(win, szBBMissing, "ADF Opus Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        // Read exactly 1024 bytes
        if (fread(bootcode, 1, sizeof(bootcode), fp) != sizeof(bootcode)) {
            fclose(fp);
            MessageBoxA(win, "Failed to read 1024 bytes from the file.", "ADF Opus Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        fclose(fp);

        // Install into the volume
        if (adfInstallBootBlock(vol, bootcode) != RC_OK)
            return 1;
    }

    return 0;
}



/*
  RawWriteBootBlock
    Prompts the user for a 1024-byte file and writes it
    directly into logical blocks 0 and 1 of the volume.
    No checksum or signature is modified.

  Returns 0 on success, nonzero on cancel or error.
*/
int RawWriteBootBlock(HWND win, struct Volume* vol, BOOL bNewAdf)
{
    static const char* szPrompt =
        "You are about to write the selected file verbatim as the ADF bootblock.\n"
        "No checksum or DOS signature adjustments will be performed.\n\n"
        "Continue?";
    static const char* szFileError =
        "Unable to open the selected file. Please pick a valid 1024-byte file.";
    static const char* szReadError =
        "Failed to read 1024 bytes from the file.";
    static const char* szWriteError =
        "Failed to write the bootblock to disk.";
    unsigned char  bootcode[1024];
    char           initialDir[MAX_PATH];
    char           fileName[MAX_PATH] = "";
    OPENFILENAMEA  ofn = { 0 };
    FILE* fp;
    size_t         got;

    // If not a fresh ADF, warn the user
    if (!bNewAdf
        && MessageBoxA(win, szPrompt, "ADF Opus Warning", MB_YESNO | MB_ICONWARNING) != IDYES)
    {
        return 1;
    }

    // Build default “Boot” folder path
    lstrcpyA(initialDir, dirOpus);
    lstrcatA(initialDir, "\\Boot");

    // Configure the Open-File dialog
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = win;
    ofn.lpstrFilter = "Raw Bootblock (*.bbk;*.bin)\0*.bbk;*.bin\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (!GetOpenFileNameA(&ofn))
        return 1;  // user cancelled

    // Open and read exactly 1024 bytes
    fp = fopen(fileName, "rb");
    if (!fp) {
        MessageBoxA(win, szFileError, "ADF Opus Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    got = fread(bootcode, 1, sizeof(bootcode), fp);
    fclose(fp);
    if (got != sizeof(bootcode)) {
        MessageBoxA(win, szReadError, "ADF Opus Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Write raw blocks 0 and 1
    if (adfWriteBlock(vol, 0, bootcode) != RC_OK
        || adfWriteBlock(vol, 1, bootcode + 512) != RC_OK)
    {
        MessageBoxA(win, szWriteError, "ADF Opus Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}



///*
//  Prompts the user for a bootblock file, reads 1024 bytes,
//  and installs it into the given volume.
//*/
//int RawWriteBootBlock(HWND win, struct Volume* vol, BOOL bNewAdf)
//{
//
//    MessageBoxA(win, "RawWRiteBootBlock", "DEBUG:", MB_OK | MB_ICONERROR);
//    
//    return 0;
//
//}





/////// Not currently needed as Opus won't open NDOS disks.

//BOOL CheckForNDOS()

/*first look if there is the DOS letters at the 3 first bytes of the bootblock,
then look if there is code at the 12th byte (this byte and the 3 following must not be 0).

working bootcode is at least around 20 bytes long...

one simple way to offer better information is to display the bootblocks sectors...
0-31 and 128-255 should be remplaced by a '.'...
*//*
{
	HANDLE				hFile;
	char				lpBuffer[4];
	LPDWORD				lpNumberOfBytesRead = 0; //**********
	long				longFilePtr;
	struct nativeDevice	*nDev;
 

	DWORD	retval;
	
//	hFile = CreateFile(gstrFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
//						FILE_ATTRIBUTE_NORMAL, NULL);
//	retval = GetLastError(); //**********

	nDev = ci->dev->nativeDev;
	longFilePtr = ftell(nDev->fd);			// Get position of file pointer.
	fseek(nDev->fd, 0, SEEK_SET);			// Seek to start of file.

	// Read the first 3 bytes and check for "DOS".
//	if(!ReadFile(nDev->fd, lpBuffer, 3, lpNumberOfBytesRead, NULL))
//		;

//size_t fread( void *buffer, size_t size, size_t count, FILE *stream );		
	retval = GetLastError(); //**********

	fseek(nDev->fd, longFilePtr, SEEK_SET);	// Reset file pointer to where we found it.



	return TRUE;
}
*/