#include "Pch.h"
#include "ADFOpus.h"
#include "TextViewer.h"
#include "ChildCommon.h"
#include "Utils.h"
#include "ListView.h"
#include "Help\\AdfOpusHlp.h"
#include <direct.h>

static DWORD aIds[] = {
    IDC_EDIT_TEXT,           IDH_TEXTVIEWER_EDIT,
    IDC_TEXTVIEWER_HELP,     IDH_TEXTVIEWER_BUTTON_HELP,
    IDOK,                    IDH_TEXTVIEWER_BUTTON_OK,
    0, 0
};

LRESULT CALLBACK TextViewerProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    static HBRUSH hBrush = NULL;
    char    buf[MAX_PATH];
    char    szError[MAX_PATH];
    char    lpFileName[MAX_PATH];
    FILE* fp;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        HWND hEdit = GetDlgItem(dlg, IDC_EDIT_TEXT);

        // Build the full path of the selected file
        int iIndex = LVGetItemFocused(ci->lv);
        LVGetItemCaption(ci->lv, buf, sizeof(buf), iIndex);

        switch (LVGetItemImageIndex(ci->lv, iIndex))
        {
        case ICO_WINFILE:
            strcpy(lpFileName, ci->curDir);
            strcat(lpFileName, buf);
            break;

        case ICO_AMIFILE:
            if (_chdir(dirTemp) == -1) {
                _mkdir(dirTemp);
                _chdir(dirTemp);
            }
            if (GetFileFromADF(ci->vol, buf) < 0) {
                sprintf(szError,
                    "Error extracting %s from %s.",
                    buf, ci->orig_path);
                MessageBox(dlg, szError, "ADF Opus Error",
                    MB_ICONSTOP);
                _chdir(dirOpus);
                EndDialog(dlg, TRUE);
                return TRUE;
            }
            strcpy(lpFileName, dirTemp);
            strcat(lpFileName, buf);
            _chdir(dirOpus);
            break;

        default:
            EndDialog(dlg, TRUE);
            return TRUE;
        }

        // Make the edit control multiline & scrollable
        {
            DWORD style = GetWindowLong(hEdit, GWL_STYLE);
            style |= ES_MULTILINE
                | ES_AUTOVSCROLL
                | ES_AUTOHSCROLL
                | WS_VSCROLL;
            SetWindowLong(hEdit, GWL_STYLE, style);
        }

        // Read file binary, expand lone '\n' to "\r\n"
        fp = fopen(lpFileName, "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            long fileSize = ftell(fp);
            rewind(fp);

            char* raw = malloc(fileSize + 1);
            if (raw)
            {
                fread(raw, 1, fileSize, fp);
                raw[fileSize] = '\0';
            }
            fclose(fp);

            if (raw)
            {
                char* conv = malloc(fileSize * 2 + 1);
                char* src = raw, * dst = conv;
                while (*src)
                {
                    if (*src == '\r') {
                        // skip stray CR
                        src++;
                    }
                    else if (*src == '\n') {
                        *dst++ = '\r';
                        *dst++ = '\n';
                        src++;
                    }
                    else {
                        *dst++ = *src++;
                    }
                }
                *dst = '\0';

                SetDlgItemText(dlg, IDC_EDIT_TEXT, conv);
                free(raw);
                free(conv);
            }
        }

        // Create background brush (if you paint the edit’s bg later)
        hBrush = CreateSolidBrush(RGB(0, 0, 255));

        return TRUE;
    }

    case WM_SIZE:
    {
        HWND hEdit = GetDlgItem(dlg, IDC_EDIT_TEXT);
        HWND hButtonOK = GetDlgItem(dlg, IDOK);
        HWND hButtonHelp = GetDlgItem(dlg, IDC_TEXTVIEWER_HELP);
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

        // Help button
        MoveWindow(
            hButtonHelp,
            nWidth - btnW - marginHelpR,
            nHeight - btnH - marginHelpB,
            btnW, btnH, TRUE
        );

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

        case IDC_TEXTVIEWER_HELP:
            //WinHelp(
            //    dlg,
            //    "ADFOpus.hlp",
            //    HELP_CONTEXT,
            //    IDH_TEXTVIEWER
            //);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        if (hBrush) {
            DeleteObject(hBrush);
            hBrush = NULL;
        }
        EndDialog(dlg, TRUE);
        return TRUE;

    case WM_HELP:
        //WinHelp(
        //    ((LPHELPINFO)lp)->hItemHandle,
        //    "ADFOpus.hlp",
        //    HELP_WM_HELP,
        //    (DWORD_PTR)aIds
        //);
        return TRUE;

    case WM_CONTEXTMENU:
        //WinHelp(
        //    (HWND)wp,
        //    "ADFOpus.hlp",
        //    HELP_CONTEXTMENU,
        //    (DWORD_PTR)aIds
        //);
        return TRUE;
    }

    return FALSE;
}