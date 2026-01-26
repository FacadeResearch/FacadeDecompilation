#include "AppWindow.h"
#include "EventLog.h"
#include <gl/GL.h>
#include "Engine.h"
#include <Windows.h>
#include <iostream>
#include "Globals.h"

int AppWindow::g_SleepTime = 0;
int AppWindow::g_MouseX = 0;
int AppWindow::g_MouseY = 0;
int AppWindow::g_mouseLButtonClicked = 0;

HPALETTE AppWindow::g_hPalette = NULL;

int AppWindow::g_windowHeight = 768;
int AppWindow::g_windowWidth = 1024;

bool AppWindow::g_bAntiAliasing = false;
bool AppWindow::g_IsInactive = false;
bool AppWindow::g_bAblDisconnect = false;

BOOL AppWindow::SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),   // nSize
        1,                               // nVersion
        PFD_DRAW_TO_WINDOW |             // dwFlags (0x25)
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                   // iPixelType
        24,                              // cColorBits (0x18)
        0, 0, 0, 0, 0, 0,                // Color bits/shifts (ignored)
        0,                               // cAlphaBits
        0,                               // cAlphaShift
        0,                               // cAccumBits
        0, 0, 0, 0,                      // Accum bits
        32,                              // cDepthBits (0x20)
        0,                               // cStencilBits
        0,                               // cAuxBuffers
        PFD_MAIN_PLANE,                  // iLayerType
        0,                               // bReserved
        0, 0, 0                          // Layer masks
    };

    int32_t format = ChoosePixelFormat(hdc, &pfd);
    if (format == 0) return FALSE;
    
    return SetPixelFormat(hdc, format, &pfd);
}

HPALETTE AppWindow::CreateEnginePalette(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd;

    int n = GetPixelFormat(hdc);
    
    if (DescribePixelFormat(hdc, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0) {
        return NULL;
    }

    if (!(pfd.dwFlags & PFD_NEED_PALETTE)) {
        return NULL;
    }

    int numEntries = 1 << pfd.cColorBits;
    int palSize = sizeof(LOGPALETTE) + (numEntries * sizeof(PALETTEENTRY));

    LOGPALETTE* plpal = (LOGPALETTE*)malloc(palSize);

    if (plpal == NULL) {
        return NULL;
    }

    plpal->palVersion = 0x300;
    plpal->palNumEntries = numEntries;

    for (int i = 0; i < numEntries; i++) {
        int redDenominator = (1 << pfd.cRedBits) - 1;
        int greenDenominator = (1 << pfd.cGreenBits) - 1;
        int blueDenominator = (1 << pfd.cBlueBits) - 1;

        plpal->palPalEntry[i].peRed = (BYTE)(((i >> pfd.cRedShift) & ((1 << pfd.cRedBits) - 1)) * 255 / ((1 << pfd.cRedBits) - 1));
        plpal->palPalEntry[i].peGreen = (BYTE)(((i >> pfd.cGreenShift) & ((1 << pfd.cGreenBits) - 1)) * 255 / ((1 << pfd.cGreenBits) - 1));
        plpal->palPalEntry[i].peBlue = (BYTE)(((i >> pfd.cBlueShift) & ((1 << pfd.cBlueBits) - 1)) * 255 / ((1 << pfd.cBlueBits) - 1));
        plpal->palPalEntry[i].peFlags = 0;
    }

    HPALETTE hPal = CreatePalette(plpal);

    free(plpal);

    if (hPal) {
        SelectPalette(hdc, hPal, FALSE);
        RealizePalette(hdc);
    }

    return hPal;
}

void AppWindow::InitOpenGLState() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if (AppWindow::g_bAntiAliasing) {
        glEnable(GL_POINT_SMOOTH);
    }
}

LRESULT CALLBACK AppWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int result = 0;

    if (uMsg > WM_QUERYNEWPALETTE) {
        switch (uMsg) {
            case WM_PALETTECHANGED:
                if (!g_hPalette || (HWND)wParam == hWnd) {
                    return 0;
                }

                SelectPalette(g_HDC, g_hPalette, FALSE);
                RealizePalette(g_HDC);
                UpdateColors(g_HDC);
                result = 0;
            break;
            case WM_USER:
                //sub_10001FAA(dword_1097947C);
                return 0;
            case 0x401: //Ida says DDM_DRAW (?)
                // sub_10002324(dword_1097947C);
                return 0;
            case 0x402: //DDM_CLOSE
                //sub_10001FAA(dword_10979480);
                return 0;
            case 0x403: //DDM_BEGIN
                //sub_10002324(dword_10979480);
                return 0;
            case 0x40D: //PBM_GETSTEP
                //sub_100018CF();
                return 0;
            default:
                return DefWindowProcA(hWnd, uMsg, wParam, lParam);
        }
    }
    else {
        if (uMsg == WM_QUERYNEWPALETTE) {
            if (g_hPalette) {
                SelectPalette(g_HDC, g_hPalette, FALSE);
                UINT result = RealizePalette(g_HDC);
                InvalidateRect(hWnd, NULL, FALSE);
                return result;
            }
        }
        else if (uMsg > WM_ACTIVATE) {
            switch (uMsg) {
                case WM_PAINT: {
                    //to-do
                    //data_1082d58c what is this? //&& g_SomeOtherFlag == 0

                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);

                    if (g_IsInactive) {
                        EndPaint(hWnd, &ps);
                        SendMessageA(Engine::g_Window, WM_CLOSE, 0, 0); //WM_CLOSE is 0x10? wtf?
                        return 0;
                    }

                    uint32_t frameStart = GetTickCount();

                    Engine::ChangeSize();

                    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    Engine::LoadingMessage();
                    Engine::UpdateFadeEffect();
                    glFlush();
                    SwapBuffers(g_HDC);

                    //CalculateFPS(frameStart);

                    if (g_SleepTime > 0) Sleep(g_SleepTime);

                    EndPaint(hWnd, &ps);
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                    break;
                case WM_MOUSEFIRST:
                    //dword_1083F5E8 = (unsigned __int16)lParam;
                    //dword_1083F5EC = HIWORD(lParam);
                    break;
                case WM_LBUTTONDOWN:
                    // byte_10979468 = 1;
                    break;
                default:
                    return DefWindowProcA(hWnd, uMsg, (WPARAM)wParam, lParam);
            }
        }
        else {
            switch (uMsg) {
                case WM_ACTIVATE:
                    break;
                case WM_CREATE: {
                    g_windowWidth = LOWORD(lParam);

                    HDC hdc = GetDC(hWnd);
                    g_HDC = hdc;
                    SetupPixelFormat(hdc);

                    g_HRC = wglCreateContext(hdc);
                    wglMakeCurrent(hdc, g_HRC);

                    g_hPalette = CreateEnginePalette(hdc);
                    InitOpenGLState();
                }
                    break;
                case WM_DESTROY: {
                    dprintf("\n### AnimEngine cleaning up\n");
                    SoundSystem::StopAllMP3s();
                    if (AppWindow::g_bAblDisconnect) {
                        ErrorMessage((uint8_t*)"We're sorry, Facade is having trouble running, possibly because other applications are runni"
                            "ng, or some other factor limiting the CPU available.\n"
                            "\n"
                            "Please try running Facade again, after closing other applications or restarting.\n"
                            "\n"
                            "Also visit www.interactivestory.net for bug fixes and patches.");
                    }
                    EventLog::OutputEventLog(1);
                    dprintf("Deallocating globals");
                }
                    break;
                case WM_SIZE:
                    //sub_100010B4((unsigned __int16)lParam, HIWORD(lParam));
                    break;
                default:
                    return DefWindowProcA(hWnd, uMsg, (WPARAM)wParam, lParam);
            }
        }
    }

    return result;
}