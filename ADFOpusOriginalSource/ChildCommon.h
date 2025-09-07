/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * ChildCommon.h - definitions common to both types of child windows
 */

#ifndef CHILDCOMMON_H
#define CHILDCOMMON_H

#define WINLISTERCLASSNAME "ADFOpusWindowsFileList"
#define AMILISTERCLASSNAME "ADFOpusAmigaFileList"

#include "ADFLib.h"

typedef struct _DIRENTRY {
	char	name[MAX_PATH];
	long	size;
	char	flags[9];
	int		icon;
	struct	_DIRENTRY *next;
} DIRENTRY;

typedef struct _CHILDINFO {
	BOOL			isAmi;				/* true if Amiga, false if Windows */
	HWND			lv;					/* handle of listview control */
	HWND			sb;					/* handle of status bar */
	char			curDir[MAX_PATH];	/* current directory */
	int				totalCount;			/* total no. of files or dirs */
	BOOL			atRoot;				/* true if at root dir */
	DIRENTRY		*content;			/* head of directory contents list */
	struct Device	*dev;
	struct Volume	*vol;
	BOOL			readOnly;
	enum DiskFormat	dfDisk;				// Disk format.
	int				compSize;			// Size of compressed disk image.
	char			orig_path[MAX_PATH];// Path to original (un/compressed) file.
	char			temp_path[MAX_PATH];// Path to temp work adf file.
} CHILDINFO;

HWND CreateChildWin(HWND, long);
LRESULT CALLBACK ChildWinProc(HWND, UINT, WPARAM, LPARAM);
int GetFileFromADF(struct Volume *vol, char	*szFileName);	// Extract a file from an ADF.

// Updated ci and file name accessed by Properties.c.
CHILDINFO	*ci;
char		buf[MAX_PATH];
HMENU		hMenu;

#endif /* ndef CHILDCOMMON_H */
