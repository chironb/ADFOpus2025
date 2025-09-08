#include "Pch.h"
#include "ADFOpus.h"
#include "ChildCommon.h"
#include <windows.h>
#include <shellapi.h>   // ShellExecuteA
#include <string.h>     // strrchr, strcpy_s, strcat_s, strncpy_s`
#include <winver.h>     // version‐info APIs

#pragma comment(lib, "Version.lib")  // link in GetFileVersionInfo* and VerQueryValue

extern HWND ghwndSB;

// About dialog procedure
LRESULT CALLBACK AboutDlgProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBmp = NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // 1) Load and display the bitmap graphic
        HINSTANCE hInst = GetModuleHandle(NULL);
        hBmp = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_NEW_ABOUT_GRAPHIC));
        if (hBmp)
        {
            SendDlgItemMessage(
                dlg,
                IDC_NEW_ABOUT_GRAPHIC,  // Static control ID
                STM_SETIMAGE,
                IMAGE_BITMAP,
                (LPARAM)hBmp
            );
        }

        // 2) Retrieve the FileVersion string from VERSIONINFO
        char versionStr[64] = "";
        char exePath[MAX_PATH] = { 0 };

        if (GetModuleFileNameA(NULL, exePath, MAX_PATH))
        {
            DWORD verHandle = 0;
            DWORD verSize = GetFileVersionInfoSizeA(exePath, &verHandle);
            if (verSize > 0)
            {
                BYTE* verData = (BYTE*)malloc(verSize);
                if (verData &&
                    GetFileVersionInfoA(exePath, verHandle, verSize, verData))
                {
                    struct LANGANDCODEPAGE {
                        WORD wLanguage;
                        WORD wCodePage;
                    } *lpTranslate = NULL;
                    UINT cbTranslate = 0;

                    if (VerQueryValueA(
                        verData,
                        "\\VarFileInfo\\Translation",
                        (LPVOID*)&lpTranslate,
                        &cbTranslate)
                        && cbTranslate >= sizeof(*lpTranslate))
                    {
                        char subBlock[64];
                        sprintf_s(
                            subBlock, sizeof(subBlock),
                            "\\StringFileInfo\\%04x%04x\\FileVersion",
                            lpTranslate[0].wLanguage,
                            lpTranslate[0].wCodePage
                        );

                        LPVOID lpBuffer = NULL;
                        UINT   dwLen = 0;
                        if (VerQueryValueA(
                            verData,
                            subBlock,
                            &lpBuffer,
                            &dwLen)
                            && dwLen > 0)
                        {
                            // Copy up to dwLen-1 chars
                            strncpy_s(versionStr, sizeof(versionStr),
                                (LPCSTR)lpBuffer, dwLen - 1);
                        }
                    }
                }
                free(verData);
            }
        }

        // 3) Build and set the bottom text: app name, version, build date/time
        char buildText[256];
        if (versionStr[0] != '\0')
        {
            sprintf_s(
                buildText, sizeof(buildText),
                "ADF Opus 2025 - v%s - Built: %s at %s",
                versionStr, __DATE__, __TIME__
            );
        }
        else
        {
            sprintf_s(
                buildText, sizeof(buildText),
                "ADF Opus 2025 - Built: %s at %s",
                __DATE__, __TIME__
            );
        }

        SetDlgItemTextA(dlg, IDC_STATIC_BUILD, buildText);

        SetWindowText(ghwndSB, "Visit ADF Opus 2025 on GitHub!"); // Needs: extern HWND ghwndSB;

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_CREDITS:
        {
            // Open Credits.txt next to the EXE
            char exeDir[MAX_PATH] = { 0 };
            char creditsPath[MAX_PATH] = { 0 };

            if (GetModuleFileNameA(NULL, exeDir, MAX_PATH))
            {
                // Keep only the directory
                char* slash = strrchr(exeDir, '\\');
                if (slash) *(slash + 1) = '\0';

                strcpy_s(creditsPath, sizeof(creditsPath), exeDir);
                strcat_s(creditsPath, sizeof(creditsPath), "Credits.txt");

                ShellExecuteA(dlg, "open", creditsPath, NULL, NULL, SW_SHOWNORMAL);
            }
            else
            {
                // fallback if path lookup fails
                ShellExecuteA(dlg, "open", "Credits.txt", NULL, NULL, SW_SHOWNORMAL);
            }

            return TRUE;

        }

        case IDC_BTN_WEBSITE:
            ShellExecuteA(
                dlg,
                "open",
                "https://github.com/chironb/ADFOpus2025",
                NULL,
                NULL,
                SW_SHOWNORMAL
            );
            return TRUE;

        case IDC_BTN_OK:
            // Clean up the bitmap and close
            if (hBmp)
            {
                DeleteObject(hBmp);
                hBmp = NULL;
            }
            EndDialog(dlg, IDOK);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        // Clean up on X click
        if (hBmp)
        {
            DeleteObject(hBmp);
            hBmp = NULL;
        }
        EndDialog(dlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;  // default processing
}