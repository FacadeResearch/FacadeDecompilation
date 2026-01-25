#pragma once
#include <dinput.h>
#include "XCursor.h"

class Input {
	public:
		static LPDIRECTINPUTDEVICE8 g_pKeyboard;
		static LPDIRECTINPUT8 g_pDI;
		static XCursor g_pCursor;
		static HRESULT Keyboard_Init();
};