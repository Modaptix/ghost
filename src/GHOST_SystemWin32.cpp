/**
 * $Id$
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/**
 * $Id$
 * Copyright (C) 2001 NaN Technologies B.V.
 * @author	Maarten Gribnau
 * @date	May 7, 2001
 */

#include "GHOST_SystemWin32.h"
#include "GHOST_EventDragnDrop.h"

#include <stdio.h> // for debug [mce]

#define _WIN32_IE 0x501 /* shipped before XP, so doesn't impose additional requirements */
#include <shlobj.h>


// win64 doesn't define GWL_USERDATA
#ifdef WIN32
  #ifndef GWL_USERDATA
    #define GWL_USERDATA GWLP_USERDATA
    #define GWL_WNDPROC GWLP_WNDPROC
  #endif
#endif

#include "GHOST_Debug.h"
#include "GHOST_DisplayManagerWin32.h"
#include "GHOST_EventButton.h"
#include "GHOST_EventCursor.h"
#include "GHOST_EventKey.h"
#include "GHOST_EventWheel.h"
#include "GHOST_TimerTask.h"
#include "GHOST_TimerManager.h"
#include "GHOST_WindowManager.h"
#include "GHOST_WindowWin32.h"
#include "GHOST_NDOFManagerWin32.h"
#include "GHOST_TabletManagerWin32.h"

// Key code values not found in winuser.h
#ifndef VK_MINUS
  #define VK_MINUS 0xBD
#endif
#ifndef VK_SEMICOLON
  #define VK_SEMICOLON 0xBA
#endif
#ifndef VK_PERIOD
  #define VK_PERIOD 0xBE
#endif
#ifndef VK_COMMA
  #define VK_COMMA 0xBC
#endif
#ifndef VK_QUOTE
  #define VK_QUOTE 0xDE
#endif
#ifndef VK_BACK_QUOTE
  #define VK_BACK_QUOTE 0xC0
#endif
#ifndef VK_SLASH
  #define VK_SLASH 0xBF
#endif
#ifndef VK_BACK_SLASH
  #define VK_BACK_SLASH 0xDC
#endif
#ifndef VK_EQUALS
  #define VK_EQUALS 0xBB
#endif
#ifndef VK_OPEN_BRACKET
  #define VK_OPEN_BRACKET 0xDB
#endif
#ifndef VK_CLOSE_BRACKET
  #define VK_CLOSE_BRACKET 0xDD
#endif
#ifndef VK_GR_LESS
  #define VK_GR_LESS 0xE2
#endif


GHOST_SystemWin32::GHOST_SystemWin32()
: m_hasPerformanceCounter(false), m_freq(0), m_start(0),
  m_separateLeftRight(false),
  m_separateLeftRightInitialized(false)
{
	m_displayManager = new GHOST_DisplayManagerWin32;
	GHOST_ASSERT(m_displayManager, "GHOST_SystemWin32::GHOST_SystemWin32(): m_displayManager==0\n");
	m_displayManager->initialize();

	// Require COM for GHOST_DropTargetWin32 created in GHOST_WindowWin32.
	OleInitialize(0);

	m_input_fidelity_hint = HI_FI; // just for testing...

	// register for RawInput devices
	RAWINPUTDEVICE devices[1];

	// multi-axis mouse (SpaceNavigator)
	devices[0].usUsagePage = 0x01;
	devices[0].usUsage = 0x08;
	devices[0].dwFlags = 0;
	devices[0].hwndTarget = NULL;

	if (RegisterRawInputDevices(devices, 1, sizeof(RAWINPUTDEVICE)))
		puts("registered for multi-axis input");
}

GHOST_SystemWin32::~GHOST_SystemWin32()
{
	// Shutdown COM
	OleUninitialize();

	delete m_tabletManager;
}


GHOST_TUns64 GHOST_SystemWin32::getMilliSeconds() const
{
	// If hardware does not support high resolution timers,
	// we will use GetTickCount instead.

	if (!m_hasPerformanceCounter) {
		return ::GetTickCount();
	}

	// Retrieve current count
	__int64 count = 0;
	::QueryPerformanceCounter((LARGE_INTEGER*)&count);

	// Calculate the time passed since system initialization.
	__int64 delta = 1000*(count-m_start);

	GHOST_TUns64 t = (GHOST_TUns64)(delta/m_freq);
	return t;
}


GHOST_TUns8 GHOST_SystemWin32::getNumDisplays() const
{
	GHOST_ASSERT(m_displayManager, "GHOST_SystemWin32::getNumDisplays(): m_displayManager==0\n");
	GHOST_TUns8 numDisplays;
	m_displayManager->getNumDisplays(numDisplays);
	return numDisplays;
}


void GHOST_SystemWin32::getMainDisplayDimensions(GHOST_TUns32& width, GHOST_TUns32& height) const
{
	width = ::GetSystemMetrics(SM_CXSCREEN);
	height= ::GetSystemMetrics(SM_CYSCREEN);
}


GHOST_IWindow* GHOST_SystemWin32::createWindow(
	const STR_String& title,
	GHOST_TInt32 left, GHOST_TInt32 top, GHOST_TUns32 width, GHOST_TUns32 height,
	GHOST_TWindowState state, GHOST_TDrawingContextType type,
	bool stereoVisual, const GHOST_TUns16 numOfAASamples, const GHOST_TEmbedderWindowID parentWindow )
{
	GHOST_WindowWin32* window =
		new GHOST_WindowWin32 (this, title, left, top, width, height, state, type, stereoVisual, numOfAASamples);

	if (window) {
		if (window->getValid()) {
			m_windowManager->addWindow(window);
			m_windowManager->setActiveWindow(window);
			if (m_tabletManager->available())
				window->becomeTabletAware(m_tabletManager);
		}
		else {
			// An invalid window could be one that was used to test for AA
			GHOST_WindowWin32* other_window = (GHOST_WindowWin32*) window->getNextWindow();

			delete window;
			window = 0;

			// If another window is found, let the wm know about that one, but not the old one
			if (other_window)
			{
				m_windowManager->addWindow(other_window);
				window = other_window;
			}
		}
	}
	return window;
}


bool GHOST_SystemWin32::processEvents(bool waitForEvent)
{
	MSG msg;
	bool anyProcessed = false;

	do {
		GHOST_TimerManager* timerMgr = getTimerManager();

		if (waitForEvent && !::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
#if 1
			::Sleep(1);
#else
			GHOST_TUns64 next = timerMgr->nextFireTime();
			GHOST_TInt64 maxSleep = next - getMilliSeconds();

			if (next == GHOST_kFireTimeNever) {
				::WaitMessage();
			} else if(maxSleep >= 0.0) {
				::SetTimer(NULL, 0, maxSleep, NULL);
				::WaitMessage();
				::KillTimer(NULL, 0);
			}
#endif
		}

		if (timerMgr->fireTimers(getMilliSeconds())) {
			anyProcessed = true;
		}

		// Process all the events waiting for us
		while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			anyProcessed = true;
		}
	} while (waitForEvent && !anyProcessed);

//	Handle tablet input either here (polling)
// or after WT_PACKET messages (event driven)
// -- not both!

//	if (m_tabletManager->processPackets())
//		anyProcessed = true;

	return anyProcessed;
}


GHOST_TSuccess GHOST_SystemWin32::getCursorPosition(GHOST_TInt32& x, GHOST_TInt32& y) const
{
	POINT point;
	if(::GetCursorPos(&point)){
		x = point.x;
		y = point.y;
		return GHOST_kSuccess;
	}
	return GHOST_kFailure;
}


GHOST_TSuccess GHOST_SystemWin32::setCursorPosition(GHOST_TInt32 x, GHOST_TInt32 y)
{
	if (!GetActiveWindow())
		return GHOST_kFailure;
	return ::SetCursorPos(x, y) == TRUE ? GHOST_kSuccess : GHOST_kFailure;
}


GHOST_TSuccess GHOST_SystemWin32::getModifierKeys(GHOST_ModifierKeys& keys) const
{
	/*
	GetKeyState and GetAsyncKeyState only work with Win95, Win98, NT4,
	Terminal Server and Windows 2000.
	But on WinME it always returns zero. These two functions are simply
	skipped by Millenium Edition!

	Official explanation from Microsoft:
	Intentionally disabled.
	It didn't work all that well on some newer hardware, and worked less 
	well with the passage of time, so it was fully disabled in ME.
	*/
	if (m_separateLeftRight && m_separateLeftRightInitialized) {
		bool down = HIBYTE(::GetKeyState(VK_LSHIFT)) != 0;
		keys.set(GHOST_kModifierKeyLeftShift, down);
		down = HIBYTE(::GetKeyState(VK_RSHIFT)) != 0;
		keys.set(GHOST_kModifierKeyRightShift, down);
		down = HIBYTE(::GetKeyState(VK_LMENU)) != 0;
		keys.set(GHOST_kModifierKeyLeftAlt, down);
		down = HIBYTE(::GetKeyState(VK_RMENU)) != 0;
		keys.set(GHOST_kModifierKeyRightAlt, down);
		down = HIBYTE(::GetKeyState(VK_LCONTROL)) != 0;
		keys.set(GHOST_kModifierKeyLeftControl, down);
		down = HIBYTE(::GetKeyState(VK_RCONTROL)) != 0;
		keys.set(GHOST_kModifierKeyRightControl, down);
	}
	else {
		bool down = HIBYTE(::GetKeyState(VK_SHIFT)) != 0;
		keys.set(GHOST_kModifierKeyLeftShift, down);
		keys.set(GHOST_kModifierKeyRightShift, down);
		down = HIBYTE(::GetKeyState(VK_MENU)) != 0;
		keys.set(GHOST_kModifierKeyLeftAlt, down);
		keys.set(GHOST_kModifierKeyRightAlt, down);
		down = HIBYTE(::GetKeyState(VK_CONTROL)) != 0;
		keys.set(GHOST_kModifierKeyLeftControl, down);
		keys.set(GHOST_kModifierKeyRightControl, down);
	}
	return GHOST_kSuccess;
}


GHOST_TSuccess GHOST_SystemWin32::getButtons(GHOST_Buttons& buttons) const
{
	/* Check for swapped buttons (left-handed mouse buttons)
	 * GetAsyncKeyState() will give back the state of the physical mouse buttons.
	 */
	bool swapped = ::GetSystemMetrics(SM_SWAPBUTTON) == TRUE;

	bool down = HIBYTE(::GetKeyState(VK_LBUTTON)) != 0;
	buttons.set(swapped ? GHOST_kButtonMaskRight : GHOST_kButtonMaskLeft, down);

	down = HIBYTE(::GetKeyState(VK_MBUTTON)) != 0;
	buttons.set(GHOST_kButtonMaskMiddle, down);

	down = HIBYTE(::GetKeyState(VK_RBUTTON)) != 0;
	buttons.set(swapped ? GHOST_kButtonMaskLeft : GHOST_kButtonMaskRight, down);
	return GHOST_kSuccess;
}


GHOST_TSuccess GHOST_SystemWin32::init()
{
	GHOST_TSuccess success = GHOST_System::init();

	m_ndofManager = new GHOST_NDOFManagerWin32(*this);
	m_tabletManager = new GHOST_TabletManagerWin32;

	/* Disable scaling on high DPI displays on Vista */
	HMODULE user32 = ::LoadLibraryA("user32.dll");
	typedef BOOL (WINAPI * LPFNSETPROCESSDPIAWARE)();
	LPFNSETPROCESSDPIAWARE SetProcessDPIAware =
		(LPFNSETPROCESSDPIAWARE)GetProcAddress(user32, "SetProcessDPIAware");
	if (SetProcessDPIAware)
		SetProcessDPIAware();
	FreeLibrary(user32);

	// Determine whether this system has a high frequency performance counter. */
	m_hasPerformanceCounter = ::QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq) == TRUE;
	if (m_hasPerformanceCounter) {
		GHOST_PRINT("GHOST_SystemWin32::init: High Frequency Performance Timer available\n")
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_start);
	}
	else {
		GHOST_PRINT("GHOST_SystemWin32::init: High Frequency Performance Timer not available\n")
	}

	if (success) {
		WNDCLASS wc;
		wc.style= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc= s_wndProc;
		wc.cbClsExtra= 0;
		wc.cbWndExtra= 0;
		wc.hInstance= ::GetModuleHandle(0);
  		wc.hIcon = ::LoadIcon(wc.hInstance, "APPICON");
  		
		if (!wc.hIcon) {
			::LoadIcon(NULL, IDI_APPLICATION);
		}
		wc.hCursor = ::LoadCursor(0, IDC_ARROW);
		wc.hbrBackground= (HBRUSH)::GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName= GHOST_WindowWin32::getWindowClassName();
    
		// Use RegisterClassEx for setting small icon
		if (::RegisterClass(&wc) == 0) {
			success = GHOST_kFailure;
		}
	}
	return success;
}

GHOST_TKey GHOST_SystemWin32::convertKey(WPARAM wParam, LPARAM lParam) const
{
	GHOST_TKey key;
	bool isExtended = (lParam&(1<<24))?true:false;

	if ((wParam >= '0') && (wParam <= '9')) {
		// VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39)
		key = (GHOST_TKey)(wParam - '0' + GHOST_kKey0);
	}
	else if ((wParam >= 'A') && (wParam <= 'Z')) {
		// VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A)
		key = (GHOST_TKey)(wParam - 'A' + GHOST_kKeyA);
	}
	else if ((wParam >= VK_F1) && (wParam <= VK_F24)) {
		key = (GHOST_TKey)(wParam - VK_F1 + GHOST_kKeyF1);
	}
	else {
		switch (wParam) {
		case VK_RETURN:
			key = isExtended?GHOST_kKeyNumpadEnter:GHOST_kKeyEnter;
			break;

		case VK_BACK:     key = GHOST_kKeyBackSpace;		break;
		case VK_TAB:      key = GHOST_kKeyTab;				break;
		case VK_ESCAPE:   key = GHOST_kKeyEsc;				break;
		case VK_SPACE:    key = GHOST_kKeySpace;			break;
		case VK_PRIOR:    key = GHOST_kKeyUpPage;			break;
		case VK_NEXT:     key = GHOST_kKeyDownPage;			break;
		case VK_END:      key = GHOST_kKeyEnd;				break;
		case VK_HOME:     key = GHOST_kKeyHome;				break;
		case VK_INSERT:   key = GHOST_kKeyInsert;			break;
		case VK_DELETE:   key = GHOST_kKeyDelete;			break;
		case VK_LEFT:     key = GHOST_kKeyLeftArrow;		break;
		case VK_RIGHT:    key = GHOST_kKeyRightArrow;		break;
		case VK_UP:       key = GHOST_kKeyUpArrow;			break;
		case VK_DOWN:     key = GHOST_kKeyDownArrow;		break;
		case VK_NUMPAD0:  key = GHOST_kKeyNumpad0;			break;
		case VK_NUMPAD1:  key = GHOST_kKeyNumpad1;			break;
		case VK_NUMPAD2:  key = GHOST_kKeyNumpad2;			break;
		case VK_NUMPAD3:  key = GHOST_kKeyNumpad3;			break;
		case VK_NUMPAD4:  key = GHOST_kKeyNumpad4;			break;
		case VK_NUMPAD5:  key = GHOST_kKeyNumpad5;			break;
		case VK_NUMPAD6:  key = GHOST_kKeyNumpad6;			break;
		case VK_NUMPAD7:  key = GHOST_kKeyNumpad7;			break;
		case VK_NUMPAD8:  key = GHOST_kKeyNumpad8;			break;
		case VK_NUMPAD9:  key = GHOST_kKeyNumpad9;			break;
		case VK_SNAPSHOT: key = GHOST_kKeyPrintScreen;		break;
		case VK_PAUSE:    key = GHOST_kKeyPause;			break;
		case VK_MULTIPLY: key = GHOST_kKeyNumpadAsterisk;	 break;
		case VK_SUBTRACT: key = GHOST_kKeyNumpadMinus;		break;
		case VK_DECIMAL:  key = GHOST_kKeyNumpadPeriod;		break;
		case VK_DIVIDE:   key = GHOST_kKeyNumpadSlash;		break;
		case VK_ADD:      key = GHOST_kKeyNumpadPlus;		break;

		case VK_SEMICOLON:		key = GHOST_kKeySemicolon;		break;
		case VK_EQUALS:			key = GHOST_kKeyEqual;			break;
		case VK_COMMA:			key = GHOST_kKeyComma;			break;
		case VK_MINUS:			key = GHOST_kKeyMinus;			break;
		case VK_PERIOD:			key = GHOST_kKeyPeriod;			break;
		case VK_SLASH:			key = GHOST_kKeySlash;			break;
		case VK_BACK_QUOTE:		key = GHOST_kKeyAccentGrave;	break;
		case VK_OPEN_BRACKET:	key = GHOST_kKeyLeftBracket;	break;
		case VK_BACK_SLASH:		key = GHOST_kKeyBackslash;		break;
		case VK_CLOSE_BRACKET:	key = GHOST_kKeyRightBracket;	break;
		case VK_QUOTE:			key = GHOST_kKeyQuote;			break;
		case VK_GR_LESS:		key = GHOST_kKeyGrLess;			break;

		// Process these keys separately because we need to distinguish right from left modifier keys
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:

		// Ignore these keys
		case VK_NUMLOCK:
		case VK_SCROLL:
		case VK_CAPITAL:
		default:
			key = GHOST_kKeyUnknown;
			break;
		}
	}
	return key;
}


void GHOST_SystemWin32::processModifierKeys(GHOST_IWindow *window)
{
	GHOST_ModifierKeys oldModifiers, newModifiers;
	// Retrieve old state of the modifier keys
	((GHOST_SystemWin32*)getSystem())->retrieveModifierKeys(oldModifiers);
	// Retrieve current state of the modifier keys
	((GHOST_SystemWin32*)getSystem())->getModifierKeys(newModifiers);

	// Compare the old and the new
	if (!newModifiers.equals(oldModifiers)) {
		// Create events for the masks that changed
		for (int i = 0; i < GHOST_kModifierKeyNumMasks; i++) {
			if (newModifiers.get((GHOST_TModifierKeyMask)i) != oldModifiers.get((GHOST_TModifierKeyMask)i)) {
				// Convert the mask to a key code
				GHOST_TKey key = GHOST_ModifierKeys::getModifierKeyCode((GHOST_TModifierKeyMask)i);
				bool keyDown = newModifiers.get((GHOST_TModifierKeyMask)i);
				GHOST_EventKey* event;
				if (key != GHOST_kKeyUnknown) {
					// Create an event
					event = new GHOST_EventKey(getSystem()->getMilliSeconds(), keyDown ? GHOST_kEventKeyDown: GHOST_kEventKeyUp, window, key);
					pushEvent(event);
				}
			}
		}
	}

	// Store new modifier keys state
	((GHOST_SystemWin32*)getSystem())->storeModifierKeys(newModifiers);
}

bool eventIsFromTablet()
	{
	// this (apparently) gives the packet serial number, which only tablet events have
	return GetMessageExtraInfo() != 0;
//	bool IsFromPen = ((GetMessageExtraInfo() & 0xFF515700) == 0xFF515700); // this only works on TabletPCs
//	static int all_magic = 0;
//	int magic = GetMessageExtraInfo();
//	all_magic |= magic;
//	printf("from tablet? %08x %08x\n", magic, all_magic);
//	return magic & 0x7f; // true for tablet mouse, not just pen
	}

GHOST_EventButton* GHOST_SystemWin32::processButtonEvent(GHOST_TEventType type, GHOST_IWindow *window, GHOST_TButtonMask mask)
{
	if (eventIsFromTablet())
		return NULL;

	static GHOST_Buttons buttons;
	if (type == GHOST_kEventButtonUp && !buttons.get(mask))
		// discard rogue button up events (probably from tablet)
		return NULL;
	buttons.set(mask, type == GHOST_kEventButtonDown);

	printf("system button %d %s\n", mask, (type == GHOST_kEventButtonDown) ? "down" : (type == GHOST_kEventButtonUp) ? "up" : "???");

	return new GHOST_EventButton (getSystem()->getMilliSeconds(), type, window, mask);
}


GHOST_EventCursor* GHOST_SystemWin32::processCursorEvent(GHOST_TEventType type, GHOST_IWindow *Iwindow, int x_screen, int y_screen)
{
	printf("system cursor (%d,%d)\n", x_screen, y_screen);

	GHOST_SystemWin32 * system = ((GHOST_SystemWin32 * ) getSystem());
	GHOST_WindowWin32 * window = ( GHOST_WindowWin32 * ) Iwindow;

	if(window->getCursorGrabMode() != GHOST_kGrabDisable && window->getCursorGrabMode() != GHOST_kGrabNormal)
	{
		GHOST_TInt32 x_new= x_screen;
		GHOST_TInt32 y_new= y_screen;
		GHOST_TInt32 x_accum, y_accum;
		GHOST_Rect bounds;

		/* fallback to window bounds */
		if(window->getCursorGrabBounds(bounds)==GHOST_kFailure){
			window->getClientBounds(bounds);
		}

		/* could also clamp to screen bounds
		 * wrap with a window outside the view will fail atm  */

		bounds.wrapPoint(x_new, y_new, 2); /* offset of one incase blender is at screen bounds */

		window->getCursorGrabAccum(x_accum, y_accum);
		if(x_new != x_screen|| y_new != y_screen) {
			/* when wrapping we don't need to add an event because the
			 * setCursorPosition call will cause a new event after */
			system->setCursorPosition(x_new, y_new); /* wrap */
			window->setCursorGrabAccum(x_accum + (x_screen - x_new), y_accum + (y_screen - y_new));
		}else{
			return new GHOST_EventCursor(system->getMilliSeconds(),
										 type,
										 window,
										 x_screen + x_accum,
										 y_screen + y_accum
			);
		}

	}
	else {
		return new GHOST_EventCursor(system->getMilliSeconds(),
									 type,
									 window,
									 x_screen,
									 y_screen
		);
	}
	return NULL;
}


GHOST_EventWheel* GHOST_SystemWin32::processWheelEvent(GHOST_IWindow *window, WPARAM wParam, LPARAM lParam)
{
	// short fwKeys = LOWORD(wParam);			// key flags
	int zDelta = (short) HIWORD(wParam);	// wheel rotation

	// zDelta /= WHEEL_DELTA;
	// temporary fix below: microsoft now has added more precision, making the above division not work
	if (zDelta <= 0 ) zDelta= -1; else zDelta= 1;	
	
	// short xPos = (short) LOWORD(lParam);	// horizontal position of pointer
	// short yPos = (short) HIWORD(lParam);	// vertical position of pointer
	return new GHOST_EventWheel (getSystem()->getMilliSeconds(), window, zDelta);
}


GHOST_EventKey* GHOST_SystemWin32::processKeyEvent(GHOST_IWindow *window, bool keyDown, WPARAM wParam, LPARAM lParam)
{
	GHOST_TKey key = ((GHOST_SystemWin32*)getSystem())->convertKey(wParam, lParam);
	GHOST_EventKey* event;
	if (key != GHOST_kKeyUnknown) {
		MSG keyMsg;
		char ascii = '\0';

			/* Eat any character related messages */
		if (::PeekMessage(&keyMsg, NULL, WM_CHAR, WM_SYSDEADCHAR, PM_REMOVE)) {
			ascii = (char) keyMsg.wParam;
		}

		event = new GHOST_EventKey(getSystem()->getMilliSeconds(), keyDown ? GHOST_kEventKeyDown: GHOST_kEventKeyUp, window, key, ascii);
	}
	else {
		event = 0;
	}
	return event;
}


GHOST_Event* GHOST_SystemWin32::processWindowEvent(GHOST_TEventType type, GHOST_IWindow* window)
{
	GHOST_System* system = (GHOST_System*)getSystem();

	if (type == GHOST_kEventWindowActivate)
		{
		puts("activating window");
		system->getWindowManager()->setActiveWindow(window);
		}

	return new GHOST_Event(system->getMilliSeconds(), type, window);
}

GHOST_TSuccess GHOST_SystemWin32::pushDragDropEvent(GHOST_TEventType eventType, 
													GHOST_TDragnDropTypes draggedObjectType,
													GHOST_IWindow* window,
													int mouseX, int mouseY,
													void* data)
{
	GHOST_SystemWin32* system = ((GHOST_SystemWin32*)getSystem());
	return system->pushEvent(new GHOST_EventDragnDrop(system->getMilliSeconds(),
													  eventType,
													  draggedObjectType,
													  window,mouseX,mouseY,data)
			);
}

void GHOST_SystemWin32::processMinMaxInfo(MINMAXINFO * minmax)
{
	minmax->ptMinTrackSize.x=320;
	minmax->ptMinTrackSize.y=240;
}

bool GHOST_SystemWin32::processRawInput(RAWINPUT const& raw, GHOST_WindowWin32* window)
{
	bool eventSent = false;

	if (raw.header.dwType == RIM_TYPEHID)
		{
//		RID_DEVICE_INFO info;
//		DWORD infoSize = sizeof(RID_DEVICE_INFO);
//		int n = GetRawInputDeviceInfo(raw.header.hDevice, RIDI_DEVICEINFO, &info, &infoSize);
		// ... and so on
		// I'll finish this device checking code later. Since we ask for only multi-axis input
		// in the constructor, that's all we should get here.

/*
Here's some handy info from http://www.linux-usb.org/usb.ids

vendor ID
046d  Logitech, Inc.

	device IDs
	c623  3Dconnexion Space Traveller 3D Mouse
	c625  3Dconnexion Space Pilot 3D Mouse
	c626  3Dconnexion Space Navigator 3D Mouse
	c627  3Dconnexion Space Explorer 3D Mouse

No other registered devices use the c62_ space, so a simple mask will work!
*/
		// The NDOF manager sends button changes immediately, and *pretends* to
		// send motion. Mark as 'sent' so motion will always get dispatched.
		eventSent = true;

		// multiple events per RAWHID? MSDN hints at this.
		// printf("%d events\n", (int)raw.data.hid.dwCount);

		BYTE const* data = &raw.data.hid.bRawData;
		// MinGW's definition (below) doesn't agree with MSDN reference for bRawData:
		// typedef struct tagRAWHID {
		// 	DWORD dwSizeHid;
		// 	DWORD dwCount;
		// 	BYTE bRawData; // <== isn't this s'posed to be a BYTE*?
		// } RAWHID,*PRAWHID,*LPRAWHID;

		BYTE packetType = data[0];
		switch (packetType)
			{
			case 1: // translation
				{
				short t[3];
				memcpy(t, data + 1, sizeof(t));
				// too much noise -- disable for now
				// printf("T: %+5d %+5d %+5d\n", t[0], t[1], t[2]);
				m_ndofManager->updateTranslation(t, getMilliSeconds());
				// wariness of alignment issues prevents me from saying it this way:
				// m_ndofManager->updateTranslation((short*)(data + 1), getMilliSeconds());
				// though it probably (94.7%) works fine
				break;
				}
			case 2: // rotation
				{
				short r[3];
				memcpy(r, data + 1, sizeof(r));
				// printf("R: %+5d %+5d %+5d\n", r[0], r[1], r[2]);
				m_ndofManager->updateRotation(r, getMilliSeconds());
				break;
				}
			case 3: // buttons
				{
				unsigned short buttons;
				memcpy(&buttons, data + 1, sizeof(buttons));
#if 0
				printf("buttons:");
				if (buttons)
					{
					// work our way through the bit mask
					for (int i = 0; i < 16; ++i)
						if (buttons & (1 << i))
							printf(" %d", i + 1);
					printf("\n");
					}
				else
					printf(" none\n");
#endif
				m_ndofManager->updateButtons(buttons, getMilliSeconds());
				break;
				}
			}
		}
	return eventSent;
}

int GHOST_SystemWin32::getMoreMousePoints(int xLatest, int yLatest, int xPrev, int yPrev, GHOST_WindowWin32* window)
	{
	MOUSEMOVEPOINT point = {
		xLatest & 0x0000FFFF, // confine to low word to make GetMouseMovePointsEx happy
		yLatest & 0x0000FFFF,
		0, // time stamp unknown
		NULL // no extra info
		};

	MOUSEMOVEPOINT morePoints[64];

	int n = GetMouseMovePointsEx(sizeof(MOUSEMOVEPOINT), &point, morePoints, 64, GMMP_USE_DISPLAY_POINTS);
	if (n == -1)
		{
		printf("<!> can't get more mouse points (error %d)\n", (int) GetLastError());
		return 0;
		}

	// search for 'prev' point (we want only newer points)
	// TODO: detect & ignore points that don't belong to our window
	for (int i = 1; i < n; ++i)
		if (morePoints[i].x == xPrev && morePoints[i].y == yPrev)
			{
			n = i; // don't include found point (or more ancient points)
			break;
			}

	for (int i = n - 1; i > 0; --i)
		{
		signed short x = morePoints[i].x;
		signed short y = morePoints[i].y;

		// uncomment here and in WM_MOUSEMOVE handler to show effectiveness of hi-fi input
		// printf("> (%d,%d)\n", x, y);

		pushEvent(processCursorEvent(GHOST_kEventCursorMove, window, x, y));
		}

	return n;
	}

LRESULT WINAPI GHOST_SystemWin32::s_wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	GHOST_SystemWin32* system = ((GHOST_SystemWin32*)getSystem());
	GHOST_ASSERT(system, "GHOST_SystemWin32::s_wndProc(): system not initialized")
	// [mce] then why not register this function at the end of SystemWin32 constructor?

	bool handled = false;

	if (hwnd) {
		GHOST_WindowWin32* window = (GHOST_WindowWin32*)::GetWindowLong(hwnd, GWL_USERDATA);
		if (window) {
			handled = system->handleEvent(window, msg, wParam, lParam);

			// take care of misc. cases that need hwnd
			if (msg == WM_ACTIVATE)
				/* WARNING: Let DefWindowProc handle WM_ACTIVATE, otherwise WM_MOUSEWHEEL
				will not be dispatched to OUR active window if we minimize one of OUR windows. */
				lResult = ::DefWindowProc(hwnd, msg, wParam, lParam);
			else if (msg == WM_PAINT)
				/* An application sends the WM_PAINT message when the system or another application
				* makes a request to paint a portion of an application's window. The message is sent
				* when the UpdateWindow or RedrawWindow function is called, or by the DispatchMessage
				* function when the application obtains a WM_PAINT message by using the GetMessage or
				* PeekMessage function. */
				::ValidateRect(hwnd, NULL);
		}
		else {
			// Event found for a window before the pointer to the class has been set.
			GHOST_PRINT("GHOST_SystemWin32::wndProc: GHOST window event before creation\n")

			/* These are events we typically miss at this point:
			   WM_GETMINMAXINFO	0x24
			   WM_NCCREATE			0x81
			   WM_NCCALCSIZE		0x83
			   WM_CREATE			0x01
			   We let DefWindowProc do the work.
			*/
		}
	}
	else {
		// Events without valid hwnd
		GHOST_PRINT("GHOST_SystemWin32::wndProc: event without window\n")
	}

	if (!handled)
		lResult = ::DefWindowProc(hwnd, msg, wParam, lParam);

	return lResult;
}

bool GHOST_SystemWin32::handleEvent(GHOST_WindowWin32* window, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	GHOST_Event* event = NULL;
	bool eventSent = false;

	static int mousePosX = 0, mousePosY = 0; // track mouse position between calls

	switch (msg) {
		////////////////////////////////////////////////////////////////////////
		// Keyboard events, processed
		////////////////////////////////////////////////////////////////////////
		case WM_KEYDOWN:
			/* The WM_KEYDOWN message is posted to the window with the keyboard focus when a
			 * nonsystem key is pressed. A nonsystem key is a key that is pressed when the alt
			 * key is not pressed.
			 */
		case WM_SYSKEYDOWN:
			/* The WM_SYSKEYDOWN message is posted to the window with the keyboard focus when 
			 * the user presses the F10 key (which activates the menu bar) or holds down the 
			 * alt key and then presses another key. It also occurs when no window currently 
			 * has the keyboard focus; in this case, the WM_SYSKEYDOWN message is sent to the 
			 * active window. The window that receives the message can distinguish between these 
			 * two contexts by checking the context code in the lKeyData parameter. 
			 */
			switch (wParam) {
				case VK_SHIFT:
				case VK_CONTROL:
				case VK_MENU:
					if (!m_separateLeftRightInitialized) {
						// Check whether this system supports separate left and right keys
						switch (wParam) {
							case VK_SHIFT:
								m_separateLeftRight =
									(HIBYTE(::GetKeyState(VK_LSHIFT)) != 0) ||
									(HIBYTE(::GetKeyState(VK_RSHIFT)) != 0) ?
									true : false;
								break;
							case VK_CONTROL:
								m_separateLeftRight =
									(HIBYTE(::GetKeyState(VK_LCONTROL)) != 0) ||
									(HIBYTE(::GetKeyState(VK_RCONTROL)) != 0) ?
									true : false;
								break;
							case VK_MENU:
								m_separateLeftRight =
									(HIBYTE(::GetKeyState(VK_LMENU)) != 0) ||
									(HIBYTE(::GetKeyState(VK_RMENU)) != 0) ?
									true : false;
								break;
						}
						m_separateLeftRightInitialized = true;
					}
					processModifierKeys(window);
					// Bypass call to DefWindowProc
					return true;
				default:
					event = processKeyEvent(window, true, wParam, lParam);
					if (!event) {
						GHOST_PRINT("GHOST_SystemWin32::wndProc: key event ")
						GHOST_PRINT(msg)
						GHOST_PRINT(" key ignored\n")
					}
					break;
				}
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			switch (wParam) {
				case VK_SHIFT:
				case VK_CONTROL:
				case VK_MENU:
					processModifierKeys(window);
					// Bypass call to DefWindowProc
					return true;
				default:
					event = processKeyEvent(window, false, wParam, lParam);
					if (!event) {
						GHOST_PRINT("GHOST_SystemWin32::wndProc: key event ")
						GHOST_PRINT(msg)
						GHOST_PRINT(" key ignored\n")
					}
					break;
			}
			break;

#if 0 // this code is illustrative; no need to compile
		////////////////////////////////////////////////////////////////////////
		// Keyboard events, ignored
		////////////////////////////////////////////////////////////////////////
		case WM_CHAR:
			/* The WM_CHAR message is posted to the window with the keyboard focus when 
			 * a WM_KEYDOWN message is translated by the TranslateMessage function. WM_CHAR 
			 * contains the character code of the key that was pressed. 
			 */
		case WM_DEADCHAR:
			/* The WM_DEADCHAR message is posted to the window with the keyboard focus when a
			 * WM_KEYUP message is translated by the TranslateMessage function. WM_DEADCHAR 
			 * specifies a character code generated by a dead key. A dead key is a key that 
			 * generates a character, such as the umlaut (double-dot), that is combined with 
			 * another character to form a composite character. For example, the umlaut-O 
			 * character (�) is generated by typing the dead key for the umlaut character, and 
			 * then typing the O key.
			 */
		case WM_SYSDEADCHAR:
			/* The WM_SYSDEADCHAR message is sent to the window with the keyboard focus when 
			 * a WM_SYSKEYDOWN message is translated by the TranslateMessage function. 
			 * WM_SYSDEADCHAR specifies the character code of a system dead key - that is, 
			 * a dead key that is pressed while holding down the alt key. 
			 */
			break;
#endif // illustrative code

		////////////////////////////////////////////////////////////////////////
		// Tablet events, processed
		////////////////////////////////////////////////////////////////////////
		case WT_PACKET:
			m_tabletManager->processPackets(window);
			break;
		case WT_CSRCHANGE:
			m_tabletManager->changeTool(window, wParam);
			break;
		case WT_PROXIMITY:
			if (LOWORD(lParam) == 0)
				{
				puts("-- dropping tool --");
				m_tabletManager->dropTool();
				}
			break;

		////////////////////////////////////////////////////////////////////////
		// Mouse events, processed
		////////////////////////////////////////////////////////////////////////
		case WM_LBUTTONDOWN:
			window->registerMouseClickEvent(true);
			event = processButtonEvent(GHOST_kEventButtonDown, window, GHOST_kButtonMaskLeft);
			break;
		case WM_MBUTTONDOWN:
			window->registerMouseClickEvent(true);
			event = processButtonEvent(GHOST_kEventButtonDown, window, GHOST_kButtonMaskMiddle);
			break;
		case WM_RBUTTONDOWN:
			window->registerMouseClickEvent(true);
			event = processButtonEvent(GHOST_kEventButtonDown, window, GHOST_kButtonMaskRight);
			break;
		case WM_XBUTTONDOWN:
			window->registerMouseClickEvent(true);
			if ((short) HIWORD(wParam) == XBUTTON1){
				event = processButtonEvent(GHOST_kEventButtonDown, window, GHOST_kButtonMaskButton4);
			}else if((short) HIWORD(wParam) == XBUTTON2){
				event = processButtonEvent(GHOST_kEventButtonDown, window, GHOST_kButtonMaskButton5);
			}
			break;
		case WM_LBUTTONUP:
			window->registerMouseClickEvent(false);
			event = processButtonEvent(GHOST_kEventButtonUp, window, GHOST_kButtonMaskLeft);
			break;
		case WM_MBUTTONUP:
			window->registerMouseClickEvent(false);
			event = processButtonEvent(GHOST_kEventButtonUp, window, GHOST_kButtonMaskMiddle);
			break;
		case WM_RBUTTONUP:
			window->registerMouseClickEvent(false);
			event = processButtonEvent(GHOST_kEventButtonUp, window, GHOST_kButtonMaskRight);
			break;
		case WM_XBUTTONUP:
			window->registerMouseClickEvent(false);
			if ((short) HIWORD(wParam) == XBUTTON1){
				event = processButtonEvent(GHOST_kEventButtonUp, window, GHOST_kButtonMaskButton4);
			}else if((short) HIWORD(wParam) == XBUTTON2){
				event = processButtonEvent(GHOST_kEventButtonUp, window, GHOST_kButtonMaskButton5);
			}
			break;

		case WM_MOUSEMOVE:
			{
			if (!eventIsFromTablet() && !m_tabletManager->anyButtonsDown())
				// Even with careful checking, a stray cursor event sneaks through just before each
				// tablet mouse/pen button up event. Keep clean separation between tablet and mouse!
				{
				int xPrev = mousePosX;
				int yPrev = mousePosY;
				// window coordinates are passed in via lParam
				window->clientToScreen(LOWORD(lParam), HIWORD(lParam), mousePosX, mousePosY);

				// putchar('\n');

				if (m_input_fidelity_hint == HI_FI)
					{
					GHOST_Buttons buttons;
					getButtons(buttons);
					// don't bother grabbing extra mouse motion unless we're in a stroke
					if (buttons.anyDown())
						{
						// int n =
						getMoreMousePoints(mousePosX, mousePosY, xPrev, yPrev, window);
						// printf("%d more mouse points found\n", n);
						}
					}

				// uncomment here and in getMoreMousePoints to show effectiveness of hi-fi input
				// printf("  (%d,%d)\n", mousePosX, mousePosY);

				event = processCursorEvent(GHOST_kEventCursorMove, window, mousePosX, mousePosY);
				}
			break;
			}

		case WM_INPUT:
			{
			// Raw mouse input benefitted from the buffered method,
			// but SpaceNav gets along fine (better, even) grabbing single events.

         RAWINPUT raw;
         RAWINPUT* raw_ptr = &raw;
         UINT rawSize = sizeof(RAWINPUT);

         GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw_ptr, &rawSize, sizeof(RAWINPUTHEADER));
         eventSent |= processRawInput(raw, window);

			if (processRawInput(raw, window))
				eventSent = true;
//			else
//				DefRawInputProc(&raw_ptr, 1, sizeof(RAWINPUTHEADER));

#if 0
			#define RAWCOUNT 10
			// just a guess that we'll receive up to 10 event reports
			// the following code fetches all available, 10 at a time
			RAWINPUT rawBuffer[RAWCOUNT];

			while (true)
				{
				UINT rawSize = sizeof(rawBuffer);
				int n = GetRawInputBuffer(rawBuffer, &rawSize, sizeof(RAWINPUTHEADER));
				if (n == -1)
					{
					printf("<!> error %d\n", (int) GetLastError());
					break;
					}
				else if (n == 0)
					{
					putchar('\n');
					break;
					}
				else
					{
					printf("processing %d events:\n", n);
					for (int i = 0; i < n; ++i)
						{
						RAWINPUT const& raw = rawBuffer[i];
						eventSent |= processRawInput(raw, window);
						}

					// clear processed events from the queue
					// DefRawInputProc((RAWINPUT**)&rawBuffer, n, sizeof(RAWINPUTHEADER));
					}
				}
#endif
 			break;
			}
		case WM_MOUSEWHEEL:
			/* The WM_MOUSEWHEEL message is sent to the focus window
			 * when the mouse wheel is rotated. The DefWindowProc
			 * function propagates the message to the window's parent.
			 * There should be no internal forwarding of the message,
			 * since DefWindowProc propagates it up the parent chain
			 * until it finds a window that processes it.
			 */
			event = processWheelEvent(window, wParam, lParam);
			break;
		case WM_SETCURSOR:
			/* The WM_SETCURSOR message is sent to a window if the mouse causes the cursor
			 * to move within a window and mouse input is not captured.
			 * This means we have to set the cursor shape every time the mouse moves!
			 * The DefWindowProc function uses this message to set the cursor to an 
			 * arrow if it is not in the client area.
			 */
			if (LOWORD(lParam) == HTCLIENT) {
				// Load the current cursor
				window->loadCursor(window->getCursorVisibility(), window->getCursorShape());
				// Bypass call to DefWindowProc
				return 0;
			} 
			else {
				// Outside of client area show standard cursor
				window->loadCursor(true, GHOST_kStandardCursorDefault);
			}
			break;

#if 0 // this code is illustrative; no need to compile
		////////////////////////////////////////////////////////////////////////
		// Mouse events, ignored
		////////////////////////////////////////////////////////////////////////
		case WM_NCMOUSEMOVE:
			/* The WM_NCMOUSEMOVE message is posted to a window when the cursor is moved 
			 * within the nonclient area of the window. This message is posted to the window 
			 * that contains the cursor. If a window has captured the mouse, this message is not posted.
			 */
		case WM_NCHITTEST:
			/* The WM_NCHITTEST message is sent to a window when the cursor moves, or 
			 * when a mouse button is pressed or released. If the mouse is not captured, 
			 * the message is sent to the window beneath the cursor. Otherwise, the message 
			 * is sent to the window that has captured the mouse. 
			 */
			break;
#endif // illustrative code

		////////////////////////////////////////////////////////////////////////
		// Window events, processed
		////////////////////////////////////////////////////////////////////////
		case WM_CLOSE:
			/* The WM_CLOSE message is sent as a signal that a window or an application should terminate. */
			event = processWindowEvent(GHOST_kEventWindowClose, window);
			break;
		case WM_ACTIVATE:
			/* The WM_ACTIVATE message is sent to both the window being activated and the window being 
			 * deactivated. If the windows use the same input queue, the message is sent synchronously, 
			 * first to the window procedure of the top-level window being deactivated, then to the window
			 * procedure of the top-level window being activated. If the windows use different input queues,
			 * the message is sent asynchronously, so the window is activated immediately. 
			 */
			event = processWindowEvent(LOWORD(wParam) ? GHOST_kEventWindowActivate : GHOST_kEventWindowDeactivate, window);
			break;
		case WM_PAINT:
			/* An application sends the WM_PAINT message when the system or another application
			 * makes a request to paint a portion of an application's window. The message is sent
			 * when the UpdateWindow or RedrawWindow function is called, or by the DispatchMessage
			 * function when the application obtains a WM_PAINT message by using the GetMessage or 
			 * PeekMessage function. 
			 */
			event = processWindowEvent(GHOST_kEventWindowUpdate, window);
			break;
		case WM_GETMINMAXINFO:
			/* The WM_GETMINMAXINFO message is sent to a window when the size or 
			 * position of the window is about to change. An application can use 
			 * this message to override the window's default maximized size and 
			 * position, or its default minimum or maximum tracking size. 
			 */
			processMinMaxInfo((MINMAXINFO *) lParam);
			/* Let DefWindowProc handle it. */
			break;
		case WM_SIZE:
			/* The WM_SIZE message is sent to a window after its size has changed.
			 * The WM_SIZE and WM_MOVE messages are not sent if an application handles the 
			 * WM_WINDOWPOSCHANGED message without calling DefWindowProc. It is more efficient
			 * to perform any move or size change processing during the WM_WINDOWPOSCHANGED 
			 * message without calling DefWindowProc.
			 */
			event = processWindowEvent(GHOST_kEventWindowSize, window);
			break;
		case WM_CAPTURECHANGED:
			window->lostMouseCapture();
			break;
		case WM_MOVING:
			/* The WM_MOVING message is sent to a window that the user is moving. By processing 
			 * this message, an application can monitor the size and position of the drag rectangle
			 * and, if needed, change its size or position.
			 */
		case WM_MOVE:
			/* The WM_SIZE and WM_MOVE messages are not sent if an application handles the 
			 * WM_WINDOWPOSCHANGED message without calling DefWindowProc. It is more efficient
			 * to perform any move or size change processing during the WM_WINDOWPOSCHANGED 
			 * message without calling DefWindowProc. 
			 */
			event = processWindowEvent(GHOST_kEventWindowMove, window);
			break;

#if 0 // this code is illustrative; no need to compile
		////////////////////////////////////////////////////////////////////////
		// Window events, ignored
		////////////////////////////////////////////////////////////////////////
		case WM_WINDOWPOSCHANGED:
			/* The WM_WINDOWPOSCHANGED message is sent to a window whose size, position, or place
			 * in the Z order has changed as a result of a call to the SetWindowPos function or 
			 * another window-management function.
			 * The WM_SIZE and WM_MOVE messages are not sent if an application handles the 
			 * WM_WINDOWPOSCHANGED message without calling DefWindowProc. It is more efficient
			 * to perform any move or size change processing during the WM_WINDOWPOSCHANGED 
			 * message without calling DefWindowProc.
			 */
		case WM_ERASEBKGND:
			/* An application sends the WM_ERASEBKGND message when the window background must be 
			 * erased (for example, when a window is resized). The message is sent to prepare an 
			 * invalidated portion of a window for painting. 
			 */
		case WM_NCPAINT:
			/* An application sends the WM_NCPAINT message to a window when its frame must be painted. */
		case WM_NCACTIVATE:
			/* The WM_NCACTIVATE message is sent to a window when its nonclient area needs to be changed 
			 * to indicate an active or inactive state. 
			 */
		case WM_DESTROY:
			/* The WM_DESTROY message is sent when a window is being destroyed. It is sent to the window 
			 * procedure of the window being destroyed after the window is removed from the screen.	
			 * This message is sent first to the window being destroyed and then to the child windows 
			 * (if any) as they are destroyed. During the processing of the message, it can be assumed 
			 * that all child windows still exist. 
			 */
		case WM_NCDESTROY:
			/* The WM_NCDESTROY message informs a window that its nonclient area is being destroyed. The 
			 * DestroyWindow function sends the WM_NCDESTROY message to the window following the WM_DESTROY
			 * message. WM_DESTROY is used to free the allocated memory object associated with the window. 
			 */
		case WM_KILLFOCUS:
			/* The WM_KILLFOCUS message is sent to a window immediately before it loses the keyboard focus. */
		case WM_SHOWWINDOW:
			/* The WM_SHOWWINDOW message is sent to a window when the window is about to be hidden or shown. */
		case WM_WINDOWPOSCHANGING:
			/* The WM_WINDOWPOSCHANGING message is sent to a window whose size, position, or place in 
			 * the Z order is about to change as a result of a call to the SetWindowPos function or 
			 * another window-management function. 
			 */
		case WM_SETFOCUS:
			/* The WM_SETFOCUS message is sent to a window after it has gained the keyboard focus. */
		case WM_ENTERSIZEMOVE:
			/* The WM_ENTERSIZEMOVE message is sent one time to a window after it enters the moving 
			 * or sizing modal loop. The window enters the moving or sizing modal loop when the user 
			 * clicks the window's title bar or sizing border, or when the window passes the 
			 * WM_SYSCOMMAND message to the DefWindowProc function and the wParam parameter of the 
			 * message specifies the SC_MOVE or SC_SIZE value. The operation is complete when 
			 * DefWindowProc returns. 
			 */
			break;
			
		////////////////////////////////////////////////////////////////////////
		// Other events
		////////////////////////////////////////////////////////////////////////
		case WM_GETTEXT:
			/* An application sends a WM_GETTEXT message to copy the text that 
			 * corresponds to a window into a buffer provided by the caller. 
			 */
		case WM_ACTIVATEAPP:
			/* The WM_ACTIVATEAPP message is sent when a window belonging to a 
			 * different application than the active window is about to be activated.
			 * The message is sent to the application whose window is being activated
			 * and to the application whose window is being deactivated. 
			 */
		case WM_TIMER:
			/* The WIN32 docs say:
			 * The WM_TIMER message is posted to the installing thread's message queue
			 * when a timer expires. You can process the message by providing a WM_TIMER
			 * case in the window procedure. Otherwise, the default window procedure will
			 * call the TimerProc callback function specified in the call to the SetTimer
			 * function used to install the timer. 
			 *
			 * In GHOST, we let DefWindowProc call the timer callback.
			 */
			break;
#endif // illustrative code
	}

	if (!eventSent)
		if (event) {
			pushEvent(event);
			eventSent = true;
		}

	return eventSent;
}

GHOST_TUns8* GHOST_SystemWin32::getClipboard(bool selection) const
{
	char *buffer;
	char *temp_buff;
	
	if ( IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(NULL) ) {
		HANDLE hData = GetClipboardData( CF_TEXT );
		if (hData == NULL) {
			CloseClipboard();
			return NULL;
		}
		buffer = (char*)GlobalLock( hData );
		
		temp_buff = (char*) malloc(strlen(buffer)+1);
		strcpy(temp_buff, buffer);
		
		GlobalUnlock( hData );
		CloseClipboard();
		
		temp_buff[strlen(buffer)] = '\0';
		if (buffer) {
			return (GHOST_TUns8*)temp_buff;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

void GHOST_SystemWin32::putClipboard(GHOST_TInt8 *buffer, bool selection) const
{
	if(selection) {return;} // for copying the selection, used on X11

	if (OpenClipboard(NULL)) {
		HLOCAL clipbuffer;
		char *data;
		
		if (buffer) {
			EmptyClipboard();
			
			clipbuffer = LocalAlloc(LMEM_FIXED,((strlen(buffer)+1)));
			data = (char*)GlobalLock(clipbuffer);

			strcpy(data, (char*)buffer);
			data[strlen(buffer)] = '\0';
			LocalUnlock(clipbuffer);
			SetClipboardData(CF_TEXT,clipbuffer);
		}
		CloseClipboard();
	} else {
		return;
	}
}

const GHOST_TUns8* GHOST_SystemWin32::getSystemDir() const
{
	static char knownpath[MAX_PATH];
	HRESULT hResult = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, knownpath);

	if (hResult == S_OK)
	{
		return (GHOST_TUns8*)knownpath;
	}

	return NULL;
}

const GHOST_TUns8* GHOST_SystemWin32::getUserDir() const
{
	static char knownpath[MAX_PATH];
	HRESULT hResult = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, knownpath);

	if (hResult == S_OK)
	{
		return (GHOST_TUns8*)knownpath;
	}

	return NULL;
}

const GHOST_TUns8* GHOST_SystemWin32::getBinaryDir() const
{
	static char fullname[MAX_PATH];
	if(GetModuleFileName(0, fullname, MAX_PATH)) {
		return (GHOST_TUns8*)fullname;
	}

	return NULL;
}
