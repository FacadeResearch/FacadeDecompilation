#pragma once
#include <Windows.h>
#include <dsound.h>
#include <dinput.h>
#include <string>
#include "SoundEntry.h"
#include <gl/GL.h>
#include "Player.h"
#include "XCursor.h"

struct Engine {
	public:
		static bool g_bPrintBetaInfo, g_stagePlayExists, g_bDSoundInitialized, g_bAtEndOfLoad, g_DebugBuild;
		static float g_GlobalAlpha; // data_106988bc
		static float g_FadeAlpha, g_FadeSpeed;
		static int fadeCurtainInCtr; // data_106988e0
		static bool bFadeCurtainOut;
		static float g_gmf[256];
		static HWND g_Window;
		static Player g_Player;
		static HDC g_HDC;
		static GLuint g_BitmapFontBase;
		static GLuint g_LoadingTextureID;
		static LPDIRECTINPUTDEVICE8 g_pKeyboard;
		static LPDIRECTINPUT8 g_pDI;
		static LPDIRECTSOUND g_pDS;
		static SoundEntry g_SoundBank[256];
		static int g_ticksIntroStarted;
		static int g_numFramesRun;
		static HRESULT Keyboard_Init();
		static HRESULT DSound_Init();
		static XCursor g_pCursor;
		static void InitGlobals();
		static void SetColorFromIndex(int colorIndex);
		static void LoadSoundManifest(const char* path);
		static void DrawArbitraryScreenText(const char* text, float xPos, float yPos, int maxWidth, GLfloat scale, float zOffset, int colorIndex);
		static void StartFade(float speed);
		static void UpdateFadeEffect();
		static void LoadingMessage();
		static void DrawRedCurtain();
		static void CutToBlack();
		static int* ChangeSize();
	private:
		static int ctr;
		static std::string g_launchErrorString;
		static long double glPrintPolygonalFont(char* __src, ...);
};