/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 */
/*! \file Init.c
 *  \brief Initialisation functions.
 *
 * Init.c - routines for registering window classes and creating windows, etc.
 */

#include "Pch.h"

#include "ADFOpus.h"
#include "ChildCommon.h"
#include "Init.h"

extern HINSTANCE instance;

/* prototypes */
void AddIcon(HIMAGELIST il, int id);

BOOL RegisterAppClass(HANDLE hInstance)
/* registers window classes */
{
	WNDCLASS wndClass;

	/* attributes common to all windows */
	wndClass.style = 0;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = sizeof(LONG);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hInstance = hInstance;

	/* the application main window */
	wndClass.lpfnWndProc = MainWinProc;
	wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wndClass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wndClass.lpszClassName = APPCLASSNAME;

	if(! RegisterClass(&wndClass))
		return FALSE;

	/* register the windows lister window */
	wndClass.lpfnWndProc = ChildWinProc;
	wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINLISTER));
	wndClass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = WINLISTERCLASSNAME;

	if(! RegisterClass(&wndClass))
	{
		UnregisterClass(APPCLASSNAME, hInstance);
		return FALSE;
	}

	/* register the amiga lister window */
	wndClass.lpfnWndProc = ChildWinProc;
	wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AMILISTER));
	wndClass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = AMILISTERCLASSNAME;

	if(! RegisterClass(&wndClass))
	{
		UnregisterClass(WINLISTERCLASSNAME, hInstance);
		UnregisterClass(APPCLASSNAME, hInstance);
		return FALSE;
	}

	return TRUE;
}

void UnregisterAllClasses(HANDLE hInstance)
/* unregisters all registered classes (part of the program cleanup) */
{
	UnregisterClass(AMILISTERCLASSNAME, hInstance);
	UnregisterClass(WINLISTERCLASSNAME, hInstance);
	UnregisterClass(APPCLASSNAME, hInstance);

	return;
}

HWND CreateAppWindow(HANDLE hInstance)
/* creates the application main window */
{
	char Title[64];
	LoadString(hInstance, IDS_VERSION, Title, sizeof(Title));

	return(CreateWindow(
		APPCLASSNAME,
		Title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		NULL,
		NULL,
		hInstance,
		NULL
	));
}

HWND CreateMDIClientWindow(HWND hwndFrame)
/* creates the client window that will handle the MDI child windows */
{
	HWND hwndClient;
	CLIENTCREATESTRUCT ccs;

    ccs.hWindowMenu = GetSubMenu(GetMenu(hwndFrame), 5);

	ccs.idFirstChild = IDM_WINDOWCHILD;

	hwndClient = CreateWindow(
		"MDICLIENT",
		NULL,
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 100,
		1, 1,
		hwndFrame,
		NULL,
		instance,
		(LPVOID)&ccs
	);

	ShowWindow(hwndClient, SW_SHOW);

	return(hwndClient);
}

void AddIcon(HIMAGELIST il, int id)
{
	HICON icon = LoadIcon(instance, MAKEINTRESOURCE(id));
	ImageList_AddIcon(il, icon);
	DeleteObject(icon);
}

HIMAGELIST CreateImageList()
/* creates an image list containing icons for the list-view control */
{
	HIMAGELIST il;
 
	/* create the image list */
	il = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
		GetSystemMetrics(SM_CYSMICON), ILC_COLOR16 | ILC_MASK, 11, 1);

	/* add the icons */
	AddIcon(il, IDI_WINFILE);
	AddIcon(il, IDI_WINDIR); 
	AddIcon(il, IDI_AMIFILE);
	AddIcon(il, IDI_AMIDIR);
	AddIcon(il, IDI_DRIVEHD);
	AddIcon(il, IDI_DRIVENET);
	AddIcon(il, IDI_DRIVECD);
	AddIcon(il, IDI_DRIVEFLOP35);
	AddIcon(il, IDI_DRIVEFLOP514);

	return il;
}

HWND CreateToolBar(HWND hwndParent)
{
	HWND hwndTB;
	TBADDBITMAP tbab;
	TBBUTTON tbb[25];
	COLORMAP cm;
	HANDLE hbm;
 
	hwndTB = CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		(LPSTR) NULL,
		WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT,
		0, 0,
		0, 0,
		hwndParent,
		(HMENU) ID_TOOLBAR,
		instance,
		NULL
	); 
 
	/* Send the TB_BUTTONSTRUCTSIZE message - this is required for
           the toolbar control to work */
	SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

	// Add the bitmap containing button images to the toolbar.
	cm.from = 0x00FF00FF;
	cm.to = GetSysColor(COLOR_BTNFACE);

	hbm = CreateMappedBitmap(instance, IDB_TBBUTTONS, 0, &cm, 1);
	tbab.hInst = NULL;
	tbab.nID = (UINT) hbm;
	SendMessage(hwndTB, TB_ADDBITMAP, (WPARAM) 17, (WPARAM) &tbab);
 
	// Fill the TBBUTTON array with button information, and add the
	// buttons to the toolbar.
	tbb[0].iBitmap = 0; 
	tbb[0].idCommand = ID_FIL_NEW; 
	tbb[0].fsState = TBSTATE_ENABLED; 
	tbb[0].fsStyle = TBSTYLE_BUTTON; 
	tbb[0].dwData = 0; 
	tbb[0].iString = 0; 
 
	tbb[1].iBitmap = 1;
	tbb[1].idCommand = ID_FIL_OPEN; 
	tbb[1].fsState = TBSTATE_ENABLED; 
	tbb[1].fsStyle = TBSTYLE_BUTTON; 
	tbb[1].dwData = 0; 
	tbb[1].iString = 0; 
 
	tbb[2].iBitmap = 2;
	tbb[2].idCommand = ID_FIL_CLOSE;
	tbb[2].fsState = TBSTATE_ENABLED;
	tbb[2].fsStyle = TBSTYLE_BUTTON;
	tbb[2].dwData = 0;
	tbb[2].iString = 0;

	tbb[3].iBitmap = 0;
	tbb[3].idCommand = 0;
	tbb[3].fsState = 0;
	tbb[3].fsStyle = TBSTYLE_SEP;
	tbb[3].dwData = 0;
	tbb[3].iString = 0;

	tbb[4].iBitmap = 3;
	tbb[4].idCommand = ID_ACTION_RENAME;
	tbb[4].fsState = TBSTATE_ENABLED;
	tbb[4].fsStyle = TBSTYLE_BUTTON;
	tbb[4].dwData = 0;
	tbb[4].iString = 0;

	tbb[5].iBitmap = 4;
	tbb[5].idCommand = ID_ACTION_DELETE;
	tbb[5].fsState = TBSTATE_ENABLED;
	tbb[5].fsStyle = TBSTYLE_BUTTON;
	tbb[5].dwData = 0;
	tbb[5].iString = 0;

	tbb[6].iBitmap = 5;
	tbb[6].idCommand = ID_ACTION_NEWDIRECTORY;
	tbb[6].fsState = TBSTATE_ENABLED;
	tbb[6].fsStyle = TBSTYLE_BUTTON;
	tbb[6].dwData = 0;
	tbb[6].iString = 0;

	tbb[7].iBitmap = 6;
	tbb[7].idCommand = ID_ACTION_UPONELEVEL;
	tbb[7].fsState = TBSTATE_ENABLED;
	tbb[7].fsStyle = TBSTYLE_BUTTON;
	tbb[7].dwData = 0;
	tbb[7].iString = 0;

	tbb[8].iBitmap = 0;
	tbb[8].idCommand = 0;
	tbb[8].fsState = 0;
	tbb[8].fsStyle = TBSTYLE_SEP;
	tbb[8].dwData = 0;
	tbb[8].iString = 0;

	tbb[9].iBitmap = 7;
	tbb[9].idCommand = ID_VIEW_SHOWUNDELETABLEFILES;
	tbb[9].fsState = TBSTATE_ENABLED;
	tbb[9].fsStyle = TBSTYLE_BUTTON;
	tbb[9].dwData = 0;
	tbb[9].iString = 0;

	tbb[10].iBitmap = 8;
	tbb[10].idCommand = ID_ACTION_UNDELETE;
	tbb[10].fsState = TBSTATE_ENABLED;
	tbb[10].fsStyle = TBSTYLE_BUTTON;
	tbb[10].dwData = 0;
	tbb[10].iString = 0;

	tbb[11].iBitmap = 0;
	tbb[11].idCommand = 0;
	tbb[11].fsState = 0;
	tbb[11].fsStyle = TBSTYLE_SEP;
	tbb[11].dwData = 0;
	tbb[11].iString = 0;

	tbb[12].iBitmap = 9;
	tbb[12].idCommand = ID_FIL_INFORMATION;
	tbb[12].fsState = TBSTATE_ENABLED;
	tbb[12].fsStyle = TBSTYLE_BUTTON;
	tbb[12].dwData = 0;
	tbb[12].iString = 0;

	tbb[13].iBitmap = 0;
	tbb[13].idCommand = 0;
	tbb[13].fsState = 0;
	tbb[13].fsStyle = TBSTYLE_SEP;
	tbb[13].dwData = 0;
	tbb[13].iString = 0;

	tbb[14].iBitmap = 10;
	tbb[14].idCommand = ID_ACTION_PROPERTIES;
	tbb[14].fsState = TBSTATE_ENABLED;
	tbb[14].fsStyle = TBSTYLE_BUTTON;
	tbb[14].dwData = 0;
	tbb[14].iString = 0;

	tbb[15].iBitmap = 0;
	tbb[15].idCommand = 0;
	tbb[15].fsState = 0;
	tbb[15].fsStyle = TBSTYLE_SEP;
	tbb[15].dwData = 0;
	tbb[15].iString = 0;

	tbb[16].iBitmap = 11;
	tbb[16].idCommand = ID_TOOLS_TEXT_VIEWER;
	tbb[16].fsState = TBSTATE_ENABLED;
	tbb[16].fsStyle = TBSTYLE_BUTTON;
	tbb[16].dwData = 0;
	tbb[16].iString = 0;

	tbb[17].iBitmap = 0;
	tbb[17].idCommand = 0;
	tbb[17].fsState = 0;
	tbb[17].fsStyle = TBSTYLE_SEP;
	tbb[17].dwData = 0;
	tbb[17].iString = 0;

	tbb[18].iBitmap = 12;
	tbb[18].idCommand = ID_TOOLS_BATCHCONVERTER;
	tbb[18].fsState = TBSTATE_ENABLED;
	tbb[18].fsStyle = TBSTYLE_BUTTON;
	tbb[18].dwData = 0;
	tbb[18].iString = 0;
	
	tbb[19].iBitmap = 13;
	tbb[19].idCommand = ID_TOOLS_DISK2FDI;
	tbb[19].fsState = TBSTATE_ENABLED;
	tbb[19].fsStyle = TBSTYLE_BUTTON;
	tbb[19].dwData = 0;
	tbb[19].iString = 0;

	tbb[20].iBitmap = 0;
	tbb[20].idCommand = 0;
	tbb[20].fsState = 0;
	tbb[20].fsStyle = TBSTYLE_SEP;
	tbb[20].dwData = 0;
	tbb[20].iString = 0;

	tbb[21].iBitmap = 14;
	tbb[21].idCommand = ID_TOOLS_INSTALL;
	tbb[21].fsState = TBSTATE_ENABLED;
	tbb[21].fsStyle = TBSTYLE_BUTTON;
	tbb[21].dwData = 0;
	tbb[21].iString = 0;

	tbb[22].iBitmap = 15;
	tbb[22].idCommand = ID_TOOLS_DISPLAYBOOTBLOCK;
	tbb[22].fsState = TBSTATE_ENABLED;
	tbb[22].fsStyle = TBSTYLE_BUTTON;
	tbb[22].dwData = 0;
	tbb[22].iString = 0;
	
	tbb[23].iBitmap = 0;
	tbb[23].idCommand = 0;
	tbb[23].fsState = 0;
	tbb[23].fsStyle = TBSTYLE_SEP;
	tbb[23].dwData = 0;
	tbb[23].iString = 0;

	tbb[24].iBitmap = 16;
	tbb[24].idCommand = ID_TOOLS_OPTIONS;
	tbb[24].fsState = TBSTATE_ENABLED;
	tbb[24].fsStyle = TBSTYLE_BUTTON;
	tbb[24].dwData = 0;
	tbb[24].iString = 0;

	SendMessage(hwndTB, TB_ADDBUTTONS, (WPARAM) 25, (LPARAM) (LPTBBUTTON) &tbb);
	SendMessage(hwndTB, TB_AUTOSIZE, 0, 0);

	ShowWindow(hwndTB, SW_SHOW);
	return hwndTB;
}

HWND CreateStatusBar(HWND hwndParent)
{ 
	HWND hwndSB;
 
	hwndSB = CreateWindowEx(
		0,
		STATUSCLASSNAME,
		(LPCTSTR) NULL,
		SBARS_SIZEGRIP | WS_CHILD,
		0, 0, 0, 0,
		hwndParent,
		(HMENU) ID_STATUSBAR,
		instance,
		NULL
	);

	ShowWindow(hwndSB, SW_SHOW);

	return hwndSB; 
} 
