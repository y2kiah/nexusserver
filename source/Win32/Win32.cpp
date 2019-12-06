/*----==== WIN32.CPP ====----
	Author:		Jeff Kiah
	Orig.Date:	10/26/2010
	Rev.Date:	9/22/2011
---------------------------*/

#include "Win32.h"
#include <shlobj.h>
#include <sstream>
#include <tchar.h>
#include "../Utility/Typedefs.h"
//#include "Event/EventManager.h"

///// DEFINITIONS /////

#define CONSOLE_X			1700	// default location of the debug console window
#define CONSOLE_Y			20
#define CONSOLE_MAX_LINES	2000
#define RESX				320
#define RESY				240

///// FUNCTIONS /////

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Win32::instance().wndProc(hwnd, msg, wParam, lParam);
}

INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Win32::instance().about(hDlg, msg, wParam, lParam);
}

////////// class Win32 //////////

bool Win32::isOnlyInstance()
{
	// use a random GUID for the named mutex of this app
	hMutex = CreateMutex(NULL, TRUE, TEXT("BE900190-4403-4669-B84D-43C6ED3A9CC6"));
	if (GetLastError() != ERROR_SUCCESS) {
		HWND hwnd = FindWindow(className, appName);
		if (hwnd) {
			ShowWindow(hwnd, SW_SHOWNORMAL);
			SetFocus(hwnd);
			SetForegroundWindow(hwnd);
			SetActiveWindow(hwnd);
			return false;
		}
	}
	return true;
}

void Win32::showErrorBox(const TCHAR *message)
{
	std::wostringstream oss;
	oss << message << TEXT(" (error ") << GetLastError() << TEXT(")");
	MessageBox((hWnd?hWnd:NULL), oss.str().c_str(), TEXT("Error"), MB_OK | MB_ICONERROR | MB_TOPMOST);
}

void Win32::setActive(bool active)
{
	mActive = active;
}

bool Win32::initWindow()
{
	WNDCLASSEX	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;

	// create WindowClass
    wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInst;
	wc.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_NEXUSSERVER));
	wc.hIconSm			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
	wc.hCursor			= LoadCursor(0, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= MAKEINTRESOURCE(IDC_NEXUSSERVER);
	wc.lpszClassName	= className;    

	if (!RegisterClassEx(&wc)) {
		showErrorBox(TEXT("Failed to register the window class"));
		return false;
	}
	mInitFlags[INIT_WNDCLASS] = true; // set the init flag

	// set windowed mode settings
	dwExStyle = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW;
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// find the window RECT size based on desired client area resolution
	RECT wRect = {0, 0, RESX, RESY};
	if (!getWindowSize(wRect, dwStyle, dwExStyle)) {
		showErrorBox(TEXT("Window sizing error"));
		return false;
	}
	int wWidth = wRect.right;
	int wHeight = wRect.bottom;

	// create new window
	hWnd = CreateWindowEx(	dwExStyle,
							className,
							appName,
							dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							wWidth,
							wHeight,
							0,
							0,
							hInst,
							0);
	if (!hWnd) {
		showErrorBox(TEXT("Window creation error"));
		return false;
	}
	mInitFlags[INIT_WINDOW] = true; // set the init flag

	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetFocus(hWnd);
	
	if (!addNotifyIcon(1, APPWM_TRAYICON, TEXT("Nexus Server"))) {
		showErrorBox(TEXT("Notification icon creation failed"));
		return false;
	}

	return true;
}

void Win32::killWindow()
{
	removeTrayIcon();

	if (hWnd && mInitFlags[INIT_WINDOW] && !mInitFlags[DEINIT_WINDOW]) {
		if (!DestroyWindow(hWnd)) {
			showErrorBox(TEXT("Could not release hWnd"));
		}
		hWnd = 0;
		mInitFlags[DEINIT_WINDOW] = true; // set the deinit flag
	}

	if (hInst && mInitFlags[INIT_WNDCLASS] && !mInitFlags[DEINIT_WNDCLASS]) {
		if (!UnregisterClass(className, hInst)) {
			showErrorBox(TEXT("Could not unregister class"));
		}
		hInst = 0;
		mInitFlags[DEINIT_WNDCLASS] = true; // set the deinit flag
	}
}

BOOL Win32::addNotifyIcon(UINT uID, UINT callbackMessage, const TCHAR *tooltip)
{
	ZeroMemory(&tnd, sizeof(tnd));

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = hWnd;
	tnd.uID = uID;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = callbackMessage;
	tnd.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NEXUSSERVER));
	_tcscpy_s(tnd.szTip, tooltip);

	return Shell_NotifyIcon(NIM_ADD, &tnd);
}

void Win32::removeTrayIcon()
{
	Shell_NotifyIcon(NIM_DELETE, &tnd);
}

bool Win32::getWindowSize(RECT &outRect, DWORD dwStyle, DWORD dwExStyle) const
{
	// find the window RECT size based on desired client area resolution
	if (!AdjustWindowRectEx(&outRect, dwStyle, FALSE, dwExStyle)) {
		return false;
	}
	// pass back the size in right, bottom
	outRect.right = outRect.right - outRect.left;
	outRect.bottom = outRect.bottom - outRect.top;
	outRect.left = outRect.top = 0;
	return true;
}

bool Win32::initInputDevices()
{
	// find number of mouse buttons
	int mouseNumBtns = GetSystemMetrics(SM_CMOUSEBUTTONS); // set some persistent value
	if (mouseNumBtns == 0) { // check for mouse existence
		showErrorBox(TEXT("No mouse detected, closing!"));
		return false;
	}
	// find if mouse has a vertical scroll wheel
	bool mouseHasWheel = (GetSystemMetrics(SM_MOUSEWHEELPRESENT) != 0); // set some persistent value

	// get keyboard type information
	int kbType = GetKeyboardType(0); // type (1-7), set some persistent value
	if (!kbType) {
		showErrorBox(TEXT("No keyboard detected, closing!"));
		return false;
	}
	int kbNumFuncKeys = GetKeyboardType(2); // number of function keys, set some persistent value

	hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_NEXUSSERVER));

	debugPrintf("Keyboard type:%i   func.keys:%i\n", kbType, kbNumFuncKeys);
	debugPrintf("Mouse detected: btns:%i   wheel:%i\n", mouseNumBtns, mouseHasWheel);
	return true;
}

bool Win32::getSystemInformation()
{
	// test the Windows OS version (XP or greater to pass)
	DWORD version = GetVersion();
	DWORD majorVersion = LOBYTE(LOWORD(version));
	DWORD minorVersion = HIBYTE(LOWORD(version));
	DWORD build = 0; if (version < 0x80000000) { build = HIWORD(version); }
	debugPrintf("Windows version %d.%d.%d\n", majorVersion, minorVersion, build);
	if (majorVersion < 5 || (majorVersion == 5 && minorVersion < 1)) {
		MessageBox((hWnd?hWnd:NULL), TEXT("This application is not supported on this version of Windows.\nThe minimum required version is 5.1 (Windows XP)."),
			TEXT("Error"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	// Get the application data path
	TCHAR appDataPath[MAX_PATH]; // this should be persistent

	// This is for Windows XP - for Vista and 7 this wraps a
	// call to SHGetKnownFolderPath(FOLDERID_ProgramData, ...)
	//if (version < 6) {
	if (SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath) != S_OK) {
		MessageBox((hWnd?hWnd:NULL), TEXT("The application data path could not be retrieved!"),
			TEXT("Error"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	// For Vista/7 we use SHGetKnownFolderPath instead
	//} else {
	//	PWSTR *ppszPath;
	//	SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_NO_ALIAS, NULL, ppszPath);
	//	// memcpy ppszPath into appDataPath	
	//	CoTaskMemFree(ppszPath);
	//}
	debugTPrintf("Save path: \"%s\"\n", appDataPath);

	return true;
}

LRESULT Win32::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CLOSE: {
			if (mCloseMeansClose) {
				killWindow(); // calls DestroyWindow
			} else {
				ShowWindow(hWnd, SW_HIDE); // Hide the main window
				HWND hWnd_c = GetConsoleWindow();
				ShowWindow(hWnd_c, SW_HIDE); // Hide the console window
			}
			break; }

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		// handle deactivate and non mouse-click activate
		case WM_ACTIVATE: {
			WORD activeFlag = LOWORD(wParam);
			WORD minimized = HIWORD(wParam);
			if (activeFlag == WA_INACTIVE || minimized) { // lost focus
				setActive(false);
			// gained focus not by mouse click, was not active and not minimized
			} else if (activeFlag == WA_ACTIVE && !isActive() && !minimized) {
				setActive(true);
			}
			break;
		}
		// handle mouse-click activate
		case WM_MOUSEACTIVATE: {
			setActive(true);
			// if the mouse caused activation, if click was in the client area
			// hide/clip the cursor, otherwise leave OS cursor visible
			if (LOWORD(lParam) == HTCLIENT) {
				return MA_ACTIVATEANDEAT;
			}
			// if user clicks the title bar or border or buttons, don't take the cursor
			return MA_ACTIVATE;
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			//return DefWindowProc(hwnd, msg, wParam, lParam);
			break; }

		// Handle Mouse Input
		case WM_LBUTTONDOWN:
			break;
		case WM_LBUTTONUP:
			break;
		case WM_RBUTTONDOWN:
			break;
		case WM_RBUTTONUP:
			break;
		case WM_MBUTTONDOWN:
			break;
		case WM_MBUTTONUP:
			break;
		case WM_XBUTTONDOWN: {
			int fwButton = GET_XBUTTON_WPARAM(wParam);
			break;
		}
		case WM_XBUTTONUP: {
			int fwButton = GET_XBUTTON_WPARAM(wParam);
			break;
		}
		case WM_MOUSEWHEEL: {
			int fwKeys = GET_KEYSTATE_WPARAM(wParam);
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			break;
		}
		case WM_MOUSEHWHEEL: {
			int fwKeys = GET_KEYSTATE_WPARAM(wParam);
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			break;
		}

		// WM_MOUSEMOVE is used for cursor control because pointer ballistics apply.
		// It is not used for high-precision first-person camera or similar control.
		case WM_MOUSEMOVE: {
			int posX = LOWORD(lParam);
			int posY = HIWORD(lParam);
			break;
		}

		// Handle Keyboard Input
		case WM_CHAR:
		case WM_KEYUP:
		case WM_KEYDOWN: {
			// keyboard snooper
			uint extended	= ((HIWORD(lParam) & KF_EXTENDED) != 0);
			uint altDown	= ((HIWORD(lParam) & KF_ALTDOWN) != 0);
			uint repeat		= ((HIWORD(lParam) & KF_REPEAT) != 0);
			uint up			= ((HIWORD(lParam) & KF_UP) != 0);
			uint repeatCnt	= LOWORD(lParam);
			uint vsc		= LOBYTE(HIWORD(lParam));
			uint vk			= MapVirtualKey(vsc, MAPVK_VSC_TO_VK_EX);
			
			switch (wParam) {
				case VK_ESCAPE:
					break;
			}
			break;
		}

        case WM_INITMENUPOPUP:
            //OnInitMenuPopup( hWnd, (HMENU)wParam, lParam );
            return 0;

		case WM_COMMAND: {
			int wmId    = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			if (!onCommand(hWnd, wmId, wmEvent, (HWND)lParam)) {
				DefWindowProc(hWnd, msg, wParam, lParam);
			}
			break; }

		case WM_SYSCOMMAND:
			switch (wParam & 0xFFF0) {
				// these are ignored and not handled
				case SC_SCREENSAVE:
					// Microsoft Windows Vista and later: If password protection is enabled by policy,
					// the screen saver is started regardless of what an application does with the
					// SC_SCREENSAVE notification even if fails to pass it to DefWindowProc.
				case SC_MONITORPOWER:
				case SC_HSCROLL:
				case SC_VSCROLL:
				case SC_KEYMENU:
				case SC_HOTKEY:
				case SC_CONTEXTHELP:
					break;
				// all others use the default handler
				default:
					return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			break;

		// Application-defined messages (for system tray menu)
		// see http://everything2.com/title/Win32+system+tray+icon
		case APPWM_NOP:
			break;

		case APPWM_TRAYICON:
			//SetForegroundWindow(hWnd);
			switch (lParam) {
				case WM_MOUSEMOVE:
					break;

				case WM_RBUTTONUP:
					SetForegroundWindow(hWnd);
					showPopupMenu(0);
					PostMessage(hWnd, APPWM_NOP, 0, 0);

					break;

				case WM_LBUTTONDBLCLK:
					PostMessage(hWnd, WM_COMMAND, ID_SHOW, 0);
					break;
			}
			break;


		// ignore these messages (return 0)
		case WM_CREATE:
		case WM_APPCOMMAND:

		// all other messages use default handler
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

// Message handler for about box
INT_PTR Win32::about(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (msg) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

bool Win32::initConsole()
{
	BOOL result = AllocConsole();
	if (result) {
		result = SetConsoleCtrlHandler((PHANDLER_ROUTINE)Win32::ctrlHandler, TRUE);
		if (result) {
			FILE *stream;
			freopen_s(&stream, "CONOUT$", "w", stdout);
			printf("Debugging console (Press CTRL-C to close)\n-----------------------------------------\n\n");
		} else {
			showErrorBox(TEXT("Could not set console handler"));
		}
	} else {
		showErrorBox(TEXT("Could not create debug console window"));
	}
	bool success = (result != 0);
	if (success) {
		HWND hWnd_c = GetConsoleWindow();
		SetWindowPos(hWnd_c, 0, CONSOLE_X, CONSOLE_Y,
					 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		// set max number of lines in console
		CONSOLE_SCREEN_BUFFER_INFO sbInfo;
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(hOut, &sbInfo);
		sbInfo.dwSize.Y = CONSOLE_MAX_LINES;
		SetConsoleScreenBufferSize(hOut, sbInfo.dwSize);
	}
	return success;
}

BOOL WINAPI Win32::ctrlHandler(DWORD dwCtrlType) 
{
	switch (dwCtrlType) {
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
			FreeConsole();
			return TRUE;

		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			Win32::instance().mCloseMeansClose = true;
			PostMessage(Win32::instance().hWnd, WM_CLOSE, 0, 0);
			return TRUE;

		default:
			return FALSE;
	}
}

// Create and display our little popupmenu when the user right-clicks on the system tray
BOOL Win32::showPopupMenu(POINT *curpos)
{
	HMENU hPop = CreatePopupMenu();

	POINT pt;
	if (!curpos) {
		GetCursorPos(&pt);
		curpos = &pt;
	}

	InsertMenu(hPop, 0, MF_BYPOSITION | MF_STRING, ID_START, TEXT("Start"));
	InsertMenu(hPop, 1, MF_BYPOSITION | MF_STRING, ID_STOP, TEXT("Stop"));
	InsertMenu(hPop, 2, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
	InsertMenu(hPop, 3, MF_BYPOSITION | MF_STRING, ID_SHOW, TEXT("Show Window"));
	InsertMenu(hPop, 4, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
	InsertMenu(hPop, 5, MF_BYPOSITION | MF_STRING, ID_EXIT, TEXT("Exit"));

	//SetMenuDefaultItem(hPop, ID_EXIT, FALSE);

	SetFocus(hWnd);

	SendMessage(hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0);

	WORD cmd = TrackPopupMenu(hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
							  curpos->x, curpos->y, 0, hWnd, NULL);

	SendMessage(hWnd, WM_COMMAND, cmd, 0);
	DestroyMenu(hPop);

	return cmd;
}

// Handle all application-defined menu events
BOOL Win32::onCommand(HWND hWnd, WORD wmID, WORD wmEvent, HWND hCtl)
{
	// Parse the menu selections
	switch (wmID) {
		case ID_START:
			break;

		case ID_STOP:
			break;

		case ID_SHOW: {
			ShowWindow(hWnd, SW_SHOWNORMAL); // Show the main window
			HWND hWnd_c = GetConsoleWindow();
			ShowWindow(hWnd_c, SW_SHOWNORMAL); // Show the console window
			SetFocus(hWnd);
			break; }

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case ID_EXIT:
		case IDM_EXIT:
			killWindow();
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

// Constructor / destructor
Win32::Win32(const TCHAR *name, HINSTANCE hInstance) :
	Singleton<Win32>(*this),
	mActive(true),
	mAppHasMouse(false),
	mCloseMeansClose(false),
	mInitFlags(0),
	hInst(hInstance),
	hWnd(0),
	hMutex(0),
	appName(name),
	className(TEXT("NEXUSSERVER"))
{}

Win32::~Win32()
{
	killWindow();
	if (hMutex) ReleaseMutex(hMutex);
}
