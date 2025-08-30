/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * About.h - definitions for about box routines
 */

#ifndef ABOUT_H
#define ABOUT_H

typedef struct _aboutInfo {
	HWND tabControl;
	int curPage;
	HWND pages[4];
} aboutInfo;

LRESULT CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

#endif /* ndef ABOUT_H */
