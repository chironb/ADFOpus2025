/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * Init.h - prototypes for initialisation routines
 */

#ifndef INIT_H
#define INIT_H

#include <string.h>

BOOL RegisterAppClass(HANDLE hInstance);
VOID UnregisterAllClasses(HANDLE hInstance);
HWND CreateAppWindow(HANDLE hInstance);
HWND CreateMDIClientWindow(HWND hwndFrame);
HIMAGELIST CreateImageList();
HWND CreateToolBar(HWND hwndParent);
HWND CreateStatusBar(HWND hwndParent);


#endif /* ndef INIT_H */