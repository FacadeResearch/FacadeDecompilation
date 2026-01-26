#pragma once
#include "Engine.h"
#include "SoundSystem.h"
#include "Input.h"

extern "C" {
    extern HWND g_Window;
    extern HDC g_HDC;
    extern HGLRC g_HRC;

    int DSound_Init();
    int DSound_Shutdown();
    void InitGlobals();
    int dprintf(const char* msg, ...);
    int __cdecl ErrorMessage(uint8_t* bytes);
    int Keyboard_Init();
}