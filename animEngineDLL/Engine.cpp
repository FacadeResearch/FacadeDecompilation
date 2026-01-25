#include "Engine.h"
#include <dinput.h>
#include "SoundEntry.h"
#include "AppWindow.h"
#include <gl/GL.h>
#include <string>
#include "EventLog.h"
#include <ctime>

bool Engine::g_bPrintBetaInfo = false;
bool Engine::g_bDSoundInitialized = false;
bool Engine::g_stagePlayExists = false;

int Engine::g_ticksIntroStarted = 0;
int Engine::fadeCurtainInCtr = 0;
int Engine::g_numFramesRun = 0;

float Engine::g_gmf[256] = { 0.0f };

GLuint Engine::g_LoadingTextureID = 0;

HDC Engine::g_HDC = NULL;
HWND Engine::g_Window = NULL;
XCursor Engine::g_pCursor = XCursor();
Player Engine::g_Player = Player();

LPDIRECTINPUTDEVICE8 Engine::g_pKeyboard = NULL;

GLuint Engine::g_BitmapFontBase = 0;

LPDIRECTINPUT8 Engine::g_pDI = NULL;
LPDIRECTSOUND Engine::g_pDS = NULL;
SoundEntry Engine::g_SoundBank[256];

float Engine::g_FadeAlpha = 0;
float Engine::g_FadeSpeed = 0;

bool Engine::g_bAtEndOfLoad = false;

float Engine::g_GlobalAlpha = 1.0f;

bool Engine::bFadeCurtainOut = false;

int Engine::ctr = 0;

std::string Engine::g_launchErrorString = "";

int *Engine::ChangeSize() {
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
	glBindTexture(0xde1, g_LoadingTextureID);
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

void Engine::StartFade(float speed) {
    g_FadeSpeed = -speed;

    if (g_FadeSpeed < 0.0f) {
        g_FadeAlpha = 1.0f;
    }
    else {
        g_FadeAlpha = 0.0f;
    }
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
    if (!text || text[0] == '\0') return;

    int textLen = static_cast<int>(strlen(text));
    int charIndex = 0;
    int lineIndex = 0;

    SetColorFromIndex(colorIndex);

    while (charIndex < textLen) {
        glLoadIdentity();

        float drawX = xPos - 25.0f;
        float drawY = ((float)lineIndex * scale * -0.7f) + yPos;
        float drawZ = zOffset - 2.0f;

        glTranslatef(drawX, drawY, drawZ);
        glScalef(scale, scale, scale);

        float currentLineWidth = 0.0f;

        while (charIndex < textLen) {
            char c = text[charIndex];
            char charStr[2] = { c, '\0' };

            int prevWidthInt = static_cast<int>(currentLineWidth);

            float charWidth = glPrintPolygonalFont(charStr);
            currentLineWidth += charWidth;

            if (c == (char)-25) {
                currentLineWidth = (float)prevWidthInt;
            }

            charIndex++;

            if (charIndex < textLen && text[charIndex - 1] == ' ') {
                if (currentLineWidth >= (float)maxWidth * 0.1f) {
                    lineIndex++;
                    goto wrap_new_line;
                }
            }
        }
    wrap_new_line:;
    }
}

void Engine::CutToBlack() {
    g_FadeAlpha = 1.0f;
    g_FadeSpeed = 0.0f;
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
                    mainStatus = "Loading...";
                }
                else {
                    if (ctr <= 30) {
                        mainStatus = "Loading.";
                    }
                    else if (ctr <= 45) {
                        mainStatus = "Loading..";
                    }
                    else if (ctr <= 60) {
                        mainStatus = "Loading...";
                    }
                    else mainStatus = "Loading";

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

void Engine::LoadSoundManifest(const char* path)
{

}

HRESULT Engine::Keyboard_Init()
{
	HRESULT hr;
	hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, NULL);

	if (FAILED(hr)) return hr;
	hr = g_pDI->CreateDevice(GUID_SysKeyboard, &g_pKeyboard, NULL);

	if (FAILED(hr)) return hr;
	hr = g_pKeyboard->SetDataFormat(&c_dfDIKeyboard);

	if (FAILED(hr)) return hr;
	hr = g_pKeyboard->SetCooperativeLevel(Engine::g_Window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	if (FAILED(hr)) return hr;

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 16;

	hr = g_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if (FAILED(hr)) return hr;

	hr = g_pKeyboard->Acquire();
	return S_OK;
}

HRESULT Engine::DSound_Init() {
	if (!g_bDSoundInitialized) {
		for (int i = 0; i < 256; i++) {
			if (g_SoundBank[i].pBuffer) {
				g_SoundBank[i].pBuffer->Stop();
				g_SoundBank[i].pBuffer->Release();
			}
			memset(&g_SoundBank[i], 0, sizeof(SoundEntry));
			g_SoundBank[i].bufferId = i;
		}
		return 1;
	}

	if (DirectSoundCreate(NULL, &g_pDS, NULL) >= 0) {
		if (g_pDS->SetCooperativeLevel(g_Window, DSSCL_PRIORITY) >= 0) {
			g_bDSoundInitialized = false;
			return 1;
		}
	}

	return 0;
}

void Engine::InitGlobals() {
    EventLog::Record(0, -1, "\n---------------------------\nInitting globals...");
    srand((unsigned int)time(NULL));

    g_numFramesRun = GetTickCount();
    fadeCurtainInCtr = 0;
    g_ticksIntroStarted = -1;

    //InitRendererSubsystem();  // j_sub_100612c0
    //InitScriptingSystem();    // j_sub_1004d590
    //InitPhysicsSystem();      // j_sub_100843e0

    g_Player = Player();
    g_Player.InitPlayer("player", 0, 1);

    LoadSoundManifest("sounds\\global\\globalsnd.txt");

    g_pCursor = XCursor();
    g_pCursor.Init();

    HICON hIcon = LoadIconA(GetModuleHandle(NULL), (LPCSTR)0x83);
    SendMessage(g_Window, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}