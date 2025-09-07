/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * Options.h - definitions for options dialogue
 */

#ifndef OPTIONS_H
#define OPTIONS_H

struct OPTIONS {
	BOOL useDirCache;
	char defaultDir[MAX_PATH];
	char defaultLabel[31];
	BOOL confirmDelete;
	BOOL confirmDeleteDirs;
	BOOL defDriveList;
};

LRESULT CALLBACK OptionsProc(HWND, UINT, WPARAM, LPARAM);
void ReadOptions();
void WriteOptions();

#endif /* ndef OPTIONS_H */
