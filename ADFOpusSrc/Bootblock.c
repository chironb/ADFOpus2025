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

#include <commdlg.h>     // for OPENFILENAME
#include <stdlib.h>      // for malloc/free
#include <string.h>

extern char gstrFileName[MAX_PATH * 2];   // your global ADF path
extern HINSTANCE instance;                // your app instance



#include <windows.h>
#include <stdio.h>    // for _flushall()
#include <ctype.h>
// … your other includes …

extern char gstrFileName[MAX_PATH * 2];
void FormatHexDump(const unsigned char* buf, size_t len, char* out, size_t outSize);


//------------------------------------------------------------------------------
// Reads the latest 1 KB from disk, formats it, and sets IDC_EDIT_BOOTBLOCK
static BOOL ReloadBootblock(HWND dlg)
{
    unsigned char buffer[1024];
    char          dump[8192];
    DWORD         bytesRead;

    // 1) Flush all C‐runtime file buffers so we force pending writes to disk
    _flushall();

    // 2) Open the ADF file for shared read/write
    HANDLE hFile = CreateFileA(
        gstrFileName,                          // file path
        GENERIC_READ,                          // read access
        FILE_SHARE_READ | FILE_SHARE_WRITE,    // allow other handles to write
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,             // optimize for linear reads
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    // 3) Read exactly 1024 bytes (Amiga bootblock)
    if (!ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL)
        || bytesRead != sizeof(buffer))
    {
        CloseHandle(hFile);
        return FALSE;
    }
    CloseHandle(hFile);

    // 4) Format and display
    FormatHexDump(buffer, sizeof(buffer), dump, sizeof(dump));
    SetDlgItemTextA(dlg, IDC_EDIT_BOOTBLOCK, dump);
    return TRUE;
}




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


/*
  Reads 1024 bytes from gstrFileName, formats as hex text,
  then lets the user save it via a standard Save-As dialog.
*/
static void SaveBootblockAsText(HWND dlg)
{
    unsigned char buffer[1024];
    char          dump[8192];
    char          initialDir[MAX_PATH];
    FILE* fp;

    fp = fopen(gstrFileName, "rb");
    if (!fp)
        return;
    if (fread(buffer, 1, sizeof(buffer), fp) != sizeof(buffer)) {
        fclose(fp);
        return;
    }
    fclose(fp);

    /* turn raw bytes into an ASCII hex dump */
    FormatHexDump(buffer, sizeof(buffer), dump, sizeof(dump));

    // Build default "Boot" subfolder path
    lstrcpyA(initialDir, dirOpus);
    lstrcatA(initialDir, "\\Boot");

    /* configure and show Save-As */
    OPENFILENAMEA ofn = { 0 };
    char          fileName[MAX_PATH] = "";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = dlg;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "txt";

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(
            fileName,
            GENERIC_WRITE,
            0, NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, dump, (DWORD)lstrlenA(dump), &written, NULL);
            CloseHandle(hFile);
        }
    }
}

/*
  Reads 1024 bytes from gstrFileName, then lets the user
  save those raw bytes as a .bin file.
*/
static void SaveBootblockAsBinary(HWND dlg)
{
    unsigned char buffer[1024];
    char          initialDir[MAX_PATH];
    FILE* fp;

    fp = fopen(gstrFileName, "rb");
    if (!fp)
        return;
    if (fread(buffer, 1, sizeof(buffer), fp) != sizeof(buffer)) {
        fclose(fp);
        return;
    }
    fclose(fp);

    // Build default "Boot" subfolder path
    lstrcpyA(initialDir, dirOpus);
    lstrcatA(initialDir, "\\Boot");

    /* configure and show Save-As */
    OPENFILENAMEA ofn = { 0 };
    char          fileName[MAX_PATH] = "";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = dlg;
    ofn.lpstrFilter = "Bootblock Files (*.bbk)\0*.bbk\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "bin";

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(
            fileName,
            GENERIC_WRITE,
            0, NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, buffer, sizeof(buffer), &written, NULL);
            CloseHandle(hFile);
        }
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

    switch (msg)
    {
    case WM_INITDIALOG:
        // Initial load when the dialog is created
        ReloadBootblock(dlg);
        return TRUE;

    case WM_ACTIVATE:
        // Reload whenever the dialog regains focus
        if (LOWORD(wp) == WA_ACTIVE || LOWORD(wp) == WA_CLICKACTIVE)
            ReloadBootblock(dlg);
        break;

    case WM_COMMAND:
        switch (LOWORD(wp))
        {
        case IDC_BOOTBLOCK_COPY:
            if (HIWORD(wp) == BN_CLICKED)
                CopyBootblockHexToClipboard(dlg);
            return TRUE;

        case IDC_BOOTBLOCK_SAVETEXT:
            if (HIWORD(wp) == BN_CLICKED)
                SaveBootblockAsText(dlg);
            return TRUE;

        case IDC_BOOTBLOCK_SAVEBINARY:
            if (HIWORD(wp) == BN_CLICKED)
                SaveBootblockAsBinary(dlg);
            return TRUE;

        case IDOK:
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




////-----------------------------------------------------------------------------
//// Bootblock display dialog proc
//LRESULT CALLBACK DisplayBootblockProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
//{
//    static DWORD aIds[] = {
//        IDC_EDIT_BOOTBLOCK, IDH_BOOTBLOCK_DISPLAY,
//        IDC_BOOTBLOCK_HELP, IDH_BOOTBLOCK_HELP_BUTTON,
//        IDOK,               IDH_BOOTBLOCK_OK_BUTTON,
//        0,                  0
//    };
//
//    switch (msg) {
//    case WM_INITDIALOG:
//    {
//        unsigned char buffer[1024];
//        char          dump[8192];
//        DWORD         bytesRead;
//        HANDLE        hFile;
//
//        // Open the ADF with write-through & share mode so we always get the
//        // freshest bytes on disk even if another handle in this process
//        // still holds a cached write.
//        hFile = CreateFileA(
//            gstrFileName,                   // path
//            GENERIC_READ,                   // only reading
//            FILE_SHARE_READ | FILE_SHARE_WRITE, // allow other read/write handles
//            NULL,                           // default security
//            OPEN_EXISTING,                  // must already exist
//            FILE_ATTRIBUTE_NORMAL
//            | FILE_FLAG_SEQUENTIAL_SCAN   // OPT: hint for linear reads
//            | FILE_FLAG_WRITE_THROUGH,    // force through OS cache
//            NULL);
//
//        if (hFile == INVALID_HANDLE_VALUE) {
//            char err[512];
//            _snprintf_s(err, sizeof(err), _TRUNCATE,
//                "Unable to open for viewing:\n%s", gstrFileName);
//            MessageBoxA(dlg, err, "ADF Opus Error", MB_OK | MB_ICONSTOP);
//            EndDialog(dlg, TRUE);
//            return TRUE;
//        }
//
//        // Read exactly 1024 bytes (the Amiga boot block)
//        if (!ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL)
//            || bytesRead != sizeof(buffer)) {
//            CloseHandle(hFile);
//            char err[512];
//            _snprintf_s(err, sizeof(err), _TRUNCATE,
//                "Unable to read 1024 bytes from:\n%s", gstrFileName);
//            MessageBoxA(dlg, err, "ADF Opus Error", MB_OK | MB_ICONSTOP);
//            EndDialog(dlg, TRUE);
//            return TRUE;
//        }
//        CloseHandle(hFile);
//
//        // Format and display
//        FormatHexDump(buffer, sizeof(buffer), dump, sizeof(dump));
//        SetDlgItemTextA(dlg, IDC_EDIT_BOOTBLOCK, dump);
//        return TRUE;
//    }
//
//    case WM_COMMAND:
//        WORD id = LOWORD(wp);
//        WORD ev = HIWORD(wp);
//
//        if (id == IDC_BOOTBLOCK_COPY && ev == BN_CLICKED)
//        {
//            CopyBootblockHexToClipboard(dlg);
//            return TRUE;
//        }
//
//        if (id == IDC_BOOTBLOCK_SAVETEXT && ev == BN_CLICKED)
//        {
//            //MessageBoxA(dlg, "Save as Text", "Temp:", MB_OK | MB_ICONSTOP);
//            SaveBootblockAsText(dlg);
//            return TRUE;
//        }
//
//        
//        if (id == IDC_BOOTBLOCK_SAVEBINARY && ev == BN_CLICKED)
//        {
//            //MessageBoxA(dlg, "Save as Binary", "Temp:", MB_OK | MB_ICONSTOP);
//            SaveBootblockAsBinary(dlg);
//            return TRUE;
//        }
//
//        
//
//
//
//        //if (id == IDC_BOOTBLOCK_HELP)
//        //{
//        //    WinHelp(dlg, "ADFOpus.hlp>Opus_win", HELP_CONTEXT, IDH_BOOTBLOCK);
//        //    return TRUE;
//        //}
//
//
//        if (id == IDOK)
//        {
//            EndDialog(dlg, TRUE);
//            return TRUE;
//        }
//
//        break;
//
//    case WM_CLOSE:
//        EndDialog(dlg, TRUE);
//        return TRUE;
//
//    case WM_HELP:
//        WinHelp(((LPHELPINFO)lp)->hItemHandle,
//            "adfopus.hlp",
//            HELP_WM_HELP,
//            (DWORD_PTR)aIds);
//        break;
//
//    case WM_CONTEXTMENU:
//        WinHelp((HWND)wp,
//            "adfopus.hlp",
//            HELP_CONTEXTMENU,
//            (DWORD_PTR)aIds);
//        break;
//    }
//
//    return FALSE;
//}
