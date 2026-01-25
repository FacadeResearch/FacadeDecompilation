#pragma once
#include "LogEntry.h"
#include <Windows.h>
#include <cstdint>

class EventLog {
	public:
		static const int MAX_ENTRIES = 5999;
		static int __cdecl AddEventInfoToLog(int type, unsigned int spriteID, const char* src, uint8_t isReplacement, int value, uint32_t timestamp = 0);
		static void ShouldntBeHere(const char* message);
		static int __cdecl OutputEventLog(bool flag);
	private:
		static LogEntry g_eventLog[MAX_ENTRIES];
		static int32_t g_eventLogCtr;
};