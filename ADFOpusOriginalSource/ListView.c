/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file Listview.c
 *  \brief Listview functions.
 *
 * ListView.c - general-purpose routines related to list-view common controls
 */

#include "Pch.h"

#include "ListView.h"

int LVGetItemFocused(HWND lv)
/* returns the index of the list-view item that has the focus or -1
 * if no item has the focus
 *
 * hWndLV - handle of the list-view control
 */
{
	int iNumItems = ListView_GetItemCount(lv);
	int i;

	for (i = 0 ; i < iNumItems ; i++) {
		if (ListView_GetItemState(lv, i, LVIS_FOCUSED))
			return i;
	}

	return -1;
}

void LVSelectAll(HWND lv)
/* selects all items in a list-view control
 *
 * hwndLV - handle of the list-view control
 */
{
	int iNumItems = ListView_GetItemCount(lv);
	int i;

	for (i = 0 ; i < iNumItems ; i++)
		ListView_SetItemState(lv, i, LVIS_SELECTED, LVIS_SELECTED);
}

void LVSelectNone(HWND lv)
/* deselects all items in a list-view control
 *
 * lv - handle of the list-view control
 */
{
	int count = ListView_GetItemCount(lv);
	int i;

	for (i = 0 ; i < count ; i++)
		ListView_SetItemState(lv, i, 0, LVIS_SELECTED);
}

void LVInvert(HWND lv)
/* toggles selected items in a list-view control
 *
 * lv - handle of the list-view control
 */
{
	int count = ListView_GetItemCount(lv);
	int i;
	UINT newState;

	for (i = 0 ; i < count ; i++) {
		newState = ListView_GetItemState(lv, i, LVIS_SELECTED) == LVIS_SELECTED;
		ListView_SetItemState(lv, i, newState ? 0 : LVIS_SELECTED, LVIS_SELECTED);
	}
}

BOOL LVAddColumn(HWND lv, char *title, int width, int pos)
/* adds a column to a list-view control
 *
 * hwndLV - handle of the list-view control
 * pstrTitle - heading for the new column
 * iWidth - width of the new column (in pixels)
 * iPos - position (index) to add column in
 *
 * returns: TRUE if the column was added OK, FALSE otherwise
 */
{
	LV_COLUMN lvc;
 
	/* Initialize the LV_COLUMN structure. */
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = title;

	/* Add the column */
	lvc.iSubItem = 0; 
	lvc.cx = width;

	return (ListView_InsertColumn(lv, pos, &lvc) != -1);
}

BOOL LVGetItemCaption(HWND lv, char *buf, int bufSize, int index)
/* returns the caption of the specified item in a list-view control
 *
 * hwndLV - handle of the list-view control
 * szStrBuf - buffer to store the caption string
 * iBufSize - size of the string buffer
 * iIndex - index of the item to get
 *
 * returns: TRUE if the operation succeeded, FALSE otherwise
 */
{
	LV_ITEM lvi;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = buf;
	lvi.cchTextMax = bufSize;

	return ListView_GetItem(lv, &lvi);
}

int LVGetItemImageIndex(HWND lv, int index)
/* returns the image index of the specified item in the specified list-view
 *
 * hwndLV - handle of the list-view control
 * iIndex - index of the item to get
 *
 * returns: the image index or -1 on error
 */
{
	LV_ITEM lvi;

	lvi.mask = LVIF_IMAGE;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = NULL;
	lvi.cchTextMax = 0;
	if (! ListView_GetItem(lv, &lvi))
		return -1;

	return lvi.iImage;
}

BOOL LVGetItemText(HWND lv, int index, char *buf, int bufSize)
/* gets the text of a specified list-view item and puts it in the provided buffer
 *
 * hwndLV - handle of the list-view control
 * iIndex - index of the item
 * strBuf - pointer to a buffer that will receive the text
 * iBS    - size of strBuf
 *
 * returns TRUE on success, otherwise FALSE (could happen if strBuf is too small)
 */
{
	LV_ITEM item;

	item.mask = LVIF_TEXT;
	item.iItem = index;
	item.iSubItem = 0;
	item.pszText = buf;
	item.cchTextMax = bufSize;

	return ListView_GetItem(lv, &item);
}

BOOL LVIsItemSelected(HWND lv, int index)
{
	return ListView_GetItemState(lv, index, LVIS_SELECTED) == LVIS_SELECTED;
}

int LVAddItem(HWND lv, char *caption, int image)
{
	LV_ITEM lvi;

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.pszText = caption;
	lvi.iItem = ListView_GetItemCount(lv);
	lvi.iSubItem = 0;
	lvi.iImage = image;

	if (ListView_InsertItem(lv, &lvi))
		return lvi.iItem;
	else
		return -1;
}

BOOL LVAddSubItem(HWND lv, char *caption, int item, int subitem)
{
	LV_ITEM lvi;

	lvi.mask = LVIF_TEXT;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.pszText = caption;
	lvi.iItem = item;
	lvi.iSubItem = subitem;
	lvi.iImage = 0;

	return ListView_SetItem(lv, &lvi);
}
