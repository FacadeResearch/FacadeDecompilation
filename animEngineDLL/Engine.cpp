#include "Engine.h"
#include <dinput.h>
#include "SoundEntry.h"
#include "AppWindow.h"
#include <gl/GL.h>
#include <string>
#include "EventLog.h"
#include <ctime>
#include "Globals.h"
#include "SoundSource.h"
#include "TextureFilmstrip.h"
#include <GL/glu.h>
#include "GlyphRenderer.h"
#include "Zone.h"
#include "BitmapFileHeader.h"

bool Engine::g_bPrintBetaInfo = false;
bool Engine::g_bDSoundInitialized = false;
bool Engine::g_stagePlayExists = false;

int Engine::g_ticksIntroStarted = 0;
int Engine::fadeCurtainInCtr = 0;
int Engine::g_numFramesRun = 0;
int Engine::g_cursorContext = 0;
int Engine::g_playerGesture = 0;
int Engine::g_cursorPositionHistory = 0; //?? figure out this type

float* Engine::g_gmf = nullptr;

SoundSource* Engine::g_soundSource = nullptr;

Sprite* Engine::g_playerSprite = nullptr;
Sprite* Engine::g_CursorSprite = nullptr;

TextureFilmstrip* Engine::g_TextureFilmstrips = nullptr;

HDC Engine::g_HDC = NULL;
HWND Engine::g_Window = NULL;
XCursor* Engine::g_pCursor = NULL;
Player* Engine::g_Player = NULL;

LPDIRECTINPUTDEVICE8 Engine::g_pKeyboard = NULL;

GLuint Engine::g_BitmapFontBase = 0;

LPDIRECTINPUT8 Engine::g_pDI = NULL;
LPDIRECTSOUND Engine::g_pDS = NULL;

float Engine::g_FadeAlpha = 0;
float Engine::g_FadeSpeed = 0;

bool Engine::g_bAtEndOfLoad = false;

float Engine::g_GlobalAlpha = 1.0f;

bool Engine::bFadeCurtainOut = false;

int32_t Engine::g_startTime = 0;
bool Engine::g_bShouldRenderThisFrame = false;
int Engine::g_SkipFramesCounter = 0;
bool Engine::g_bAssertionThrown = false;
bool Engine::g_bAllQuadLineOutline = false;
bool Engine::g_bAllCircleOutline = false;

int Engine::ctr = 0;

std::string Engine::g_launchErrorString = "";

int Engine::g_numZones = 0;
Zone Engine::g_zones[250];
uint8_t Engine::g_fixedPathPlanGrid[90][90]; // 8100 bytes
int Engine::g_printMapCtr = -1;

bool Engine::g_bCinematographyOn = false;
int Engine::g_frameWhenLastCinematographyMove = 0;
int Engine::g_framePlayerLastMoved = 0;
bool Engine::g_bRequestAutoFraming = false;
float Engine::g_autoTurnAmount = 0.0f;
Controller Engine::g_autoTurnController;

float Engine::g_playerTransAmt = 0.0f;
float Engine::g_playerRotAmt = 0.0f;
bool Engine::g_bPlayerRotateLeft = false;
bool Engine::g_bPlayerRotateRight = false;
bool Engine::g_bPlayerMoveForwards = false;
bool Engine::g_bPlayerMoveBackwards = false;
bool Engine::g_bPlayerMoveUpwards = false;
bool Engine::g_bPlayerMoveDownwards = false;
int Engine::g_AutoMovePlayerStatus = 0;
bool Engine::g_bSeatedOnCouch = false;
bool Engine::g_bTripCarry = false;

float Engine::g_autoMovePlayerPt[3] = { 0.0f, 0.0f, 0.0f };
float Engine::g_autoMovePlayerRot = 0.0f;

Controller Engine::g_PlayerXController;
Controller Engine::g_PlayerYController;
Controller Engine::g_PlayerZController;
Controller Engine::g_PlayerRotController;

int* Engine::ChangeSize() {
	int* result;

	glViewport(0, 0, 1016, 1016);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-100.0, 100.0, -100.0, 100.0, 0.0, 2000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	AppWindow::g_windowWidth = 1016;
	result = &AppWindow::g_windowHeight;
	AppWindow::g_windowHeight = 741;
	return result;
}

void Engine::UpdateFadeEffect() {
	g_FadeAlpha += g_FadeSpeed;

	if (g_FadeAlpha <= 0.0f) {
		g_FadeAlpha = 0.0f;
		g_FadeSpeed = 0.0f;
	}

	if (g_FadeAlpha >= 1.0f) {
		g_FadeAlpha = 1.0f;
		g_FadeSpeed = 0.0f;
	}

	if (g_FadeAlpha > 0.0f) {
		glLoadIdentity();
		glTranslatef(-150.0f, -150.0f, -7.0f);

		glColor4f(0.0f, 0.0f, 0.0f, g_FadeAlpha);

		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(400.0f, 0.0f, 0.0f);
		glVertex3f(400.0f, 250.0f, 0.0f);
		glVertex3f(0.0f, 250.0f, 0.0f);
		glEnd();

		glLoadIdentity();
	}
}

void Engine::DrawRedCurtain() {
	SetColorFromIndex(7);

	glLoadIdentity();
	glTranslatef(-100.0f, -100.0f, -8.0f);

	float x = 200.0f;
	float y = 150.0f;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(0xde1, (GLuint)g_TextureFilmstrips[51].textures[0].handle);
	glBegin(7);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x, y, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0.0f, y, 0.0f);
	glEnd();
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);
}

int Engine::BuildBitmapFont()
{
	if (g_gmf == nullptr) {
		g_gmf = new float[256];
		memset(g_gmf, 0, sizeof(float) * 256);
	}

	uint32_t startTick = GetTickCount();
	dprintf("Building bitmap font...");

	g_BitmapFontBase = glGenLists(256);

	for (int i = 0; i < 256; i++)
	{
		glNewList(g_BitmapFontBase + i, GL_COMPILE);

		if (GlyphRenderer::glyphRenderFunctions[i])
		{
			g_gmf[i] = GlyphRenderer::glyphRenderFunctions[i]();
		}

		glEndList();
	}

	uint32_t duration = GetTickCount() - startTick;

	dprintf("Done building bitmap font (%.3f seconds)", (float)duration * 0.001f);

	return 0;
}

int Engine::InitCinematography()
{
	g_bCinematographyOn = false;
	g_frameWhenLastCinematographyMove = 0;
	g_framePlayerLastMoved = 0;
	g_bRequestAutoFraming = false;

	return g_autoTurnController.InitController(
		&g_autoTurnAmount,
		-1000.0f,
		1000.0f,
		1
	);
}

int Engine::InitPlayerNavigation()
{
	g_playerTransAmt = 0.0f;
	g_playerRotAmt = 0.0f;
	g_bPlayerRotateLeft = false;
	g_bPlayerRotateRight = false;
	g_bPlayerMoveForwards = false;
	g_bPlayerMoveBackwards = false;
	g_bPlayerMoveUpwards = false;
	g_bPlayerMoveDownwards = false;
	g_AutoMovePlayerStatus = 0;
	g_bSeatedOnCouch = false;
	g_bTripCarry = false;

	g_PlayerXController.InitController(&g_autoMovePlayerPt[0], -10000.0f, 10000.0f, 0);
	g_PlayerYController.InitController(&g_autoMovePlayerPt[1], -10000.0f, 10000.0f, 0);
	g_PlayerZController.InitController(&g_autoMovePlayerPt[2], -10000.0f, 10000.0f, 0);

	return g_PlayerRotController.InitController(&g_autoMovePlayerRot, -1000.0f, 1000.0f, 0);
}

unsigned __int8* __cdecl Engine::LoadTexture(const char* srcPath, bool generateAlpha, int minFilter, int magFilter, int wrapMode)
{
	void* dibInfo = nullptr;
	unsigned char* pixels = (unsigned char*)Engine::LoadDIBitmap(srcPath, &dibInfo);

	if (!pixels)
		return 0;

	int width = *((int*)dibInfo + 1);
	int height = *((int*)dibInfo + 2);
	int numPixels = width * height;

	unsigned char* p = pixels;

	for (int i = 0; i < numPixels; ++i)
	{
		unsigned char b = p[0];
		p[0] = p[2];
		p[2] = b;
		p += 3;
	}

	unsigned int target = (height == 1) ? GL_TEXTURE_1D : GL_TEXTURE_2D;

	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(target, texID);

	glTexParameterf(target, GL_TEXTURE_WRAP_S, (float)wrapMode);
	glTexParameterf(target, GL_TEXTURE_WRAP_T, (float)wrapMode);
	glTexParameterf(target, GL_TEXTURE_MAG_FILTER, (float)magFilter);
	glTexParameterf(target, GL_TEXTURE_MIN_FILTER, (float)minFilter);

	if (generateAlpha)
	{
		unsigned char* rgbaPixels = (unsigned char*)malloc(4 * numPixels);

		if (!rgbaPixels)
		{
			free(dibInfo);
			free(pixels);
			return 0;
		}

		unsigned char* src = pixels;
		unsigned char* dst = rgbaPixels;

		for (int j = 0; j < numPixels; ++j)
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = (unsigned char)((src[0] + src[1] + src[2]) / 3);

			dst += 4;
			src += 3;
		}

		if (minFilter == GL_NEAREST || minFilter == GL_LINEAR)
		{
			glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
		}
		else
		{
			if (target == GL_TEXTURE_1D)
				gluBuild1DMipmaps(GL_TEXTURE_1D, GL_RGBA, width, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
			else
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
		}
		free(rgbaPixels);
	}
	else
	{
		if (minFilter == GL_NEAREST || minFilter == GL_LINEAR)
		{
			glTexImage2D(target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		}
		else
		{
			if (target == GL_TEXTURE_1D)
				gluBuild1DMipmaps(GL_TEXTURE_1D, GL_RGB, width, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			else
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		}
	}

	free(dibInfo);
	free(pixels);

	return (unsigned __int8*)texID;
}

void* __cdecl Engine::LoadDIBitmap(const char* filename, void** outDibInfo)
{
	// AI did help with this function as the asm is too messy & I cannot figure out reading bitmaps and textures and bla bla - so hopefully it works :check:
	FILE* file = fopen_UnixPath((char*)filename, "rb");

	if (!file) {
		return nullptr;
	}

	BitmapFileHeader bfh;

	bool headerSuccess = (fread(&bfh.type, 2, 1, file) == 1);

	headerSuccess &= (fread(&bfh.size, 4, 1, file) == 1);
	headerSuccess &= (fread(&bfh.reserved1, 2, 1, file) == 1);
	headerSuccess &= (fread(&bfh.reserved2, 2, 1, file) == 1);
	headerSuccess &= (fread(&bfh.offBits, 4, 1, file) == 1);

	if (!headerSuccess || bfh.type != 19778) { // 'BM'
		fclose(file);
		return nullptr;
	}

	uint8_t* dibInfo = (uint8_t*)malloc(0x428u);

	if (!dibInfo) {
		fclose(file);
		return nullptr;
	}

	memset(dibInfo, 0, 0x428u);
	*outDibInfo = dibInfo;

	int remainingHeader = bfh.offBits - 14;

	bool dataSuccess = true;

	auto SafeRead = [&](void* ptr, size_t size) 
	{
		if (remainingHeader <= 0) {
			return;
		}

		if (fread(ptr, size, 1, file) != 1) {
			dataSuccess = false;
		}

		remainingHeader -= size;
	};

	SafeRead(dibInfo, 4);      // biSize
	SafeRead(dibInfo + 4, 4);  // biWidth
	SafeRead(dibInfo + 8, 4);  // biHeight
	SafeRead(dibInfo + 12, 2); // biPlanes
	SafeRead(dibInfo + 14, 2); // biBitCount
	SafeRead(dibInfo + 16, 4); // biCompression
	SafeRead(dibInfo + 20, 4); // biSizeImage
	SafeRead(dibInfo + 24, 4); // biXPelsPerMeter
	SafeRead(dibInfo + 28, 4); // biYPelsPerMeter
	SafeRead(dibInfo + 32, 4); // biClrUsed
	SafeRead(dibInfo + 36, 4); // biClrImportant

	uint8_t* paletteEntry = dibInfo + 40;
	for (int i = 0; i < 256; ++i) {
		if (remainingHeader > 0) {
			if (fread(paletteEntry, 1, 4, file) != 4) {
				dataSuccess = false;
			}

			remainingHeader -= 4;
		}
		paletteEntry += 4;
	}

	if (remainingHeader != 0 || !dataSuccess) {
		free(dibInfo);
		fclose(file);
		return nullptr;
	}

	uint32_t* infoHeader = (uint32_t*)dibInfo;
	uint32_t imageSize = infoHeader[5];

	if (imageSize == 0) {
		int width = infoHeader[1];
		int height = (int)infoHeader[2];
		if (height < 0) height = -height;

		uint16_t bitCount = *(uint16_t*)(dibInfo + 14);

		int rowSize = (width * bitCount + 7) / 8;
		imageSize = height * rowSize;
	}

	void* pixels = malloc(imageSize);
	if (!pixels) {
		free(dibInfo);
		fclose(file);
		return nullptr;
	}

	if (fread(pixels, 1, imageSize, file) < imageSize) {
		free(pixels);
		free(dibInfo);
		fclose(file);
		return nullptr;
	}

	fclose(file);
	return pixels;
}

void Engine::StartFade(float speed) {
	g_FadeSpeed = -speed;

	if (g_FadeSpeed < 0.0f) {
		g_FadeAlpha = 1.0f;
	}
	else {
		g_FadeAlpha = 0.0f;
	}
}

char* __cdecl Engine::LoadTextureFilmstrip(int index)
{
	TextureFilmstrip* filmstrip = &g_TextureFilmstrips[index];

	char fileNames[20][256];
	memset(fileNames, 0, sizeof(fileNames));

	bool useMipmaps = true;
	int width = 256, height = 256, origW = 256, origH = 256;

	switch (index)
	{
	case 0:  strcpy(fileNames[0], "plant.bmp"); break;
	case 1:  strcpy(fileNames[0], "ashtray.bmp"); break;
	case 2:  strcpy(fileNames[0], "martini_empty_hold.bmp"); break;
	case 3:  strcpy(fileNames[0], "redwinebottle.bmp"); break;
	case 4:  strcpy(fileNames[0], "whitewinebottle.bmp"); break;
	case 5:
		strcpy(fileNames[0], "martini_empty_hold.bmp");
		strcpy(fileNames[1], "martini_classic_hold1.bmp");
		strcpy(fileNames[2], "martini_classic_hold33.bmp");
		strcpy(fileNames[3], "martini_classic_hold66.bmp");
		strcpy(fileNames[4], "martini_classic_hold100.bmp");
		strcpy(fileNames[5], "martini_empty_pdrink.bmp");
		strcpy(fileNames[6], "martini_classic_pdrink1.bmp");
		strcpy(fileNames[7], "martini_classic_pdrink33.bmp");
		strcpy(fileNames[8], "martini_classic_pdrink66.bmp");
		strcpy(fileNames[9], "martini_classic_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 6:
		strcpy(fileNames[0], "martini_empty_hold.bmp");
		strcpy(fileNames[1], "martini_cosmo_hold1.bmp");
		strcpy(fileNames[2], "martini_cosmo_hold33.bmp");
		strcpy(fileNames[3], "martini_cosmo_hold66.bmp");
		strcpy(fileNames[4], "martini_cosmo_hold100.bmp");
		strcpy(fileNames[5], "martini_empty_pdrink.bmp");
		strcpy(fileNames[6], "martini_cosmo_pdrink1.bmp");
		strcpy(fileNames[7], "martini_cosmo_pdrink33.bmp");
		strcpy(fileNames[8], "martini_cosmo_pdrink66.bmp");
		strcpy(fileNames[9], "martini_cosmo_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 7:
		strcpy(fileNames[0], "wine_empty_hold.bmp");
		strcpy(fileNames[1], "wine_white_hold1.bmp");
		strcpy(fileNames[2], "wine_white_hold33.bmp");
		strcpy(fileNames[3], "wine_white_hold66.bmp");
		strcpy(fileNames[4], "wine_white_hold100.bmp");
		strcpy(fileNames[5], "wine_empty_pdrink.bmp");
		strcpy(fileNames[6], "wine_white_pdrink1.bmp");
		strcpy(fileNames[7], "wine_white_pdrink33.bmp");
		strcpy(fileNames[8], "wine_white_pdrink66.bmp");
		strcpy(fileNames[9], "wine_white_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 8:
		strcpy(fileNames[0], "wine_empty_hold.bmp");
		strcpy(fileNames[1], "wine_red_hold1.bmp");
		strcpy(fileNames[2], "wine_red_hold33.bmp");
		strcpy(fileNames[3], "wine_red_hold66.bmp");
		strcpy(fileNames[4], "wine_red_hold100.bmp");
		strcpy(fileNames[5], "wine_empty_pdrink.bmp");
		strcpy(fileNames[6], "wine_red_pdrink1.bmp");
		strcpy(fileNames[7], "wine_red_pdrink33.bmp");
		strcpy(fileNames[8], "wine_red_pdrink66.bmp");
		strcpy(fileNames[9], "wine_red_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 9:
		strcpy(fileNames[0], "pint_empty_hold.bmp");
		strcpy(fileNames[1], "beer_light_hold1.bmp");
		strcpy(fileNames[2], "beer_light_hold33.bmp");
		strcpy(fileNames[3], "beer_light_hold66.bmp");
		strcpy(fileNames[4], "beer_light_hold100.bmp");
		strcpy(fileNames[5], "pint_empty_pdrink.bmp");
		strcpy(fileNames[6], "beer_light_pdrink1.bmp");
		strcpy(fileNames[7], "beer_light_pdrink33.bmp");
		strcpy(fileNames[8], "beer_light_pdrink66.bmp");
		strcpy(fileNames[9], "beer_light_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 10:
		strcpy(fileNames[0], "pint_empty_hold.bmp");
		strcpy(fileNames[1], "beer_dark_hold1.bmp");
		strcpy(fileNames[2], "beer_dark_hold33.bmp");
		strcpy(fileNames[3], "beer_dark_hold66.bmp");
		strcpy(fileNames[4], "beer_dark_hold100.bmp");
		strcpy(fileNames[5], "pint_empty_pdrink.bmp");
		strcpy(fileNames[6], "beer_dark_pdrink1.bmp");
		strcpy(fileNames[7], "beer_dark_pdrink33.bmp");
		strcpy(fileNames[8], "beer_dark_pdrink66.bmp");
		strcpy(fileNames[9], "beer_dark_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 11:
		strcpy(fileNames[0], "liquor_empty_hold.bmp");
		strcpy(fileNames[1], "liquor_scotch_hold1.bmp");
		strcpy(fileNames[2], "liquor_scotch_hold33.bmp");
		strcpy(fileNames[3], "liquor_scotch_hold66.bmp");
		strcpy(fileNames[4], "liquor_scotch_hold100.bmp");
		strcpy(fileNames[5], "liquor_empty_pdrink.bmp");
		strcpy(fileNames[6], "liquor_scotch_pdrink1.bmp");
		strcpy(fileNames[7], "liquor_scotch_pdrink33.bmp");
		strcpy(fileNames[8], "liquor_scotch_pdrink66.bmp");
		strcpy(fileNames[9], "liquor_scotch_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 12:
		strcpy(fileNames[0], "liquor_empty_hold.bmp");
		strcpy(fileNames[1], "liquor_clear_hold1.bmp");
		strcpy(fileNames[2], "liquor_clear_hold33.bmp");
		strcpy(fileNames[3], "liquor_clear_hold66.bmp");
		strcpy(fileNames[4], "liquor_clear_hold100.bmp");
		strcpy(fileNames[5], "liquor_empty_pdrink.bmp");
		strcpy(fileNames[6], "liquor_clear_pdrink1.bmp");
		strcpy(fileNames[7], "liquor_clear_pdrink33.bmp");
		strcpy(fileNames[8], "liquor_clear_pdrink66.bmp");
		strcpy(fileNames[9], "liquor_clear_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 13:
		strcpy(fileNames[0], "pint_empty_hold.bmp");
		strcpy(fileNames[1], "water_hold1.bmp");
		strcpy(fileNames[2], "water_hold33.bmp");
		strcpy(fileNames[3], "water_hold66.bmp");
		strcpy(fileNames[4], "water_hold100.bmp");
		strcpy(fileNames[5], "pint_empty_pdrink.bmp");
		strcpy(fileNames[6], "water_pdrink1.bmp");
		strcpy(fileNames[7], "water_pdrink33.bmp");
		strcpy(fileNames[8], "water_pdrink66.bmp");
		strcpy(fileNames[9], "water_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 14:
		strcpy(fileNames[0], "pint_empty_hold.bmp");
		strcpy(fileNames[1], "cola_hold1.bmp");
		strcpy(fileNames[2], "cola_hold33.bmp");
		strcpy(fileNames[3], "cola_hold66.bmp");
		strcpy(fileNames[4], "cola_hold100.bmp");
		strcpy(fileNames[5], "pint_empty_pdrink.bmp");
		strcpy(fileNames[6], "cola_pdrink1.bmp");
		strcpy(fileNames[7], "cola_pdrink33.bmp");
		strcpy(fileNames[8], "cola_pdrink66.bmp");
		strcpy(fileNames[9], "cola_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 15:
		strcpy(fileNames[0], "pint_empty_hold.bmp");
		strcpy(fileNames[1], "juice_hold1.bmp");
		strcpy(fileNames[2], "juice_hold33.bmp");
		strcpy(fileNames[3], "juice_hold66.bmp");
		strcpy(fileNames[4], "juice_hold100.bmp");
		strcpy(fileNames[5], "pint_empty_pdrink.bmp");
		strcpy(fileNames[6], "juice_pdrink1.bmp");
		strcpy(fileNames[7], "juice_pdrink33.bmp");
		strcpy(fileNames[8], "juice_pdrink66.bmp");
		strcpy(fileNames[9], "juice_pdrink100.bmp");
		for (int i = 0; i < 5; ++i) strcpy(fileNames[10 + i], fileNames[5 + i]);
		break;
	case 16: strcpy(fileNames[0], "painting1.bmp"); useMipmaps = 0; break;
	case 17: strcpy(fileNames[0], "littlepainting.bmp"); useMipmaps = 0; break;
	case 18: strcpy(fileNames[0], "weddingpic.bmp"); useMipmaps = 0; break;
	case 19: strcpy(fileNames[0], "tuscany.bmp"); useMipmaps = 0; break;
	case 20: strcpy(fileNames[0], "gownsketch1.bmp"); useMipmaps = 0; break;
	case 21: strcpy(fileNames[0], "post_its.bmp"); useMipmaps = 0; break;
	case 22: strcpy(fileNames[0], "gownsketch4.bmp"); useMipmaps = 0; break;
	case 23: strcpy(fileNames[0], "phoneontable.bmp"); strcpy(fileNames[1], "phoneheld.bmp"); break;
	case 24: strcpy(fileNames[0], "answeringmachine.bmp"); break;
	case 25: strcpy(fileNames[0], "eightball.bmp"); strcpy(fileNames[1], "eightball2.bmp"); break;
	case 26: strcpy(fileNames[0], "brassbull.bmp"); break;
	case 27: strcpy(fileNames[0], "trinket1.bmp"); break;
	case 28: strcpy(fileNames[0], "trinket2.bmp"); break;
	case 29: strcpy(fileNames[0], "trinket3.bmp"); break;
	case 30: strcpy(fileNames[0], "trinket4.bmp"); break;
	case 31: strcpy(fileNames[0], "trinket5.bmp"); break;
	case 32: strcpy(fileNames[0], "trinket6.bmp"); break;
	case 33: strcpy(fileNames[0], "trinket7.bmp"); break;
	case 34: strcpy(fileNames[0], "trinket8.bmp"); break;
	case 35:
		strcpy(fileNames[0], "curtain.bmp");
		useMipmaps = false;
		width = 512;
		origW = 512;
		break;
	case 36: strcpy(fileNames[0], "frontdoor.bmp"); useMipmaps = 0; break;
	case 37: strcpy(fileNames[0], "bedroomdoor.bmp"); useMipmaps = 0; break;
	case 38: strcpy(fileNames[0], "elevator.bmp"); useMipmaps = 0; break;
	case 39: strcpy(fileNames[0], "elevatorButton.bmp"); useMipmaps = 0; width = 128; height = 128; origW = 128; origH = 128; break;
	case 40: strcpy(fileNames[0], "yellowfont.bmp"); width = 512; height = 512; origW = 512; origH = 512; break;
	case 41: strcpy(fileNames[0], "yellowfont2.bmp"); width = 512; height = 512; origW = 512; origH = 512; break;
	case 42:
		strcpy(fileNames[0], "eightballMessages.bmp");
		strcpy(fileNames[1], "eightballFadein.bmp");
		useMipmaps = false; width = 83; height = 83; origW = 512; origH = 512;
		break;
	case 43: strcpy(fileNames[0], "blackstripes.bmp"); useMipmaps = 0; break;
	case 44: strcpy(fileNames[0], "whitecabinetdoor.bmp"); useMipmaps = 0; break;
	case 45: strcpy(fileNames[0], "whitecabinetdoor2.bmp"); useMipmaps = 0; break;
	case 46: strcpy(fileNames[0], "redCurtain.bmp"); useMipmaps = 0; width = 512; height = 512; origW = 512; origH = 512; break;
	case 47: strcpy(fileNames[0], "instr_keyboard.bmp"); useMipmaps = 0; break;
	case 48: strcpy(fileNames[0], "instr_arrowKeys.bmp"); useMipmaps = 0; break;
	case 49: strcpy(fileNames[0], "instr_mouse.bmp"); useMipmaps = 0; break;
	case 50: strcpy(fileNames[0], "label_logo.bmp"); useMipmaps = 0; width = 128; height = 128; origW = 128; origH = 128; break;
	case 51: strcpy(fileNames[0], "redCurtain.bmp"); useMipmaps = 0; width = 512; height = 512; origW = 512; origH = 512; break;
	case 52: strcpy(fileNames[0], "tipjar.bmp"); useMipmaps = 0; break;
	default:
		EventLog::ShouldntBeHere("unknown texture filmstrip");
		break;
	}

	filmstrip->numTextures = 0;

	for (int i = 0; i < 20; ++i) {
		if (fileNames[i][0] == '\0') continue;

		char path[280];
		sprintf(path, "textures\\%s", fileNames[i]);
		strcpy(filmstrip->textures[i].path, path);
		filmstrip->numTextures++;

		int handle = -1;
		for (int fs = 0; fs < 53; ++fs) {
			if (g_TextureFilmstrips[fs].isLoaded) {
				for (int t = 0; t < 20; ++t) {
					if (g_TextureFilmstrips[fs].textures[t].path[0] != '\0' &&
						strcmp(path, g_TextureFilmstrips[fs].textures[t].path) == 0) {
						handle = g_TextureFilmstrips[fs].textures[t].handle;
						break;
					}
				}
			}
			if (handle != -1) break;
		}

		if (handle != -1) {
			filmstrip->textures[i].handle = handle;
		}
		else {
			filmstrip->textures[i].handle = (int)LoadTexture(path, useMipmaps, 9729, 9729, 9729);
		}
	}

	filmstrip->useMipmaps = useMipmaps;
	filmstrip->width = width;
	filmstrip->height = height;
	filmstrip->originalWidth = origW;
	filmstrip->originalHeight = origH;
	filmstrip->isLoaded = 1;

	return (char*)filmstrip;
}

long double Engine::glPrintPolygonalFont(char* __src, ...) {
	unsigned int originalLength;
	unsigned int nonNullTerminatedLength;
	unsigned int iterator;
	float temporaryWidth;
	float totalWidth;
	char __dst[280];

	if (!__src) {
		return 0.0f;
	}

	strcpy(__dst, __src);

	originalLength = (unsigned int)strlen(__dst) + 1;
	nonNullTerminatedLength = originalLength - 1;

	if (originalLength == 1) {
		totalWidth = 0.0;
	}
	else {
		iterator = 0;
		temporaryWidth = 0.0;
		do {
			temporaryWidth = temporaryWidth + g_gmf[(unsigned char)__dst[iterator]];
			totalWidth = temporaryWidth;
			++iterator;
		} while (iterator < nonNullTerminatedLength);
	}

	glPushAttrib(GL_LIST_BIT);
	glListBase(g_BitmapFontBase);
	glCallLists(nonNullTerminatedLength, GL_UNSIGNED_BYTE, __src);
	glPopAttrib();

	return totalWidth;
}

void Engine::DrawArbitraryScreenText(const char* text, float xPos, float yPos, int maxWidth, GLfloat scale, float zOffset, int colorIndex) {
	int textLen = strlen(text);

	if (textLen <= 0) return;

	SetColorFromIndex(colorIndex);

	int lineIndex = 0;
	int charProgress = 0;

	while (charProgress < textLen) {
		glLoadIdentity();

		float drawY = ((float)((float)lineIndex * scale) * -0.69999999f) + yPos;

		glTranslatef(xPos - 25.0f, drawY, zOffset - 2.0f);
		glScalef(scale, scale, scale);

		float currentLineWidth = 0.0f;

		while (charProgress < textLen) {
			char c = text[charProgress];
			char charStr[2] = { c, '\0' };

			int prevWidthInt = (int)currentLineWidth;
			float charWidth = (float)glPrintPolygonalFont(charStr);
			currentLineWidth += charWidth;

			if (c == (char)-25) {
				currentLineWidth = (float)prevWidthInt;
			}

			charProgress++;

			if (charProgress > 0 && c == 32) {
				if (currentLineWidth >= (float)((float)maxWidth * 0.1f)) {
					lineIndex++;
					goto next_line;
				}
			}
		}
	next_line:;
	}
	glLoadIdentity();
}

void Engine::CutToBlack() {
	g_FadeAlpha = 1.0f;
	g_FadeSpeed = 0.0f;
}

char* Engine::InitEnvironment()
{
	g_numZones = 0;

	for (int i = 0; i < 250; ++i) {
		// some fucking weird offset at 80 bytes is set to -1. do the same
		g_zones[i].someFlag = -1;
		memset(g_zones[i].unknownData, 0, 80); //we dont use the rest so set it free. i guess
	}

	for (int y = 0; y < 90; ++y) {
		for (int x = 0; x < 90; ++x) {
			g_fixedPathPlanGrid[y][x] = 0;
		}
	}

	g_printMapCtr = -1;

	return (char*)&g_fixedPathPlanGrid[89][89] + 1;
}

void Engine::LoadingMessage() {
	//data_101c7618

	if (Engine::g_ticksIntroStarted < 0) {
		DrawRedCurtain();

		//(data_106988e0
		if (Engine::fadeCurtainInCtr == 0) {
			CutToBlack();
			StartFade(0.015);
		}

		if (Engine::fadeCurtainInCtr >= 60) {
			if (Engine::g_bAtEndOfLoad) {
				if (!bFadeCurtainOut) {
					bFadeCurtainOut = 1;
					StartFade(-0.015);
				}
			}
			else {
				std::string mainStatus = "Loading";
				std::string subStatus = "(May take up to 60 seconds)";

				if (Engine::g_numFramesRun <= 90) {
					mainStatus = "Loading Façade...";
				}
				else {
					if (ctr <= 30) {
						mainStatus = "Loading Façade.";
					}
					else if (ctr <= 45) {
						mainStatus = "Loading Façade..";
					}
					else if (ctr <= 60) {
						mainStatus = "Loading Façade...";
					}
					else mainStatus = "Loading Façade";

					++ctr;

					if (ctr == 104) {
						ctr = 0;
					}
				}

				if (!g_launchErrorString.empty()) {
					mainStatus = "Load Error";
					subStatus = g_launchErrorString;
				}

				DrawArbitraryScreenText(mainStatus.c_str(), -10.0f, 0.0f, 200, 10.0f, 0.0f, 7);
				DrawArbitraryScreenText(subStatus.c_str(), 0.0f, -10.0f, 200, 5.0f, 0.0f, 7);
				DrawArbitraryScreenText("www.interactivestory.net", 2.0, -50.0, 350, 5.0, 0.0, 7);
				DrawArbitraryScreenText("(c)opyright 2005-2006 Procedural Arts", -11.0, -56.0, 350, 5.0, 0.0, 7);
				DrawArbitraryScreenText("an [auto mata] release ", 4.5, -62.0, 350, 5.0, 0.0, 7);
				DrawArbitraryScreenText("v1.1", 19.5, -68.0, 350, 5.0, 0.0, 7);

				if (Engine::g_bPrintBetaInfo) {
					DrawArbitraryScreenText("Debug", 18.5f, -74.0, 350, 5.0f, 0.0f, 7);
				}
			}
		}
		else {
			Engine::fadeCurtainInCtr++;
		}
	}
}

void Engine::SetColorFromIndex(int colorIndex) {
	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	float alpha = g_GlobalAlpha;

	switch (colorIndex) {
	case 0:
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		break;
	case 1:
		red = 0.959999979f;
		green = 0.870000005f;
		blue = 0.70f;
		break;
	case 2:
		red = 0.628000021f;
		green = 0.734000027f;
		blue = 0.921000004f;
		break;
	case 3:
		red = 0.519999981f;
		green = 0.514999986f;
		blue = 0.519999981f;
		break;
	case 4:
		red = 0.200000003f;
		green = 0.200000003f;
		blue = 0.300000012f;
		break;
	case 5:
		red = 0.419999987f;
		green = 0.414999992f;
		blue = 0.419999987f;
		break;
	case 6:
		red = 0.428000003f;
		green = 0.533999979f;
		blue = 0.721000016f;
		break;
	case 7:
		red = 1.0f;
		green = 1.0f;
		blue = 1.0f;
		break;
	case 8:
		red = 0.25f;
		green = 0.150000006f;
		blue = 0.100000001f;
		break;
	case 9:
		red = 1.0f;
		green = 0.75f;
		blue = 0.700000003f;
		break;
	case 0xA:
		red = 1.0f;
		green = 0.800000012f;
		blue = 0.0f;
		break;
	case 0xB:
		red = 1.0f;
		green = 0.600000024f;
		blue = 0.0f;
		break;
	case 0xC:
		red = 1.0f;
		green = 1.0f;
		blue = 0.150000006f;
		break;
	case 0xD:
		red = 0.829999983f;
		green = 0.829999983f;
		blue = 0.829999983f;
		break;
	case 0xE:
		red = 0.75f;
		green = 1.0f;
		blue = 0.75f;
		break;
	case 0xF:
		red = 1.0f;
		green = 1.0f;
		blue = 1.0f;
		break;
	case 0x10:
		red = 1.0f;
		green = 1.0f;
		blue = 1.0f;
		break;
	case 0x11:
		red = 1.0f;
		green = 1.0f;
		blue = 1.0f;
		break;
	case 0x12:
		red = 1.0f;
		green = 1.0f;
		blue = 0.200000003f;
		break;
	case 0x13:
		red = 0.600000024f;
		green = 0.600000024f;
		blue = 0.600000024f;
		break;
	case 0x14:
		red = 0.5f;
		green = 0.5f;
		blue = 0.5f;
		break;
	case 0x15:
		red = 0.600000024f;
		green = 0.700000003f;
		blue = 1.0f;
		break;
	case 0x16:
		red = 0.620000005f;
		green = 0.529999971f;
		blue = 0.569999993f;
		break;
	case 0x17:
		red = 0.419999987f;
		green = 0.330000013f;
		blue = 0.370000005f;
		break;
	case 0x18:
		red = 0.273000002f;
		green = 0.222000003f;
		blue = 0.191f;
		break;
	case 0x19:
		red = 0.829999983f;
		green = 0.769999981f;
		blue = 0.700000003f;
		break;
	case 0x1A:
		red = 0.400000006f;
		green = 0.449999988f;
		blue = 0.400000006f;
		break;
	case 0x1B:
		red = 0.349999994f;
		green = 0.400000006f;
		blue = 0.349999994f;
		break;
	case 0x1C:
		red = 0.600000024f;
		green = 0.5f;
		blue = 0.300000012f;
		break;
	case 0x1D:
		red = 0.5f;
		green = 0.400000006f;
		blue = 0.25f;
		break;
	case 0x1E:
		red = 0.150000006f;
		green = 0.800000012f;
		blue = 0.75f;
		break;
	case 0x1F:
		red = 0.5f;
		green = 1.0f;
		blue = 0.550000012f;
		break;
	case 0x20:
		red = 0.769999981f;
		green = 0.730000019f;
		blue = 0.700000003f;
		break;
	case 0x21:
		red = 0.200000003f;
		green = 0.200000003f;
		blue = 0.400000006f;
		alpha = 0.200000003f;
		break;
	case 0x22:
		red = 1.0f;
		green = 1.0f;
		blue = 0.800000012f;
		break;
	case 0x23:
		red = 0.150000006f;
		green = 0.850000024f;
		blue = 0.779999971f;
		break;
	case 0x24:
		red = 0.200000003f;
		green = 0.200000003f;
		blue = 0.200000003f;
		alpha = 0.300000012f;
		break;
	case 0x25:
		red = 1.0f;
		green = 1.0f;
		blue = 0.150000006f;
		break;
	case 0x26:
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		break;
	case 0x27:
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
		break;
	case 0x28:
		red = 0.0f;
		green = 0.0f;
		blue = 1.0f;
		break;
	case 0x29:
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
		break;
	case 0x2A:
		red = 1.0f;
		green = 0.0f;
		blue = 1.0f;
		break;
	case 0x2B:
		red = 0.600000024f;
		green = 0.800000012f;
		blue = 1.0f;
		break;
	case 0x2C:
		red = 0.330000013f;
		green = 0.159999996f;
		blue = 0.0199999996f;
		break;
	case 0x2D:
		red = 0.430000007f;
		green = 0.25999999f;
		blue = 0.119999997f;
		break;
	case 0x2E:
		red = 0.529999971f;
		green = 0.360000014f;
		blue = 0.219999999f;
		break;
	case 0x2F:
		red = 0.629999995f;
		green = 0.460000008f;
		blue = 0.319999993f;
		break;
	case 0x30:
		red = 0.230000004f;
		green = 0.159999996f;
		blue = 0.0199999996f;
		break;
	case 0x31:
		red = 0.330000013f;
		green = 0.25999999f;
		blue = 0.119999997f;
		break;
	case 0x32:
		red = 0.430000007f;
		green = 0.360000014f;
		blue = 0.219999999f;
		break;
	case 0x33:
		red = 0.479999989f;
		green = 0.409999996f;
		blue = 0.270000011f;
		break;
	case 0x34:
		red = 0.0500000007f;
		green = 0.0500000007f;
		blue = 0.0500000007f;
		break;
	case 0x35:
		red = 0.100000001f;
		green = 0.100000001f;
		blue = 0.100000001f;
		break;
	case 0x36:
		red = 0.150000006f;
		green = 0.150000006f;
		blue = 0.150000006f;
		break;
	case 0x37:
		red = 0.200000003f;
		green = 0.200000003f;
		blue = 0.200000003f;
		break;
	case 0x38:
		red = 0.25f;
		green = 0.25f;
		blue = 0.25f;
		break;
	case 0x39:
		red = 0.300000012f;
		green = 0.300000012f;
		blue = 0.300000012f;
		break;
	case 0x3A:
		red = 0.349999994f;
		green = 0.349999994f;
		blue = 0.349999994f;
		break;
	case 0x3B:
		red = 0.400000006f;
		green = 0.400000006f;
		blue = 0.400000006f;
		break;
	case 0x3C:
		red = 0.449999988f;
		green = 0.449999988f;
		blue = 0.449999988f;
		break;
	default:
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		break;
	}

	glColor4f(red, green, blue, alpha);
}

Sprite* Engine::CreateCursorSprite()
{
	Sprite* cursorSprite = new Sprite();

	//to-do figure out that weird "cursor" initialization

	g_cursorContext = 0;
	g_playerGesture = 0;
	g_cursorPositionHistory = 0;

	return cursorSprite;
}

void Engine::InitGlobals() {
	g_TextureFilmstrips = new TextureFilmstrip[18448];

	dprintf("\n---------------------------\nInitting globals...");
	srand((unsigned int)time(NULL));

	g_startTime = GetTickCount();
	g_numFramesRun = 0;
	g_bShouldRenderThisFrame = 1;
	g_SkipFramesCounter = 0;
	g_bAssertionThrown = 0;
	g_launchErrorString = "";
	g_bAllQuadLineOutline = 0;
	g_bAllCircleOutline = 0;
	fadeCurtainInCtr = 0;
	g_ticksIntroStarted = -1;

	for (int i = 40; i != 53; ++i)
		LoadTextureFilmstrip(i);

	InitEnvironment();
	InitCinematography();
	InitPlayerNavigation();

	g_playerSprite = new Sprite();
	g_CursorSprite = CreateCursorSprite();

	//CreateRoom();
	// (*(void (__cdecl **)(int, _DWORD))(*(_DWORD *)g_CursorSprite + 36))(g_CursorSprite, 0);
	/*
	(*(void (__cdecl **)(Sprite *, int, const char *, _DWORD, int, int))(*(_DWORD *)v2 + 8))(v2, 2, "player", 0, 1, -1);
	  (*(void (__cdecl **)(int, _DWORD))(*(_DWORD *)g_PlayerSprite + 36))(g_PlayerSprite, 0);
	*/

	/*

	  
	  InitGraceSprite();
	  InitTripSprite();
	  InitTransRotScale(0, 0);
	  InitCollisionZonesAndPathPlanGrid();
	  InitPathPlanning();
	  InitCircleDrawing();
	  InitQueuedUpRequestedPerformance();
	  SoundSource::InitSoundSource((SoundSource *)&g_soundSource, "sounds\\global\\globalsnd.txt");
	  g_soundSourceID = SoundMgr::AddSoundSource((SoundMgr *)&g_soundMgr, (SoundSource *)&g_soundSource);
	  BuildBitmapFont();
	  v3 = (XCursor *)operator new(0x4Cu);
	  XCursor::XCursor(v3);
	  XCursor::theirCursor = v3;
	  XCursor::InitCursor(v3);
	  XCursor::JumpCutToCursor(XCursor::theirCursor, 0, 1);
	  result = 0;
	  memset(&g_curTextInput, 0, 0x20u);
	  g_curTextCharInput = 0;


	  Fuck there is so much stuff just for the initialization HELP
	*/


	g_Player = new Player();
	g_Player->InitPlayer("player", 0, 1);

	g_soundSource = new SoundSource();
	g_soundSource->InitSoundSource((char*)"sounds\\global\\globalsnd.txt");

	BuildBitmapFont();

	g_pCursor = new XCursor();
	g_pCursor->InitCursor();
	XCursor::theirCursor = g_pCursor;

	HICON hIcon = LoadIconA(GetModuleHandle(NULL), (LPCSTR)0x83);
	SendMessage(g_Window, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}