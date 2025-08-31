#pragma once
#include <windows.h>

// Call this on any HMENU (main frame or popup) after LoadMenu/GetSubMenu
void SetMenuBitmaps(HINSTANCE hInst, HMENU hMenu);