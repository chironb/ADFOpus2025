#include "Pch.h"
#include "ADFOpus.h"
#include "HexViewer.h"
#include "ChildCommon.h"
#include "Utils.h"
#include "ListView.h"
#include "Help\\AdfOpusHlp.h"
#include <direct.h>

static DWORD aIds[] = {
    IDC_EDIT_HEX,         
    IDOK,                   
    0, 0
};

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




LRESULT CALLBACK HexViewerProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    static HBRUSH hBrush = NULL;
    //char    buf[MAX_PATH];
    //char    szError[MAX_PATH];
    //char    lpFileName[MAX_PATH];
    //FILE* fp;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        HWND    hEdit = GetDlgItem(dlg, IDC_EDIT_HEX);
        char    buf[MAX_PATH];
        char    lpFileName[MAX_PATH];
        FILE* fp;
        long    fileSize;
        unsigned char* raw;
        char* dump;

        // --- 1) get the selected filename from your listview ---
        // assume you have a global "ci" pointer pointing at your CHILDINFO
        int iIndex = LVGetItemFocused(ci->lv);
        LVGetItemCaption(ci->lv, buf, sizeof(buf), iIndex);

        // build the full path (same logic as your text‐viewer)
        if (LVGetItemImageIndex(ci->lv, iIndex) == ICO_WINFILE) {
            strcpy_s(lpFileName, sizeof(lpFileName), ci->curDir);
            strcat_s(lpFileName, sizeof(lpFileName), buf);
        }
        else {
            // your ADF‐extraction path
            if (_chdir(dirTemp) == -1) {
                _mkdir(dirTemp);
                _chdir(dirTemp);
            }
            GetFileFromADF(ci->vol, buf);  // assume this always succeeds for debug
            strcpy_s(lpFileName, sizeof(lpFileName), dirTemp);
            strcat_s(lpFileName, sizeof(lpFileName), buf);
            _chdir(dirOpus);
        }

        //// --- 2) Debug: make sure the path is valid ---
        //MessageBoxA(dlg, lpFileName, "HexViewer Opening", MB_OK);

        // --- 3) Open & read the file ---
        fp = fopen(lpFileName, "rb");
        if (!fp) {
            MessageBoxA(dlg, "Unable to open file", "Error", MB_OK | MB_ICONERROR);
            EndDialog(dlg, TRUE);
            return TRUE;
        }

        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        rewind(fp);

        raw = malloc(fileSize);
        if (!raw || fread(raw, 1, fileSize, fp) != (size_t)fileSize) {
            MessageBoxA(dlg, "Unable to read file", "Error", MB_OK | MB_ICONERROR);
            free(raw);
            fclose(fp);
            EndDialog(dlg, TRUE);
            return TRUE;
        }
        fclose(fp);

        // --- 4) Hex‐dump it ---
        // allocate ~70 chars per 16‐byte line
        dump = malloc((fileSize / 16 + 1) * 80);
        if (!dump) {
            free(raw);
            EndDialog(dlg, TRUE);
            return TRUE;
        }

        FormatHexDump(raw, fileSize, dump, (fileSize / 16 + 1) * 80);
        SetDlgItemTextA(dlg, IDC_EDIT_HEX, dump);

        free(raw);
        free(dump);

        // make sure it’s multiline/scrollable
        {
            DWORD style = GetWindowLong(hEdit, GWL_STYLE);
            style |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
            SetWindowLong(hEdit, GWL_STYLE, style);
        }

        return TRUE;
    }

    case WM_SIZE:
    {
        HWND hEdit = GetDlgItem(dlg, IDC_EDIT_HEX);
        HWND hButtonOK = GetDlgItem(dlg, IDOK);
        //HWND hButtonHelp = GetDlgItem(dlg, IDC_TEXTVIEWER_HELP);
        RECT lpRect;

        int nWidth = LOWORD(lp);
        int nHeight = HIWORD(lp);

        // Recompute margins & sizes in dialog units
        lpRect = (RECT){ 0,0,7,6 };
        MapDialogRect(dlg, &lpRect);
        int marginOKR = lpRect.right, marginOKB = lpRect.bottom;

        lpRect = (RECT){ 0,0,63,6 };
        MapDialogRect(dlg, &lpRect);
        int marginHelpR = lpRect.right, marginHelpB = lpRect.bottom;

        lpRect = (RECT){ 0,0,7,28 };
        MapDialogRect(dlg, &lpRect);
        int marginEditR = lpRect.right, marginEditB = lpRect.bottom;

        lpRect = (RECT){ 0,0,50,14 };
        MapDialogRect(dlg, &lpRect);
        int btnW = lpRect.right, btnH = lpRect.bottom;

        // OK button
        MoveWindow(
            hButtonOK,
            nWidth - btnW - marginOKR,
            nHeight - btnH - marginOKB,
            btnW, btnH, TRUE
        );

        //// Help button
        //MoveWindow(
        //    hButtonHelp,
        //    nWidth - btnW - marginHelpR,
        //    nHeight - btnH - marginHelpB,
        //    btnW, btnH, TRUE
        //);

        // Edit control
        MoveWindow(
            hEdit,
            7, // left margin
            7, // top margin
            nWidth - marginEditR - 7,
            nHeight - marginEditB - 7,
            TRUE
        );
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wp))
        {
        case IDOK:
            EndDialog(dlg, TRUE);
            return TRUE;

        /*case IDC_TEXTVIEWER_HELP:
            WinHelp(
                dlg,
                "ADFOpus.hlp",
                HELP_CONTEXT,
                IDH_TEXTVIEWER
            );
            return TRUE;*/
        }
        break;

    case WM_CLOSE:
        if (hBrush) {
            DeleteObject(hBrush);
            hBrush = NULL;
        }
        EndDialog(dlg, TRUE);
        return TRUE;

    //case WM_HELP:
    //    WinHelp(
    //        ((LPHELPINFO)lp)->hItemHandle,
    //        "ADFOpus.hlp",
    //        HELP_WM_HELP,
    //        (DWORD_PTR)aIds
    //    );
    //    return TRUE;

    case WM_CONTEXTMENU:
        WinHelp(
            (HWND)wp,
            "ADFOpus.hlp",
            HELP_CONTEXTMENU,
            (DWORD_PTR)aIds
        );
        return TRUE;
    }

    return FALSE;
}