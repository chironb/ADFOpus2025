/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * This code Copyright 2000 by Gary Harris.
 *
 */
/*! \file ShellOpen.c
 *  \brief File type registration functions.
 *
 * ShellOpen.c - routines to handle registration of Amiga file types with Windows.
 */

#include "shellopen.h"
#include <string.h>
#include <stdio.h>
#include <direct.h>

int	RegisterFileTypes(void)
// Register Amiga Disk File types with Windows.
// Input:  Nil.
// Output: Returns success or failure.
{
	// Array of strings describing Opus file types:
	// "file type key", "file type target key", "Description", "Icon index within AdfOpus.exe".
	char	*KeyArray[20] = {".adf", "ADFOpus.adf", "Amiga Disk File",			  "12",
							 ".adz", "ADFOpus.adz", "GZipped Amiga Disk File",	  "13",
							 ".dms", "ADFOpus.dms", "DiskMashed Amiga Disk File", "14",
							 ".hdf", "ADFOpus.hdf",	"Amiga Hard Disk File",		  "15",
							 ".dmp", "ADFOpus.dmp", "Amiga Hard Disk Dump File",  "16"};
	int			iNumKeys = 20;
	int			i;
	char		szError[100];


	getcwd(szOpusPath, MAX_PATH);					// Get the path to ADF Opus.
	strcat(szOpusPath, "\\ADFOpus.exe");

	for(i = 0;i < iNumKeys;i=i+4){
		if(CreateFileTypeKey(KeyArray[i], KeyArray[i+1]) != 0){
			sprintf(szError, "Error creating file type key \"%s\"", KeyArray[i]);
			MessageBox(NULL, "ADF Opus", szError, IDOK);
			return(-1);
		}
		if(CreateFileTypeTargetKey(KeyArray[i+1], KeyArray[i+2], KeyArray[i+3]) != 0){
			sprintf(szError, "Error creating file type target key \"%s\"", KeyArray[i+1]);
			MessageBox(NULL, "ADF Opus", szError, IDOK);
			return(-1);
		}
	}
	
	return(0);
}


//////////////////

long CreateFileTypeKey(char *szFileType, char *szFileTypeTarget)
// Creates registry keys for Amiga file types .adf, .adz, .dms, .hdf and .dmp.
// Input:  Receives file type and target key strings from the master key array.
// Output: Returns a long value which is either an error code returned by RegCreateKeyEx()
//         or RegSetValueEx() or 0 on success.
{
	char		szClass[] = "Amiga Disk File";		// String values.
	CONST BYTE	*lpKeyData = szFileTypeTarget;
	HKEY		hkResult;
	HKEY		*phkResult = &hkResult;
	DWORD		dwDisposition;
	LPDWORD		lpdwDisposition = &dwDisposition;
	DWORD		cbData;
	long		lRetVal;

	// Create file type key. 
	cbData = strlen(szFileTypeTarget) + 1;
	lRetVal = RegCreateKeyEx(HKEY_CLASSES_ROOT,		// handle to an open key
							szFileType,				// address of subkey name
							0,						// reserved
							szClass,				// address of class string
							REG_OPTION_NON_VOLATILE,// special options flag
							KEY_ALL_ACCESS,			// desired security access
							NULL,					// address of key security structure
							phkResult,				// address of buffer for opened handle
							lpdwDisposition);		// address of disposition value buffer
									
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult);						// Close the open key.
		return(lRetVal);
	}
		
	lRetVal = RegSetValueEx(hkResult,				// handle to key to set value for
							NULL,					// name of the value to set
							0,						// reserved  
							REG_SZ,					// flag for value type
							szFileTypeTarget,		// address of value data
							cbData);				// size of value data

	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult);						// Close the open key.
		return(lRetVal);
	}

	RegCloseKey(hkResult);							// Close the open key.
	return(0);										// Return success.
}


long CreateFileTypeTargetKey(char *szFileTypeTarget, char *szDescription, char *szIconID)
// This function creates target registry keys for the file type subkeys
// e.g. ADFOpus.adf for the .adf file type key.
// Input:  Target key name, file description and icon index number strings from the
//		   master key array.
// Output: Returns a long value which is either an error code returned by RegCreateKeyEx()
//         or RegSetValueEx() or 0 on success.
{
	const char	lpOpenLine[] = "&Open with ADF Opus";
	const char	lpIconSubKey[] = "DefaultIcon";			// Subkeys.
	const char	lpShellSubKey[] = "shell";
	const char	lpOpenSubKey[] = "open";
	const char	lpCommandSubKey[] = "command";

	char		szIconPath[MAX_PATH];
	char		szCommandLine[MAX_PATH];
	CONST BYTE	*lpDescription = szDescription;
	CONST BYTE	*lpIconPath = szIconPath;
	CONST BYTE	*lpOpenLineData = lpOpenLine;
	CONST BYTE	*lpCommandLineData = szCommandLine;

	HKEY		hkResult1, hkResult2, hkResult3, hkResult4;
	HKEY		*phkResult1 = &hkResult1, *phkResult2 = &hkResult2,
				*phkResult3 = &hkResult3, *phkResult4 = &hkResult4;
	DWORD		dwDisposition;
	LPDWORD		lpdwDisposition = &dwDisposition;
	DWORD		cbData;
	long		lRetVal;

	
	// Build the command line.
	strcpy(szCommandLine, szOpusPath);
	strcat(szCommandLine, " %1");
	
	// Create the icon path string.
	strcpy(szIconPath, szOpusPath);	
	strcat(szIconPath, ",");
	strcat(szIconPath, szIconID);			

	// Create target key for .xxx file type - "ADFOpus.xxx".
	lRetVal = RegCreateKeyEx(HKEY_CLASSES_ROOT,		// handle to an open key
						   szFileTypeTarget,		// address of subkey name
						   0,						// reserved
						   szDescription,			// address of class string
						   REG_OPTION_NON_VOLATILE, // special options flag
						   KEY_ALL_ACCESS,			// desired security access
						   NULL,					// address of key security structure
						   phkResult1,				// address of buffer for opened handle
						   lpdwDisposition);		// address of disposition value buffer);
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open key.
		return(lRetVal);
	}


	// Set (default) to description of file type.
	cbData = strlen(szDescription) + 1;
	lRetVal = RegSetValueEx(hkResult1,				// handle to key to set value for
							NULL,					// name of the value to set
							0,						// reserved  
							REG_SZ,					// flag for value type
							lpDescription,			// address of value data
							cbData);				// size of value data
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open key.
		return(lRetVal);
	}

				 
	// Create the "Default Icon" subkey.
	lRetVal = RegCreateKeyEx(hkResult1,				// handle to an open key
						   lpIconSubKey,			// address of subkey name
						   0,						// reserved
						   szDescription,			// address of class string
						   REG_OPTION_NON_VOLATILE, // special options flag
						   KEY_ALL_ACCESS,			// desired security access
						   NULL,					// address of key security structure
						   phkResult2,				// address of buffer for opened handle
						   lpdwDisposition);		// address of disposition value buffer);
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		return(lRetVal);
	}


	// Set default icon path.
	cbData = strlen(szIconPath) + 1;
	lRetVal = RegSetValueEx(hkResult2,				// handle to key to set value for
						   NULL,					// name of the value to set
						   0,						// reserved  
						   REG_SZ,					// flag for value type
						   szIconPath,				// address of value data
						   cbData);					// size of value data
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		return(lRetVal);
	}
	
	RegCloseKey(hkResult2);							// Close the open key.

	// Create the "shell" subkey.
	lRetVal = RegCreateKeyEx(hkResult1,				// handle to an open key
						   lpShellSubKey,			// address of subkey name
						   0,						// reserved
						   szDescription,			// address of class string
						   REG_OPTION_NON_VOLATILE, // special options flag
						   KEY_ALL_ACCESS,			// desired security access
						   NULL,					// address of key security structure
						   phkResult2,				// address of buffer for opened handle
						   lpdwDisposition);		// address of disposition value buffer);
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		return(lRetVal);
	}

	// Create the "shell\open" subkey.
	lRetVal = RegCreateKeyEx(hkResult2,				// handle to an open key
						   lpOpenSubKey,			// address of subkey name
						   0,						// reserved
						   szDescription,			// address of class string
						   REG_OPTION_NON_VOLATILE, // special options flag
						   KEY_ALL_ACCESS,			// desired security access
						   NULL,					// address of key security structure
						   phkResult3,				// address of buffer for opened handle
						   lpdwDisposition);		// address of disposition value buffer);
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		RegCloseKey(hkResult3);
		return(lRetVal);
	}

	// Set the "open" command entry line.
	cbData = strlen(lpOpenLine) + 1;
	lRetVal = RegSetValueEx(hkResult3,				// handle to key to set value for
						   NULL,					// name of the value to set
						   0,						// reserved  
						   REG_SZ,					// flag for value type
						   lpOpenLineData,			// address of value data
						   cbData);					// size of value data
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		RegCloseKey(hkResult3);
		return(lRetVal);
	}
	
	// Create the "shell\open\command" subkey.
	lRetVal = RegCreateKeyEx(hkResult3,				// handle to an open key
							lpCommandSubKey,		// address of subkey name
							0,						// reserved
							szDescription,			// address of class string
							REG_OPTION_NON_VOLATILE, // special options flag
							KEY_ALL_ACCESS,			// desired security access
							NULL,					// address of key security structure
							phkResult4,				// address of buffer for opened handle
							lpdwDisposition);		// address of disposition value buffer);
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		RegCloseKey(hkResult3);
		RegCloseKey(hkResult4);
		return(lRetVal);
	}

	// Set the command line.
	cbData = strlen(szCommandLine) + 1;
	lRetVal = RegSetValueEx(hkResult4,				// handle to key to set value for
							NULL,					// name of the value to set
							0,						// reserved  
							REG_SZ,					// flag for value type
							lpCommandLineData,		// address of value data
							cbData);				// size of value data
	// Check for error condition.
	if(lRetVal != ERROR_SUCCESS){
		RegCloseKey(hkResult1);						// Close the open keys.
		RegCloseKey(hkResult2);
		RegCloseKey(hkResult3);
		RegCloseKey(hkResult4);
		return(lRetVal);
	}
	
	
	// Close all open keys.
	RegCloseKey(hkResult1);
	RegCloseKey(hkResult2);
	RegCloseKey(hkResult3);
	RegCloseKey(hkResult4);
	return(0);
}