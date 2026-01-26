#include "globals.h"
#include "Engine.h"
#include "SoundSystem.h"
#include "Input.h"


HWND g_Window = NULL;
HDC g_HDC = NULL;
HGLRC g_HRC = NULL;

extern "C" {
    int DSound_Init() {
        return SoundSystem::DSound_Init();
    }

    int DSound_Shutdown() {
        return SoundSystem::DSound_Shutdown();
    }

    void InitGlobals() {
        Engine::InitGlobals();
    }

    int dprintf(const char* msg, ...)
    {
        //I assume the code is supposed to call printf here but they just never got around to it
        printf(msg);
        return 1;
    }

    int __cdecl ErrorMessage(uint8_t* bytes)
    {
        dprintf("Error: %s", (const char*)bytes);
        int result = MessageBoxA(g_Window, (const char*)bytes, "Error Message", (UINT)0x50030);
        return result; //SDL_ShowCursor();
    }

    FILE* __cdecl fopen_UnixPath(char* filePath, const char* mode)
    {
        char* path = UnixPath(filePath, *mode != 114);
        return fopen(path, mode);
    }

    char* __cdecl UnixPath(char* __s2, char a2)
    {
        //to-do
        return __s2;
    }

    int Keyboard_Init() {
        return Input::Keyboard_Init();
    }
}