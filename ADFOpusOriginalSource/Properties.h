/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 *	This code by Gary Harris.
 *
 * Properties.h - volume info dialogue's window proc prototype
 */


#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Pch.h"

#include "ADFOpus.h"
#include "Utils.h"
#include "ADFlib.h"
#include "Help\AdfOpusHlp.h"
#include "ChildCommon.h"
#include "ListView.h"

LRESULT CALLBACK PropertiesProcWin(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PropertiesProcAmi(HWND, UINT, WPARAM, LPARAM);

void GetPropertiesAmi(HWND dlg, DIRENTRY *DirPtr);
void GetPropertiesWin(HWND dlg, DIRENTRY *DirPtr);
void SetPropertiesAmi(HWND dlg, DIRENTRY *DirPtr);
void SetPropertiesWin(HWND dlg, DIRENTRY *DirPtr);

#endif /* PROPERTIES_H */
