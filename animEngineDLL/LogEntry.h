#pragma once

#include <Windows.h>
#include <cstdint>

struct LogEntry {
	uint32_t timestamp;
	uint32_t type;
	uint32_t spriteID;
	char message[170];
	uint8_t flag;
	uint32_t result;
	uint8_t padding;
	int value;
	int unusedTail; //Unused ending part?
};