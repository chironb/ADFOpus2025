#ifndef MENUICONS_H
#define MENUICONS_H

#include <windows.h>

// Call this after LoadMenu/SetMenu
void    InitMenuIcons(HINSTANCE hInst, HMENU hMenu);

// Call this when you’re done with that popup (or on WM_DESTROY)
void    CleanupMenuIcons(void);

// Forward these from your WndProc
void    OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* mis);
void    OnDrawItem(HWND hWnd, DRAWITEMSTRUCT* dis);

#endif // MENUICONS_H