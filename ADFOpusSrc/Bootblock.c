/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file Bootblock.c
 *  \brief Bootblock display functions.
 *
 * Bootblock.c - routines to handle the display bootblock dialogue
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "ChildCommon.h"
#include "Help\AdfOpusHlp.h"
#include "Bootblock.h"

extern char gstrFileName[MAX_PATH * 2];

#include <windows.h>
#include <stdio.h>
#include <ctype.h>

extern char gstrFileName[MAX_PATH * 2];   // your global ADF path
extern HINSTANCE instance;                // your app instance

//------------------------------------------------------------------------------
// Turn a binary buffer into a classic hex+ASCII dump, 16 bytes per line
static void FormatHexDump(
    const unsigned char* buf,
    size_t               len,
    char* out,
    size_t               outSize)
{
    char* p = out;
    size_t   rem = outSize;
    int      written;

    for (size_t offset = 0; offset < len; offset += 16) {
        // 1) offset
        written = _snprintf_s(p, rem, _TRUNCATE, "%04X: ", (unsigned)offset);
        if (written < 0) break;
        p += written; rem -= written;

        // 2) hex bytes
        for (int i = 0; i < 16; ++i) {
            if (offset + i < len) {
                written = _snprintf_s(p, rem, _TRUNCATE,
                    "%02X ", buf[offset + i]);
            }
            else {
                written = _snprintf_s(p, rem, _TRUNCATE, "   ");
            }
            if (written < 0) break;
            p += written; rem -= written;
        }

        // 3) gap
        if (rem > 1) { *p++ = ' '; rem--; }

        // 4) ASCII characters
        for (int i = 0; i < 16 && rem > 1; ++i) {
            unsigned char c = (offset + i < len) ? buf[offset + i] : ' ';
            *p++ = (c >= 32 && c < 127) ? (char)c : '.';
            rem--;
        }

        // 5) newline
        written = _snprintf_s(p, rem, _TRUNCATE, "\r\n");
        if (written < 0) break;
        p += written; rem -= written;

        if (rem < 1) break;
    }

    if (outSize > 0)
        out[outSize - 1] = '\0';
}





//------------------------------------------------------------------------------
// Copies the entire contents of IDC_EDIT_BOOTBLOCK into the clipboard
static void CopyBootblockHexToClipboard(HWND dlg)
{
    HWND    hEdit = GetDlgItem(dlg, IDC_EDIT_BOOTBLOCK);
    int     len = GetWindowTextLengthA(hEdit);
    if (len <= 0)
        return;

    // Grab the text
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (!hMem)
        return;

    char* ptr = (char*)GlobalLock(hMem);
    if (ptr) {
        GetWindowTextA(hEdit, ptr, len + 1);
        GlobalUnlock(hMem);

        // Open & clear the clipboard, put in our text
        if (OpenClipboard(dlg)) {
            EmptyClipboard();
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
            // NOTE: do NOT GlobalFree(hMem) after SetClipboardData—Windows owns it now
        }
        else {
            GlobalFree(hMem);
        }
    }
    else {
        GlobalFree(hMem);
    }
}




//-----------------------------------------------------------------------------
// Bootblock display dialog proc
LRESULT CALLBACK DisplayBootblockProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    static DWORD aIds[] = {
        IDC_EDIT_BOOTBLOCK, IDH_BOOTBLOCK_DISPLAY,
        IDC_BOOTBLOCK_HELP, IDH_BOOTBLOCK_HELP_BUTTON,
        IDOK,               IDH_BOOTBLOCK_OK_BUTTON,
        0,                  0
    };

    switch (msg) {
    case WM_INITDIALOG:
    {
        unsigned char buffer[1024];
        char          dump[8192];
        FILE* fp = fopen(gstrFileName, "rb");

        if (!fp) {
            char err[512];
            _snprintf_s(err, sizeof(err), _TRUNCATE,
                "Unable to open:\n%s", gstrFileName);
            MessageBoxA(dlg, err, "ADF Opus Error", MB_OK | MB_ICONSTOP);
            EndDialog(dlg, TRUE);
            return TRUE;
        }

        if (fread(buffer, 1, sizeof(buffer), fp) != sizeof(buffer)) {
            fclose(fp);
            char err[512];
            _snprintf_s(err, sizeof(err), _TRUNCATE,
                "Unable to read 1024 bytes from:\n%s",
                gstrFileName);
            MessageBoxA(dlg, err, "ADF Opus Error", MB_OK | MB_ICONSTOP);
            EndDialog(dlg, TRUE);
            return TRUE;
        }
        fclose(fp);

        FormatHexDump(buffer, sizeof(buffer), dump, sizeof(dump));
        SetDlgItemTextA(dlg, IDC_EDIT_BOOTBLOCK, dump);
        return TRUE;
    }

    case WM_COMMAND:
        WORD id = LOWORD(wp);
        WORD ev = HIWORD(wp);

        if (id == IDC_BOOTBLOCK_COPY && ev == BN_CLICKED)
        {
            CopyBootblockHexToClipboard(dlg);
            return TRUE;
        }

        if (id == IDC_BOOTBLOCK_HELP)
        {
            WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_BOOTBLOCK);
            return TRUE;
        }
        if (id == IDOK)
        {
            EndDialog(dlg, TRUE);
            return TRUE;
        }

        break;

    case WM_CLOSE:
        EndDialog(dlg, TRUE);
        return TRUE;

    case WM_HELP:
        WinHelp(((LPHELPINFO)lp)->hItemHandle,
            "adfopus.hlp",
            HELP_WM_HELP,
            (DWORD_PTR)aIds);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wp,
            "adfopus.hlp",
            HELP_CONTEXTMENU,
            (DWORD_PTR)aIds);
        break;
    }

    return FALSE;
}
