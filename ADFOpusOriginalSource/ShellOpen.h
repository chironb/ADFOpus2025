/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * This code Copyright 2000 by Gary Harris.
 *
 * ShellOpen.h - prototypes for file registration functions
 */



#ifndef SHELLOPEN_H_SEEN
#define SHELLOPEN_H_SEEN

#include <windows.h>

int	 RegisterFileTypes(void);
long CreateFileTypeKey(char *szFileType, char *szFileTypeTarget);
long CreateFileTypeTargetKey(char *szFileTypeTarget, char *szDescription,
							 char *szIconID);


char		szOpusPath[MAX_PATH];		// Path to ADF Opus.


#endif //SHELLOPEN_H_SEEN