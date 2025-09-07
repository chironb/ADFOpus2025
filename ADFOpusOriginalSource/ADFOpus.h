/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * ADFOpus.h - global #defines and prototypes
 */


#ifndef ADFOPUS_H
#define ADFOPUS_H

//*************************************
//* Activate debug info dialogue.     *
//* Enable this or declare in VC++'s  *
//* preprocessor definitions.         *
//*************************************
//#define DEBUG_INFO
//*************************************

#include "Resource.h"

#define APPCLASSNAME "ADFOpus"
#define MDICLIENTWIN 0

/* icon indices for list-box controls */
#define ICO_WINFILE			0
#define ICO_WINDIR			1
#define ICO_AMIFILE			2
#define ICO_AMIDIR			3
#define ICO_DRIVEHD			4
#define ICO_DRIVENET		5
#define ICO_DRIVECD			6
#define ICO_DRIVEFLOP35		7
#define ICO_DRIVEFLOP514	8

/* possible types of MDI child window */
#define CHILD_WINLISTER 87651
#define CHILD_AMILISTER 87652

/* some resource IDs (values are arbitary) */
#define ID_TOOLBAR		45000
#define ID_STATUSBAR	45001
#define IDM_WINDOWCHILD 700

LONG APIENTRY MainWinProc(HWND, UINT, WPARAM, LPARAM);

/* ADFLib callback functions */
VOID ADFError(char *);
VOID ADFWarning(char *);
VOID ADFVerbose(char *);
void ADFAccess(SECTNUM, SECTNUM, BOOL);
void ADFProgress(int);
//void MainWinOnDragOver(int x, int y);
//void MainWinOnDrop();

/* struct containing some globals (stored in window long of frame) */
typedef struct _FRAMEINFO {
	HWND mdiClient;
	HIMAGELIST images;
	HWND toolbar;
	HWND statusBar;
	char currentFile[MAX_PATH * 3];
	BOOL toolbarVisible;
	BOOL statusBarVisible;
	HMENU mainMenu;
} FRAMEINFO;

// Temp directory and .adf name.
char	dirOpus[MAX_PATH], dirTemp[MAX_PATH], tempName[MAX_PATH];

// Disk file types.
enum df {ADF, ADZ, DMS};

// Handle to main menu. Accessed by ChildOnNotify() in ChildCommon.c.
HMENU		ghmenuMain;



#endif /* ndef ADFOPUS_H */


