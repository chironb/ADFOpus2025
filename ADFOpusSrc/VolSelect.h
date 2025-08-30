/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * VolSelect.h - definitions for Volume selector
 */

#ifndef VOLSELECT_H
#define VOLSELECT_H


LRESULT CALLBACK VolSelectProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
enum DiskFormat	VolGetFormat(HWND dlg);


// Globals for access by ChildCommon.c.
enum DiskFormat	dfDisk;
char			FileSuf[4], FileRoot[MAX_PATH];		// Disk file path path/name and suffix.
char			inBuf[MAX_PATH];					// Original file name.
int				comp_size;							// Original compressed file size.


#endif /* ndef VOLSELECT_H */

