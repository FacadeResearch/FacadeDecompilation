#pragma once
#include <stdint.h>

class SoundSource {
	public:
		uint32_t m_vtable;
		char sourcePath[256];
		char baseSoundName[252];
		int32_t soundCount;
		char soundList[5000][256];
		SoundSource();
		int __cdecl InitSoundSource(char* srcPath);
};