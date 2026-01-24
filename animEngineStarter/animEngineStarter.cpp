#include <Windows.h>

typedef int (__cdecl* AnimEngineWinMain)(HINSTANCE);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	HMODULE animEngineDLL = LoadLibraryA("animEngineDLL.dll");

	if (!animEngineDLL) return 1;

	AnimEngineWinMain animWinMain = (AnimEngineWinMain)GetProcAddress(animEngineDLL, "?AnimEngineWinMain@@YAHPAUHINSTANCE__@@0PADH@Z");

	if (animWinMain) {
		return animWinMain(hInstance);
	}

	return 1;
}