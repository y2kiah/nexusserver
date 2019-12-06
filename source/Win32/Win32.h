/*----==== WIN32.H ====----
	Author:		Jeff Kiah
	Orig.Date:	12/01/2007
	Rev.Date:	07/09/2009
-------------------------*/

#pragma once

#include "../targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN	// defined in project settings
#endif

#include <Windows.h>
#include <ShellAPI.h>
#include <bitset>
#include "../resource.h"
#include "../Utility/Singleton.h"

using std::bitset;

///// DECLARATIONS /////

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

///// STRUCTURES /////

/*=============================================================================
class Win32
=============================================================================*/
class Win32 : public Singleton<Win32> {
	private:
		bool	mActive;		// true if this is the active window
		bool	mAppHasMouse;	// true if app has mouse control, false if Windows cursor visible
		bool	mCloseMeansClose;	// Close message normally hides window, set this to true and it will call killWindow instead

		// initialization flags
		enum Win32InitFlags {
			INIT_WINDOW = 0,	DEINIT_WINDOW,
			INIT_WNDCLASS,		DEINIT_WNDCLASS,
			INIT_MAX
		};
		bitset<INIT_MAX>	mInitFlags;

	public:
		enum {
			// Application Messages
			APPWM_TRAYICON = WM_APP,
			APPWM_NOP = WM_APP + 1,
			// Menu Commands
			ID_START = 2000,
			ID_STOP,
			ID_SHOW,
			ID_EXIT
		};

		///// VARIABLES /////

		HINSTANCE	hInst;		
		HWND		hWnd;
		HANDLE		hMutex;
		HACCEL		hAccelTable;
		NOTIFYICONDATA tnd;

		const TCHAR	*appName;
		const TCHAR	*className;

		///// FUNCTIONS /////

		// misc functions
		bool	isOnlyInstance();
		bool	isActive() const	{ return mActive; }
		void	setActive(bool active = true);	// set and unset app active status
		void	showErrorBox(const TCHAR *message);

		// main window
		bool	initWindow();
		void	killWindow();
		BOOL	addNotifyIcon(UINT uID, UINT callbackMessage, const TCHAR *tooltip);
		void	removeTrayIcon();
		bool	getWindowSize(RECT &outRect, DWORD dwStyle, DWORD dwExStyle) const;
		bool	initInputDevices();
		bool	getSystemInformation();
		LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		// about window
		INT_PTR about(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

		// debug console
		bool	initConsole();
		static BOOL WINAPI	ctrlHandler(DWORD dwCtrlType);

		// application menu commands
		BOOL	showPopupMenu(POINT *curpos);
		BOOL	onCommand(HWND hWnd, WORD wmId, WORD wmEvent, HWND hCtl); // Handle all application-defined menu events

		// static functions
		//static void	showErrorBox(const TCHAR *message);

		// Constructor / Destructor
		explicit Win32(const TCHAR *name, HINSTANCE hInstance);
		~Win32();
};