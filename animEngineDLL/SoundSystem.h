#pragma once
#include "SoundEntry.h"

class SoundSystem {
public:
    static HRESULT DSound_Init();
    static HRESULT DSound_Shutdown();
    static void StopAllMP3s();
    static bool g_bDSoundInitialized;
    static SoundEntry g_SoundBank[256];
    static LPDIRECTSOUND g_pDS;
};