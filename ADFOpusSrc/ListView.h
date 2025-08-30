/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * ListView.h - prototypes for list-view management functions
 */

#ifndef LISTVIEW_H
#define LISTVIEW_H

int LVGetItemFocused(HWND);
void LVSelectAll(HWND);
void LVSelectNone(HWND);
void LVInvert(HWND);
BOOL LVAddColumn(HWND, char *, int, int);
BOOL LVGetItemCaption(HWND, char *, int, int);
int LVGetItemImageIndex(HWND, int);
BOOL LVIsItemSelected(HWND, int);
BOOL LVAddItem(HWND, char *, int);
BOOL LVAddSubItem(HWND, char *, int, int);

#endif /* ndef LISTVIEW_H */