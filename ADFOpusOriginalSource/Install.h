/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 *	This code by Gary Harris.
 *
 * Install.h - Bootblock installation routines.
 */


#ifndef INSTALL_H
#define INSTALL_H

#include "Pch.h"

#include "ADFOpus.h"
#include "ADFlib.h"
#include "ChildCommon.h"
#include "adf_nativ.h"

InstallBootBlock(HWND win, struct Volume *vol, BOOL bNewAdf);
//BOOL CheckForNDOS();


#endif /* INSTALL_H */
