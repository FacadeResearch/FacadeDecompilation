#include "Input.h"
#include "Globals.h"

LPDIRECTINPUTDEVICE8 Input::g_pKeyboard = NULL;
LPDIRECTINPUT8 Input::g_pDI = NULL;
XCursor Input::g_pCursor;

HRESULT Input::Keyboard_Init()
{
	HRESULT hr;
	hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&Input::g_pDI, NULL);

	if (FAILED(hr)) return hr;
	hr = Input::g_pDI->CreateDevice(GUID_SysKeyboard, &Input::g_pKeyboard, NULL);

	if (FAILED(hr)) return hr;
	hr = Input::g_pKeyboard->SetDataFormat(&c_dfDIKeyboard);

	if (FAILED(hr)) return hr;
	hr = Input::g_pKeyboard->SetCooperativeLevel(g_Window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	if (FAILED(hr)) return hr;

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 16;

	hr = Input::g_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if (FAILED(hr)) return hr;

	hr = Input::g_pKeyboard->Acquire();
	return S_OK;
}
