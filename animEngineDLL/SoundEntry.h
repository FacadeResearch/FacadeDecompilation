#pragma once
#include <dsound.h>
#include <cstdint>

struct SoundEntry {
	LPDIRECTSOUNDBUFFER pBuffer;
	uint32_t unknown1;
	uint32_t unknowwn2;
	uint32_t unknown3;
	uint32_t bufferId;
};