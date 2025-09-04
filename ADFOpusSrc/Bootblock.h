// somewhere before your switch → case ID_TOOLS_WRITE_RAW_BOOTBLOCK:
// e.g. put in Bootblock.h (and #include "Bootblock.h" in ADFOpus.c)

#ifndef BOOTBLOCK_H
#define BOOTBLOCK_H

#include <windows.h>      // for HWND, BOOL
#include "adf_raw.h"      // for struct Volume

// Writes the 1 KB raw bootblock back into 'vol'.
// hwndParent: parent window for any messages.
// vol:         the in‐memory Volume* you’re working on.
// showMsg:     if TRUE pop up success/failure dialogs.
void RawWriteBootBlock(HWND hwndParent,
    struct Volume* vol,
    BOOL showMsg);

// Bootblock display dialog proc
LRESULT CALLBACK DisplayBootblockProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp);

#endif // BOOTBLOCK_H