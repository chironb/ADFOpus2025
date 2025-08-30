/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * Utils.h - prototypes for general utility functions, not specific to ADF Opus
 */

#ifndef UTILS_H
#define UTILS_H

#include "ChildCommon.h"


void SizeToParent(HWND, HWND);
BOOL OpenDlg(HWND);
void SizeToStr(const long, char *);
void AddCommas(char *);
void ResizeMDIClientWin();
void SetMenuBitmaps(HINSTANCE, HMENU);
void UpdateMenuItems(HMENU);
void UpdateToolbar();
int  CountOFNFiles(char *, int);
void GetOFNFile(char *, int, char *);
DIRENTRY *FindCIData(HWND dlg, DIRENTRY *DirPtr);

#ifdef DEBUG_INFO
// Debug info dialogue.
// Activate  DEBUG_INFO in ADFOpus.h.
void adfVolumeInfoWin(HWND hWnd, struct Volume *vol);
#endif

#endif /* ndef UTILS_H */
