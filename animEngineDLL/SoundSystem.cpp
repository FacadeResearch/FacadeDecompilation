#include "SoundSystem.h"
#include <wincrypt.h>
#include "Globals.h"

bool SoundSystem::g_bDSoundInitialized = false;
SoundEntry SoundSystem::g_SoundBank[256];
LPDIRECTSOUND SoundSystem::g_pDS = NULL;

HRESULT SoundSystem::DSound_Init()
{
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

HRESULT SoundSystem::DSound_Shutdown()
{
	return E_NOTIMPL;
}

void SoundSystem::StopAllMP3s()
{

}