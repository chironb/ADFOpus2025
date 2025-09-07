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


#include "Install.h"

    
extern char	gstrFileName[MAX_PATH * 2];
extern char	dirOpus[MAX_PATH];

InstallBootBlock(HWND win, struct Volume *vol, BOOL bNewAdf)
{
    FILE* boot;
    unsigned char	bootcode[1024], szBootPath[MAX_PATH];
	char			*szWarning = "ADFLib is unable to detect whether or not existing boot code is valid. "
								"You should display the bootblock to get some idea of whether or not it"
								"contains reasonable looking code and test bootblock installation on a "
								"copy of the ADF unless you are sure you want to continue. Overwriting "
								"many non-standard or proprietry bootblocks will invalidate the ADF."
								"\n\nDo you still want to install the new bootblock?";
 
	char			*szBBMissing = "ADF Opus can't open the bootblock file. Please make sure that the "
								"bootblock (\"stdboot3.bbk\") is located in a sub-directory called \"Boot\" "
								"in the directory where ADF Opus is installed. This should automatically be "
								"the case if you use any of the automated installation methods.";



	if(bNewAdf || MessageBox(win, szWarning, "ADF Opus Warning!", MB_YESNO|MB_ICONWARNING) == IDYES){
		strcpy(szBootPath, dirOpus);
		strcat(szBootPath, "\\Boot\\stdboot3.bbk");
		boot = fopen(szBootPath, "rb");
		if (!boot) {
   			MessageBox(win, szBBMissing, "ADF Opus error", MB_OK|MB_ICONERROR);
			return(1);
		}
		fread(bootcode, sizeof(unsigned char), 1024, boot);
		fclose(boot);
		
		if(adfInstallBootBlock(vol,  bootcode) != RC_OK)
			return 1;
	}
	return 0;
}



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