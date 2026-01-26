#pragma once
#include <Windows.h>
#include <cstdint>

class AppWindow {
	public:
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static BOOL SetupPixelFormat(HDC hdc);
		static int g_SleepTime;
		static int g_MouseX, g_MouseY;
		static bool g_bAntiAliasing;
		static int g_mouseLButtonClicked;
		static int g_windowWidth, g_windowHeight;
		static bool g_bAblDisconnect;
		static HPALETTE g_hPalette;
		static bool g_IsInactive;
	private:
		static HPALETTE CreateEnginePalette(HDC hdc);
		static void InitOpenGLState();
};