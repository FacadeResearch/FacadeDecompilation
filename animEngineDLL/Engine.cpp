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

	CreateRoom();
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

Sprite* Engine::CreatePanel(int spriteId, uint32_t nameId, int param3, int objectType, uint32_t param5, uint32_t param6, uint32_t param7, uint32_t param8, uint32_t param9, int param10, int param11, uint32_t color1, uint32_t texture, uint32_t color2, uint32_t posX, uint32_t posY, uint32_t posZ, int param16, int param17)
{
	(void)spriteId;
	(void)nameId;
	(void)param3;
	(void)objectType;
	(void)param5;
	(void)param6;
	(void)param7;
	(void)param8;
	(void)param9;
	(void)param10;
	(void)param11;
	(void)color1;
	(void)texture;
	(void)color2;
	(void)posX;
	(void)posY;
	(void)posZ;
	(void)param16;
	(void)param17;
	return new Sprite();
}

void Engine::CreateCouchSprite(int spriteId, uint32_t nameId)
{
	(void)spriteId;
	(void)nameId;
	(void)new Sprite();
}

void Engine::CreateCabinetSprite(int spriteId, uint32_t nameId)
{
	(void)spriteId;
	(void)nameId;
	(void)new Sprite();
}

void Engine::CreateSideTableSprite(int spriteId, uint32_t nameId)
{
	(void)spriteId;
	(void)nameId;
	(void)new Sprite();
}

void Engine::CreateWorkTableSprite(int spriteId, uint32_t nameId)
{
	(void)spriteId;
	(void)nameId;
	(void)new Sprite();
}

void Engine::CreateBarSprite(int spriteId, uint32_t nameId)
{
	(void)spriteId;
	(void)nameId;
	(void)new Sprite();
}

void Engine::CreateLuxTableSprite(int spriteId, uint32_t nameId)
{
	(void)spriteId;
	(void)nameId;
	(void)new Sprite();
}

int Engine::GetFullDrinkLevel(bool isTrip)
{
	(void)isTrip;
	return 0;
}

void Engine::CreateRoom()
{
	const uint32_t unaff_EBX = 0;

	CreateCouchSprite(3, unaff_EBX + 0x38523);
	CreateCabinetSprite(6, unaff_EBX + 0x38673);
	CreateSideTableSprite(4, unaff_EBX + 0x3867b);
	CreateWorkTableSprite(5, unaff_EBX + 0x37f5b);
	CreateBarSprite(7, unaff_EBX + 0x37f57);
	CreateLuxTableSprite(8, unaff_EBX + 0x38687);
	CreatePanel(0x21, unaff_EBX + 0x38697, 1, 0xffffffff, 0x42f00000, 0x42f00000, 0, 0, 0, 1, 1, 0xfffffffe, 0,
		0xffffffff, 0x43200000, 0x42700000, 0x42820000, 0, 0);
	CreatePanel(9, unaff_EBX + 0x3863b, 0, 0xffffffff, 0x41200000, 0x40a00000, 0, 0, 0, 1, 1,
		0xfffffffe, 1, 0xffffffff, 0x43250000, 0x42700000, 0x41200000, 0, 0);
	CreatePanel(10, unaff_EBX + 0x35ae7, 1, 3, 0x41800000, 0x41800000, 0, 0, 0, 1, 1, 0xfffffffe,
		0x17, 0xffffffff, 0x43110000, 0x42700000, 0xc2700000, 0, 0);
	CreatePanel(0xb, unaff_EBX + 0x35aef, 1, 4, 0x41800000, 0x41800000, 0, 0, 0, 1, 1, 0xfffffffe
		, 0x18, 0xffffffff, 0x432a0000, 0x42700000, 0xc28c0000, 0, 0);
	CreatePanel(0xc, unaff_EBX + 0x38643, 1, 0xffffffff, 0x41c80000, 0x41c80000, 0, 0, 0, 1, 1,
		0xfffffffe, 3, 0xffffffff, 0xc3390000, 0x42a00000, 0xc2d80000, 0, 0);
	CreatePanel(0xd, unaff_EBX + 0x38653, 1, 0xffffffff, 0x41c80000, 0x41c80000, 0, 0, 0, 1, 1,
		0xfffffffe, 4, 0xffffffff, 0xc3410000, 0x42a00000, 0xc2e00000, 0, 0);
	CreatePanel(0xe, unaff_EBX + 0x3869f, 1, 0xffffffff, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 2, 0xffffffff, 0xc3380000, 0x42240000, 0xc3890000, 0, 0);
	CreatePanel(0xf, unaff_EBX + 0x386af, 1, 0xffffffff, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 2, 0xffffffff, 0xc3370000, 0x42240000, 0xc38f0000, 0, 0);
	CreatePanel(0x10, unaff_EBX + 0x386bf, 1, 0xffffffff, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 2, 0xffffffff, 0xc3378000, 0x42240000, 0xc3948000, 0, 0);
	CreatePanel(0x11, unaff_EBX + 0x386cf, 1, 0xffffffff, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 2, 0xffffffff, 0xc3370000, 0x42240000, 0xc39a0000, 0, 0);
	CreatePanel(0x12, unaff_EBX + 0x386df, 0, 0xffffffff, 0x41c80000, 0x41c80000, 0, 0, 0, 1, 1,
		0xfffffffe, 3, 0xffffffff, 0xc2c80000, 0xc2c80000, 0xc2c80000, 0, 0);
	CreatePanel(0x13, unaff_EBX + 0x386f3, 0, 0xffffffff, 0x41c80000, 0x41c80000, 0, 0, 0, 1, 1,
		0xfffffffe, 4, 0xffffffff, 0xc2c80000, 0xc2c80000, 0xc2c80000, 0, 0);
	CreatePanel(0x14, unaff_EBX + 0x38707, 0, 1, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 6, 0xffffffff, 0xc32a0000, 0x42920000, 0xc2dc0000, 0, 0);
	CreatePanel(0x15, unaff_EBX + 0x38717, 0, 1, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 6, 0xffffffff, 0xc2c80000, 0xc2c80000, 0xc2c80000, 0, 0);
	CreatePanel(0x16, unaff_EBX + 0x38727, 0, 1, 0x41400000, 0x41400000, 0, 0, 0, 1, 1,
		0xfffffffe, 5, 0xffffffff, 0xc2c80000, 0xc2c80000, 0xc2c80000, 0, 0);
	CreatePanel(0x22, unaff_EBX + 0x38737, 1, 0xffffffff, 0x42500000, 0x428c0000, 0,
		0x42b40000, 0, 1, 0, 7, 0x10, 0, 0x433e0000, 0x42c80000, 0xc3be0000, 0, 0);
	CreatePanel(0x23, unaff_EBX + 0x3874f, 1, 0xffffffff, 0x41a00000, 0x42000000, 0,
		0x42b40000, 0, 1, 0, 7, 0x11, 0, 0x433e0000, 0x42ec0000, 0xc3a78000, 0, 0);
	CreatePanel(0x24, unaff_EBX + 0x3875f, 0, 0xffffffff, 0x42200000, 0x42480000, 0, 0x43870000, 0, 1, 0, 7, 0x11,
		0, 0x43570000, 0x42c80000, 0x43c80000, 0, 0);
	CreatePanel(0x25, unaff_EBX + 0x3876b, 1, 0xffffffff, 0x42480000, 0x42c80000, 0x42b40000, 0, 0, 0, 0, 7, 0x23,
		0, 0x42aa0000, 0x40400000, 0xc3af0000, 0, 0);
	CreatePanel(0x26, unaff_EBX + 0x3876f, 1, 0xffffffff, 0x42700000, 0x428c0000, 0,
		0x42b40000, 0, 1, 0, 7, 0x12, 0, 0x433e0000, 0x42b40000, 0xc3110000, 0, 0);
	CreatePanel(0x27, unaff_EBX + 0x3877f, 1, 0xffffffff, 0x42000000, 0x42000000, 0,
		0x43870000, 0, 1, 0, 7, 0x13, 0xffffffff, 0xc3470000, 0x42b40000, 0xc2700000, 0,
		0);
	CreatePanel(0x28, unaff_EBX + 0x3878f, 1, 0xffffffff, 0x42000000, 0x42000000, 0,
		0x42b40000, 0, 1, 0, 0xfffffffe, 0x14, 0xffffffff, 0x433e0000, 0x42b40000, 0, 0,
		0);
	CreatePanel(0x29, unaff_EBX + 0x387a3, 1, 0xffffffff, 0x42100000, 0x42100000, 0,
		0x42b40000, 0, 1, 0, 0xfffffffe, 0x15, 0xffffffff, 0x433e0000, 0x42480000,
		0xc1e00000, 0, 0);
	CreatePanel(0x2a, unaff_EBX + 0x387bb, 0, 0xffffffff, 0x41c80000, 0x41c80000, 0x42b40000
		, 0, 0, 1, 0, 0xfffffffe, 0x14, 0xffffffff, 0x43200000, 0x42680000, 0xc2700000, 0
		, 0);
	CreatePanel(0x2b, unaff_EBX + 0x387d3, 1, 0xffffffff, 0x41f00000, 0x41f00000, 0,
		0x42b40000, 0, 1, 0, 0xfffffffe, 0x16, 0xffffffff, 0x433e0000, 0x42a60000,
		0xc20c0000, 0, 0);
	CreatePanel(0x2c, unaff_EBX + 0x387eb, 0, 0xffffffff, 0x42000000, 0x42000000, 0,
		0x42b40000, 0, 1, 0, 0xfffffffe, 0x14, 0xffffffff, 0x433e0000, 0x42be0000,
		0x41c80000, 0, 0);
	CreatePanel(0x2d, unaff_EBX + 0x387fb, 0, 0xffffffff, 0x41c80000, 0x41c80000, 0x42b40000
		, 0, 0, 1, 0, 0xfffffffe, 0x14, 0xffffffff, 0x43200000, 0x42680000, 0xc1700000, 0
		, 0);
	CreatePanel(0x17, unaff_EBX + 0x38667, 1, 0xffffffff, 0x41000000, 0x41000000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x19, 0xffffffff, 0xc3150000, 0x428e0000, 0xc32a0000, 0, 0);
	CreatePanel(0x36, unaff_EBX + 0x3880b, 0, 2, 0x40a00000, 0x40a00000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x2a, 0xffffffff, 0, 0, 0, 0, 0);
	CreatePanel(0x18, unaff_EBX + 0x3881f, 1, 0xffffffff, 0x41800000, 0x41800000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x1a, 0xffffffff, 0xc3390000, 0x42960000, 0xc3960000, 0, 0);
	CreatePanel(0x19, unaff_EBX + 0x3882b, 1, 0xffffffff, 0x41e00000, 0x41e00000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x1b, 0xffffffff, 0xc3390000, 0x42400000, 0xc3e10000, 0, 0);
	CreatePanel(0x1a, unaff_EBX + 0x38837, 1, 0xffffffff, 0x41e00000, 0x41e00000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x1c, 0xffffffff, 0xc33a0000, 0x42400000, 0xc3d48000, 0, 0);
	CreatePanel(0x1b, unaff_EBX + 0x38843, 1, 0xffffffff, 0x41a00000, 0x41a00000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x1d, 0xffffffff, 0xc3388000, 0x42380000, 0xc3c80000, 0, 0);
	CreatePanel(0x1c, unaff_EBX + 0x3884f, 1, 0xffffffff, 0x41800000, 0x41800000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x1e, 0xffffffff, 0xc3390000, 0x42960000, 0xc3dd8000, 0, 0);
	CreatePanel(0x1d, unaff_EBX + 0x3885b, 1, 0xffffffff, 0x41800000, 0x41800000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x1f, 0xffffffff, 0xc33a0000, 0x42960000, 0xc3ca8000, 0, 0);
	CreatePanel(0x1e, unaff_EBX + 0x38867, 1, 0xffffffff, 0x41a00000, 0x41a00000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x20, 0xffffffff, 0xc33a0000, 0x42c40000, 0xc3c78000, 0, 0);
	CreatePanel(0x1f, unaff_EBX + 0x38873, 1, 0xffffffff, 0x41800000, 0x41800000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x21, 0xffffffff, 0xc33a0000, 0x42c00000, 0xc3d40000, 0, 0);
	CreatePanel(0x20, unaff_EBX + 0x3887f, 1, 0xffffffff, 0x41800000, 0x41800000, 0, 0, 0, 1, 1,
		0xfffffffe, 0x22, 0xffffffff, 0xc33a0000, 0x42c00000, 0xc3e10000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x3888b, 1, 0xffffffff, 0x428c0000, 0x43200000, 0, 0, 0, 1, 0, 0,
		0xffffffff, 0xffffffff, 0xc3250000, 0x42a00000, 0xc3fa0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x3889b, 1, 0xffffffff, 0x40d00000, 0x43200000, 0, 0, 0, 1, 0, 0,
		0xffffffff, 0xffffffff, 0xc2840000, 0x42a00000, 0xc3fa0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x388af, 1, 0xffffffff, 0x40d00000, 0x43200000, 0, 0, 0, 1, 0, 0,
		0xffffffff, 0xffffffff, 0x40c00000, 0x42a00000, 0xc3fa0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x388c3, 1, 0xffffffff, 0x41f00000, 0x43200000, 0, 0, 0, 1, 0, 0,
		0xffffffff, 0xffffffff, 0x42aa0000, 0x42a00000, 0xc3fa0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x388d3, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0, 0, 1, 0, 0,
		0xffffffff, 0xffffffff, 0x43160000, 0x42a00000, 0xc3fa0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x388e3, 1, 0xffffffff, 0x43480000, 0x41200000, 0, 0, 0, 1, 0, 0,
		0xffffffff, 0xffffffff, 0xc1f00000, 0x431b0000, 0xc3fa0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x388f7, 1, 0xffffffff, 0x428c0000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0xc3480000, 0x42a00000, 0xc0a00000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38907, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0xc3480000, 0x42a00000, 0xc2b40000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38917, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0xc3480000, 0x42a00000, 0xc33e0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38927, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0xc3480000, 0x42a00000, 0xc3910000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38937, 1, 0xffffffff, 0x42700000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0xc3480000, 0x42a00000, 0xc3b90000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38947, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0xc3480000, 0x42a00000, 0xc3e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38957, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0x43480000, 0x42a00000, 0x42480000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38967, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0x43480000, 0x42a00000, 0xc2480000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38977, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0x43480000, 0x42a00000, 0xc3160000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38987, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0x43480000, 0x42a00000, 0xc37a0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38997, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0x43480000, 0x42a00000, 0xc3af0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x389a7, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0, 0
		, 0xffffffff, 0xffffffff, 0x43480000, 0x42a00000, 0xc3e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x389b7, 1, 0xffffffff, 0x42840000, 0x43160000, 0, 0, 0, 1, 0,
		0x21, 0xffffffff, 0xffffffff, 0xc2ca0000, 0x429a0000, 0xc3f90000, 1, 0);
	CreatePanel(0x2e, unaff_EBX + 0x389c7, 1, 0xffffffff, 0x42840000, 0x43160000, 0, 0, 0, 1, 0, 0x21,
		0xffffffff, 0xffffffff, 0xc1f00000, 0x429a0000, 0xc3f90000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x389d7, 1, 0xffffffff, 0x42840000, 0x43160000, 0, 0, 0, 1, 0,
		0x21, 0xffffffff, 0xffffffff, 0x42240000, 0x429a0000, 0xc3f90000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x389e7, 1, 0xffffffff, 0x42700000, 0x43200000, 0, 0, 0, 1, 0, 0x35,
		0xffffffff, 0xffffffff, 0x432a0000, 0x42a00000, 0x42c40000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x389f7, 1, 0xffffffff, 0x42840000, 0x43200000, 0, 0, 0, 1, 0, 0x35,
		0xffffffff, 0xffffffff, 0x41880000, 0x42a00000, 0x42c40000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a07, 1, 0xffffffff, 0x42480000, 0x43200000, 0, 0, 0, 1, 0, 0x35,
		0xffffffff, 0xffffffff, 0x40000000, 0x42a00000, 0x42d80000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a1b, 1, 0xffffffff, 0x42a00000, 0x43200000, 0, 0, 0, 1, 0, 0x35,
		0xffffffff, 0xffffffff, 0xc2380000, 0x42a00000, 0x42c40000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x37a3b, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42700000, 0, 1, 0,
		0x36, 0xffffffff, 0xffffffff, 0xc3610000, 0x42a00000, 0x42900000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x37a4b, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42700000, 0, 1, 0,
		0x36, 0xffffffff, 0xffffffff, 0xc2dc0000, 0x42a00000, 0x430c0000, 1, 0);
	CreatePanel(0x2f, unaff_EBX + 0x37913, 1, 0, 0x42700000, 0x43020000, 0, 0, 0, 1, 0, 7, 0x24, 0,
		0xc33e0000, 0x42820000, 0x43020000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a2b, 1, 0xffffffff, 0x42700000, 0x41f00000, 0, 0xc1f00000, 0, 1, 0,
		0x37, 0xffffffff, 0xffffffff, 0xc3430000, 0x43110000, 0x43110000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x3791f, 1, 0xffffffff, 0x42200000, 0x43200000, 0, 0xc1f00000, 0, 1, 0,
		0x37, 0xffffffff, 0xffffffff, 0xc36b0000, 0x42a00000, 0x42f40000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x3792f, 1, 0xffffffff, 0x42300000, 0x43200000, 0, 0xc1f00000, 0, 1, 0,
		0x37, 0xffffffff, 0xffffffff, 0xc3160000, 0x42a00000, 0x432b0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a3f, 1, 0xffffffff, 0x42f80000, 0x43200000, 0, 0, 0, 1, 0, 0x35,
		0xffffffff, 0xffffffff, 0xc39b8000, 0x42a00000, 0x42e60000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a4f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x34, 0xffffffff, 0xffffffff, 0xc3b90000, 0x42a00000, 0x43160000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a5f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x34, 0xffffffff, 0xffffffff, 0xc3b90000, 0x42a00000, 0x437a0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a6f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x34, 0xffffffff, 0xffffffff, 0xc3b90000, 0x42a00000, 0x43af0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a7f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x34, 0xffffffff, 0xffffffff, 0xc3b90000, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a8f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0xc3070000, 0x42a00000, 0x43680000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38a9f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0xc3070000, 0x42a00000, 0x43a60000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38aaf, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0xc3070000, 0x42a00000, 0x43d80000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38abf, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0xc3070000, 0x42a00000, 0x44050000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38acf, 1, 0xffffffff, 0x42a00000, 0x43200000, 0, 0, 0, 1, 0, 0x36,
		0xffffffff, 0xffffffff, 0xc3aa0000, 0x42a00000, 0x43f00000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38adf, 1, 0xffffffff, 0x42700000, 0x43200000, 0, 0, 0, 1, 0, 0x36,
		0xffffffff, 0xffffffff, 0xc3250000, 0x42a00000, 0x43f00000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38aef, 1, 0xffffffff, 0x42dc0000, 0x41f00000, 0, 0, 0, 1, 0, 0x36,
		0xffffffff, 0xffffffff, 0xc3750000, 0x43110000, 0x43f00000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38aff, 1, 0xffffffff, 0x425c0000, 0x43020000, 0, 0, 0, 1, 0, 7, 0x26, 0,
		0xc35d0000, 0x42820000, 0x43f04000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38b0f, 1, 0xffffffff, 0x425c0000, 0x43020000, 0, 0, 0, 1, 0, 7, 0x26, 0,
		0xc3890000, 0x42820000, 0x43f04000, 1, 0);
	CreatePanel(0x30, unaff_EBX + 0x38b1f, 1, 0xffffffff, 0x40e00000, 0x41700000, 0, 0, 0, 1, 0,
		7, 0x27, 0, 0xc39d8000, 0x42820000, 0x43efc000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38b2f, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42480000, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38b43, 1, 0xffffffff, 0x42a00000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42480000, 0x42a00000, 0x43b40000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38b57, 1, 0xffffffff, 0x41a00000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42480000, 0x42a00000, 0x439b0000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38b6f, 1, 0xffffffff, 0x42700000, 0x42200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42480000, 0x430c0000, 0x43870000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38b8f, 1, 0xffffffff, 0x42200000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42480000, 0x42a00000, 0x435c0000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38ba3, 1, 0xffffffff, 0x42cc0000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42480000, 0x42a00000, 0x43150000, 0, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38bb7, 1, 0xffffffff, 0x42d20000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x35, 0xffffffff, 0xffffffff, 0x42200000, 0x42a00000, 0x432f0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38bd3, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x36, 0xffffffff, 0xffffffff, 0x430c0000, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38be7, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x36, 0xffffffff, 0xffffffff, 0x430c0000, 0x42a00000, 0x43af0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38bfb, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x36, 0xffffffff, 0xffffffff, 0x430c0000, 0x42a00000, 0x437a0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c0f, 1, 0xffffffff, 0x42cc0000, 0x43200000, 0, 0x42b40000, 0, 1, 0,
		0x36, 0xffffffff, 0xffffffff, 0x430c0000, 0x42a00000, 0x43150000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c23, 1, 0xffffffff, 0x42700000, 0x43020000, 0, 0, 0, 1, 0, 7, 0x25, 0,
		0x42be0000, 0x42820000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c33, 1, 0xffffffff, 0x41a00000, 0x43200000, 0, 0, 0, 1, 0, 0x34,
		0xffffffff, 0xffffffff, 0x43070000, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c43, 1, 0xffffffff, 0x41a00000, 0x43200000, 0, 0, 0, 1, 0, 0x34,
		0xffffffff, 0xffffffff, 0x425c0000, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c53, 1, 0xffffffff, 0x42700000, 0x41f00000, 0, 0, 0, 1, 0, 0x34,
		0xffffffff, 0xffffffff, 0x42be0000, 0x43110000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c67, 1, 0xffffffff, 0x42ac0000, 0x43200000, 0, 0, 0, 1, 0, 0x36,
		0xffffffff, 0xffffffff, 0x40e00000, 0x42a00000, 0x43480000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c77, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0, 0, 1, 0, 0x36,
		0xffffffff, 0xffffffff, 0xc2aa0000, 0x42a00000, 0x43480000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c87, 1, 0xffffffff, 0x42c80000, 0x43200000, 0, 0, 0, 1, 0, 0x34,
		0xffffffff, 0xffffffff, 0, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38c97, 1, 0xffffffff, 0x42b40000, 0x43200000, 0, 0, 0, 1, 0, 0x34,
		0xffffffff, 0xffffffff, 0xc2aa0000, 0x42a00000, 0x43e10000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x3855b, 1, 0xffffffff, 0x42700000, 0x42480000, 0, 0x42b40000, 0, 1, 0, 7
		, 0x2c, 0, 0xc2c80000, 0x41c80000, 0x43660000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38573, 1, 0xffffffff, 0x42700000, 0x42480000, 0, 0x42b40000, 0, 1, 0, 7
		, 0x2c, 0, 0xc2c80000, 0x41c80000, 0x43910000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x3858b, 1, 0xffffffff, 0x42700000, 0x42480000, 0, 0x42b40000, 0, 1, 0, 7
		, 0x2c, 0, 0xc2c80000, 0x41c80000, 0x43af0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x385a3, 1, 0xffffffff, 0x41a00000, 0x42480000, 0, 0x42b40000, 0, 1, 0, 7
		, 0x2c, 0, 0xc2c80000, 0x41c80000, 0x43c30000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x385bb, 1, 0xffffffff, 0x41a00000, 0x42700000, 0x42b40000, 0, 0, 1, 0, 7
		, 0xffffffff, 0xffffffff, 0xc2dc0000, 0x42480000, 0x43660000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x385cf, 1, 0xffffffff, 0x41a00000, 0x42700000, 0x42b40000, 0, 0, 1, 0, 7
		, 0xffffffff, 0xffffffff, 0xc2dc0000, 0x42480000, 0x43910000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x385e3, 1, 0xffffffff, 0x41a00000, 0x42700000, 0x42b40000, 0, 0, 1, 0, 7
		, 0xffffffff, 0xffffffff, 0xc2dc0000, 0x42480000, 0x43af0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x385f7, 1, 0xffffffff, 0x41a00000, 0x42700000, 0x42b40000, 0, 0, 1, 0, 7
		, 0xffffffff, 0xffffffff, 0xc2dc0000, 0x42480000, 0x43cd0000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38ca7, 1, 0xffffffff, 0x41a00000, 0x42480000, 0, 0, 0, 1, 0, 7, 0x2c, 0,
		0xc2b40000, 0x41c80000, 0x43c80000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38cbf, 1, 0xffffffff, 0x42700000, 0x42480000, 0, 0, 0, 1, 0, 7, 0x2c, 0,
		0xc2480000, 0x41c80000, 0x43c80000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38cd7, 1, 0xffffffff, 0x42a00000, 0x42200000, 0x42b40000, 0, 0, 1, 0, 7
		, 0xffffffff, 0xffffffff, 0xc2700000, 0x42480000, 0x43d20000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38ceb, 1, 0xffffffff, 0x42700000, 0x42f00000, 0, 0x42b40000, 0, 1, 0,
		0x25, 0xffffffff, 0, 0xc1a00000, 0x42700000, 0x43d70000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38cfb, 1, 0xffffffff, 0x42700000, 0x42f00000, 0, 0x42b40000, 0, 1, 0,
		0x25, 0xffffffff, 0, 0x42200000, 0x42700000, 0x43d70000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d0b, 1, 0xffffffff, 0x42700000, 0x42200000, 0, 0, 0, 1, 0, 0x25,
		0xffffffff, 0, 0x41200000, 0x42c80000, 0x43c80000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d1b, 1, 0xffffffff, 0x42700000, 0x42900000, 0, 0, 0, 1, 0, 0x25,
		0xffffffff, 0, 0x41200000, 0x42300000, 0x43c80000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d2b, 1, 0xffffffff, 0x42700000, 0x41000000, 0, 0, 0, 1, 0, 0x11,
		0xffffffff, 0, 0x41200000, 0x40800000, 0x43c88000, 1, 0);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d43, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x39, 0xffffffff, 0xffffffff, 0xc37a0000, 0x43200000, 0x43960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d4f, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x39, 0xffffffff, 0xffffffff, 0x437a0000, 0x43200000, 0x43960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d5b, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x39, 0xffffffff, 0xffffffff, 0xc37a0000, 0x43200000, 0xc3960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d67, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x39, 0xffffffff, 0xffffffff, 0x437a0000, 0x43200000, 0xc3960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d73, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x3c, 0xffffffff, 0xffffffff, 0xc37a0000, 0, 0x43960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d7b, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x3c, 0xffffffff, 0xffffffff, 0x437a0000, 0, 0x43960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d83, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x3c, 0xffffffff, 0xffffffff, 0xc37a0000, 0, 0xc3960000, 0, 1);
	CreatePanel(0xffffffff, unaff_EBX + 0x38d8b, 1, 0xffffffff, 0x43fa0000, 0x44160000, 0x42b40000, 0, 0, 0, 0,
		0x3c, 0xffffffff, 0xffffffff, 0x437a0000, 0, 0xc3960000, 0, 1);
}