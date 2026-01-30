#pragma once
#include <Windows.h>
#include <dsound.h>
#include <dinput.h>
#include <string>
#include "SoundEntry.h"
#include "SoundSource.h"
#include <gl/GL.h>
#include "Player.h"
#include "XCursor.h"
#include "TextureFilmstrip.h"
#include "Zone.h"
#include "Controller.h"
#include "Sprite.h"

struct Engine {
	public:
		static bool g_bPrintBetaInfo, g_stagePlayExists, g_bDSoundInitialized, g_bAtEndOfLoad;
		static float g_GlobalAlpha; // data_106988bc
		static float g_FadeAlpha, g_FadeSpeed;
		static int fadeCurtainInCtr; // data_106988e0
		static bool bFadeCurtainOut;
		static float* g_gmf;
		static int32_t g_startTime;
		static bool g_bShouldRenderThisFrame;
		static int g_SkipFramesCounter;
		static int g_numZones;
		static Zone g_zones[250];
		static uint8_t g_fixedPathPlanGrid[90][90];
		static int g_printMapCtr;
		static bool g_bAssertionThrown;
		static bool g_bAllQuadLineOutline;
		static bool g_bAllCircleOutline;
		static bool g_bCinematographyOn;
		static int g_frameWhenLastCinematographyMove;
		static int g_framePlayerLastMoved;
		static bool g_bRequestAutoFraming;
		static Controller g_autoTurnController;
		static float  g_autoTurnAmount;
		static HWND g_Window;
		static Player* g_Player;
		static HDC g_HDC;
		static GLuint g_BitmapFontBase;
		static TextureFilmstrip* g_TextureFilmstrips;
		static LPDIRECTINPUTDEVICE8 g_pKeyboard;
		static LPDIRECTINPUT8 g_pDI;
		static SoundSource* g_soundSource;
		static LPDIRECTSOUND g_pDS;
		static int g_ticksIntroStarted;
		static int g_numFramesRun;
		static float g_playerTransAmt, g_playerRotAmt;
		static bool g_bPlayerRotateLeft, g_bPlayerRotateRight, g_bPlayerMoveForwards;
		static bool g_bPlayerMoveBackwards, g_bPlayerMoveUpwards, g_bPlayerMoveDownwards;
		static int g_AutoMovePlayerStatus;
		static bool g_bSeatedOnCouch, g_bTripCarry;
		static float g_autoMovePlayerPt[3]; // X, Y, Z
		static float g_autoMovePlayerRot;
		static Controller g_PlayerXController;
		static Controller g_PlayerYController;
		static Controller g_PlayerZController;
		static Controller g_PlayerRotController;
		static XCursor* g_pCursor;
		static Sprite* g_playerSprite;
		static Sprite* g_CursorSprite;
		static Sprite* CreateCursorSprite();
		static int g_cursorContext;
		static int g_playerGesture;
		static int g_cursorPositionHistory;
		static void InitGlobals();
		static void CreateRoom();
		static Sprite* CreatePanel(int spriteId, uint32_t nameId, int param3, int objectType, uint32_t param5, uint32_t param6, uint32_t param7, uint32_t param8, uint32_t param9, int param10, int param11, uint32_t color1, uint32_t texture, uint32_t color2, uint32_t posX, uint32_t posY, uint32_t posZ, int param16, int param17);
		static void CreateCouchSprite(int spriteId, uint32_t nameId);
		static void CreateCabinetSprite(int spriteId, uint32_t nameId);
		static void CreateSideTableSprite(int spriteId, uint32_t nameId);
		static void CreateWorkTableSprite(int spriteId, uint32_t nameId);
		static void CreateBarSprite(int spriteId, uint32_t nameId);
		static void CreateLuxTableSprite(int spriteId, uint32_t nameId);
		static int GetFullDrinkLevel(bool isTrip);
		static void SetColorFromIndex(int colorIndex);
		static void DrawArbitraryScreenText(const char* text, float xPos, float yPos, int maxWidth, GLfloat scale, float zOffset, int colorIndex);
		static void StartFade(float speed);
		static char* __cdecl LoadTextureFilmstrip(int index);
		static void UpdateFadeEffect();
		static void LoadingMessage();
		static void DrawRedCurtain();
		static int BuildBitmapFont();
		static int InitCinematography();
		static int InitPlayerNavigation();
		static unsigned __int8* __cdecl LoadTexture(const char* srcPath, bool generateAlpha, int minFilter, int magFilter, int wrapMode);
		static void* __cdecl LoadDIBitmap(const char* filename, void** outDibInfo);
		static void CutToBlack();
		static char* InitEnvironment();
		static int* ChangeSize();
	private:
		static int ctr;
		static std::string g_launchErrorString;
		static long double glPrintPolygonalFont(char* __src, ...);
};