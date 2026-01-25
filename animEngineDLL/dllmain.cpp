#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dsound.lib")

#include <Windows.h>
#include <string>
#include <iostream>
#include "EventLog.h"
#include "AppWindow.h"
#include <dinput.h>
#include "SoundEntry.h"
#include "Engine.h"
#include "Globals.h"

void CreateDebugConsole() {
	AllocConsole();

	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);

	SetConsoleTitleA("Façade Debug Console");
}

__declspec(dllexport) int __cdecl AnimEngineWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	CreateDebugConsole();

	int result = 0;

	Engine::g_bPrintBetaInfo = GetAsyncKeyState(VK_CONTROL) & 0x8000;

	if (Engine::g_bPrintBetaInfo) {
		const char* version = "8 (7.9.05)";
		char buffer[32];
		sprintf(buffer, "Beta build %s", version);
	}
	
	char stageplayPath[MAX_PATH];
	WIN32_FIND_DATAA findData;
	HANDLE hFind;

	strcpy(stageplayPath, "..\\..\\..\\stageplays\\");
	strcat(stageplayPath, "*.txt");

	hFind = FindFirstFileA(stageplayPath, &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		if (FindNextFileA(hFind, &findData)) {
			Engine::g_stagePlayExists = true;
		}
		FindClose(hFind);
	}

	if (DSound_Init() == 0) {
		EventLog::ShouldntBeHere("DSound did not initialize");
	}

	if (FAILED(CoInitialize(NULL))) {
		EventLog::ShouldntBeHere("CoInitialize Failed!\n");
		exit(1);
	}

	WNDCLASSA windowClass = { 0 };

	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = AppWindow::WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
	windowClass.lpszClassName = "FacadeClass";

	if (RegisterClassA(&windowClass)) {
		HWND window = CreateWindowExA(
			WS_EX_LEFT,
			"FacadeClass",
			"Façade",
			WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0,
			AppWindow::g_windowWidth, AppWindow::g_windowHeight,
			NULL, NULL,
			hInstance,
			NULL
		);

		if (window != NULL) {
			Engine::g_Window = window;

			Engine::InitGlobals();

			if (Keyboard_Init() != 0) {
				EventLog::ShouldntBeHere("Keyboard did not initialize");
			}

			ShowWindow(window, SW_SHOW);
			UpdateWindow(window);

			SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

			MSG msg;
			while (GetMessageA(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			CoUninitialize();
			ReleaseDC(window, GetDC(window));
		}
	}

	return result;
}