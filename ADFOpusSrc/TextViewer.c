// TextViewer.c

#include <windows.h>
#include <direct.h>
#include "Pch.h"
#include "ADFOpus.h"
#include "TextViewer.h"
#include "ChildCommon.h"
#include "Utils.h"
#include "ListView.h"
#include "Help\\AdfOpusHlp.h"

// Global state
static char* g_textBuf = NULL;   // full file text (with real CR+LFs)
static HWND   g_hEdit = NULL;   // dynamic EDIT control
static HFONT  g_hFont = NULL;   // custom font
static HBRUSH g_hBkgBrush = NULL;   // custom background brush

LRESULT CALLBACK TextViewerProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    char   fileName[MAX_PATH], err[MAX_PATH], fullPath[MAX_PATH];
    FILE* fp;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // 1) Build full path
        int idx = LVGetItemFocused(ci->lv);
        LVGetItemCaption(ci->lv, fileName, sizeof(fileName), idx);

        if (LVGetItemImageIndex(ci->lv, idx) == ICO_AMIFILE)
        {
            if (_chdir(dirTemp) == -1) { _mkdir(dirTemp); _chdir(dirTemp); }
            if (GetFileFromADF(ci->vol, fileName) < 0)
            {
                sprintf(err, "Error extracting %s from %s.",
                    fileName, ci->orig_path);
                MessageBoxA(dlg, err, "ADF Opus Error", MB_ICONERROR);
                _chdir(dirOpus);
                EndDialog(dlg, TRUE);
                return TRUE;
            }
            strcpy(fullPath, dirTemp);
            strcat(fullPath, fileName);
            _chdir(dirOpus);
        }
        else
        {
            strcpy(fullPath, ci->curDir);
            strcat(fullPath, fileName);
        }

        // 2) Read file into raw[]
        fp = fopen(fullPath, "rb");
        if (!fp)
        {
            MessageBoxA(dlg, "Unable to open file.", "Error", MB_ICONERROR);
            EndDialog(dlg, TRUE);
            return TRUE;
        }
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        rewind(fp);
        char* raw = malloc(size + 1);
        fread(raw, 1, size, fp);
        raw[size] = '\0';
        fclose(fp);

        // 3) Build g_textBuf with real CR+LFs only
        g_textBuf = malloc(size * 2 + 2);
        {
            char* s = raw, * d = g_textBuf;
            while (*s)
            {
                if (*s == '\r')
                    s++;
                else if (*s == '\n')
                {
                    *d++ = '\r';
                    *d++ = '\n';
                    s++;
                }
                else
                    *d++ = *s++;
            }
            *d = '\0';
        }
        free(raw);

        // 4) Remove design-time EDITTEXT and get its rectangle
        HWND hOld = GetDlgItem(dlg, IDC_EDIT_TEXT);
        RECT rc;
        GetWindowRect(hOld, &rc);
        MapWindowPoints(NULL, dlg, (LPPOINT)&rc, 2);
        DestroyWindow(hOld);

        // 5) Create the EDIT (unwrapped by default)
        {
            DWORD style = WS_CHILD | WS_VISIBLE
                | WS_BORDER | WS_TABSTOP
                | ES_LEFT | ES_MULTILINE
                | ES_READONLY | ES_AUTOVSCROLL
                | WS_VSCROLL | WS_HSCROLL;

            g_hEdit = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "EDIT", NULL,
                style,
                rc.left, rc.top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                dlg,
                (HMENU)(INT_PTR)IDC_EDIT_TEXT,
                GetModuleHandle(NULL),
                NULL
            );
        }

        // 6) Create custom font (Courier New, 10pt) and background brush
        {
            HDC hdc = GetDC(dlg);
            LOGFONT lf = { 0 };
            lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
            strcpy(lf.lfFaceName, "Courier New");
            g_hFont = CreateFontIndirect(&lf);
            ReleaseDC(dlg, hdc);

            g_hBkgBrush = CreateSolidBrush(RGB(255, 255, 224));
            SendMessageA(g_hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        }

        // 7) Load text, focus edit, init checkbox
        SetWindowTextA(g_hEdit, g_textBuf);
        SetFocus(g_hEdit);
        HideCaret(g_hEdit);
        SendDlgItemMessage(dlg, IDC_WORD_WRAP,
            BM_SETCHECK, BST_UNCHECKED, 0);
        return TRUE;
    }

    case WM_SIZE:
    {
        int w = LOWORD(lp), h = HIWORD(lp);

        HWND btnOK = GetDlgItem(dlg, IDOK);
        HWND btnHelp = GetDlgItem(dlg, IDC_TEXTVIEWER_HELP);
        HWND chkWrap = GetDlgItem(dlg, IDC_WORD_WRAP);

        RECT rc;
        // button size
        rc = (RECT){ 0,0,50,14 };
        MapDialogRect(dlg, &rc);
        int bw = rc.right, bh = rc.bottom;

        // OK margins
        rc = (RECT){ 0,0,7,6 };
        MapDialogRect(dlg, &rc);
        int mOKR = rc.right, mOKB = rc.bottom;

        // Help margins
        rc = (RECT){ 0,0,63,6 };
        MapDialogRect(dlg, &rc);
        int mHR = rc.right, mHB = rc.bottom;

        // Edit margins
        rc = (RECT){ 0,0,7,28 };
        MapDialogRect(dlg, &rc);
        int mER = rc.right, mEB = rc.bottom;

        // Wrap checkbox position & size
        rc = (RECT){ 0,0,11,0 };
        MapDialogRect(dlg, &rc);
        int mWrapL = rc.right;

        rc = (RECT){ 0,0,0,9 };
        MapDialogRect(dlg, &rc);
        int mWrapB = rc.bottom;

        rc = (RECT){ 0,0,52,10 };
        MapDialogRect(dlg, &rc);
        int wWrap = rc.right, hWrap = rc.bottom;

        // Move OK
        MoveWindow(btnOK,
            w - bw - mOKR,
            h - bh - mOKB,
            bw, bh, TRUE);

        // Move Help
        MoveWindow(btnHelp,
            w - bw - mHR,
            h - bh - mHB,
            bw, bh, TRUE);

        // Move Wrap checkbox
        MoveWindow(chkWrap,
            mWrapL,
            h - hWrap - mWrapB,
            wWrap, hWrap, TRUE);

        // Move Edit
        MoveWindow(g_hEdit,
            7, 7,
            w - mER - 7,
            h - mEB - 7,
            TRUE);

        return TRUE;
    }

    case WM_MOUSEWHEEL:
        SendMessage(g_hEdit, WM_MOUSEWHEEL, wp, lp);
        return TRUE;

    case WM_CTLCOLOREDIT:
    {
        HDC  hdc = (HDC)wp;
        HWND hwndEd = (HWND)lp;
        if (hwndEd == g_hEdit)
        {
            SetTextColor(hdc, RGB(0, 0, 128));
            SetBkColor(hdc, RGB(255, 255, 224));
            return (INT_PTR)g_hBkgBrush;
        }
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wp) == IDC_WORD_WRAP && HIWORD(wp) == BN_CLICKED)
        {
            BOOL wrapOn = (IsDlgButtonChecked(dlg, IDC_WORD_WRAP) == BST_CHECKED);

            // Save and destroy old EDIT
            RECT r;
            GetWindowRect(g_hEdit, &r);
            MapWindowPoints(NULL, dlg, (LPPOINT)&r, 2);
            DestroyWindow(g_hEdit);

            // Build new style
            DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP
                | ES_LEFT | ES_MULTILINE | ES_READONLY
                | ES_AUTOVSCROLL | WS_VSCROLL
                | (wrapOn ? 0 : WS_HSCROLL);

            // Recreate EDIT
            g_hEdit = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "EDIT", NULL,
                style,
                r.left, r.top,
                r.right - r.left,
                r.bottom - r.top,
                dlg,
                (HMENU)(INT_PTR)IDC_EDIT_TEXT,
                GetModuleHandle(NULL),
                NULL
            );

            // Restore font & text
            SendMessageA(g_hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SetWindowTextA(g_hEdit, g_textBuf);
            SetFocus(g_hEdit);
            HideCaret(g_hEdit);

            return TRUE;
        }

        switch (LOWORD(wp))
        {
        case IDOK:
            EndDialog(dlg, TRUE);
            return TRUE;

        case IDC_TEXTVIEWER_HELP:
            // WinHelp(dlg, "ADFOpus.hlp", HELP_CONTEXT, IDH_TEXTVIEWER);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        if (g_hFont) { DeleteObject(g_hFont);     g_hFont = NULL; }
        if (g_hBkgBrush) { DeleteObject(g_hBkgBrush); g_hBkgBrush = NULL; }
        if (g_textBuf) { free(g_textBuf);           g_textBuf = NULL; }
        EndDialog(dlg, TRUE);
        return TRUE;
    }

    return FALSE;
}