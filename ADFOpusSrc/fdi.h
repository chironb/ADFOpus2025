/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * This code by Gary Harris.
 *
 * FDI.h - routines to handle the Disk2FDI dialogue.
 */

#ifndef DISK2FDI_H
#define DISK2FDI_H

#include <string.h>

LRESULT CALLBACK	Disk2FDIProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp);
void				RunDisk2FDI(HWND dlg);


#endif /* ndef DISK2FDI_H */
