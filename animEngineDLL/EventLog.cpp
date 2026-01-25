#include "EventLog.h"
#include <stdio.h>
#include <string.h>
#include "Globals.h"
#include "EEventLogType.h"

int32_t EventLog::g_eventLogCtr = 0;
LogEntry EventLog::g_eventLog[5999];

int __cdecl EventLog::AddEventInfoToLog(int type, unsigned int spriteID, const char* src, uint8_t isReplacement, int value, uint32_t timestamp) {
    static bool bPrintedErrorMessage = false;

    if (g_eventLogCtr > 5999) {
        if (!bPrintedErrorMessage) {
            dprintf("### ERROR: Event log is FULL!");
            bPrintedErrorMessage = true;
        }
        return (int)isReplacement;
    }

    if (type == 1) {
        if (!src || src[0] == '\0' || strcmp(src, " ") == 0 || strcmp(src, "q") == 0) {
            return (int)isReplacement;
        }
    }
    else if (type == 12) {
        if (!src || src[0] == '\0') return (int)isReplacement;
    }
    else if (type == 0) {
        if (!src || src[0] == '\0') return (int)isReplacement;
    }

    int targetIndex = g_eventLogCtr;

    if (isReplacement && spriteID <= 1 && g_eventLogCtr > 0) {
        int prevIndex = g_eventLogCtr - 1;
        LogEntry* prevEntry = &g_eventLog[prevIndex];

        if (prevEntry->spriteID == 2) {
            LogEntry* currentEntry = &g_eventLog[g_eventLogCtr];

            currentEntry->timestamp = prevEntry->timestamp;
            currentEntry->type = prevEntry->type;
            currentEntry->spriteID = 2;

            strcpy(currentEntry->message, prevEntry->message);
            currentEntry->flag = prevEntry->flag;
            currentEntry->value = prevEntry->value;

            targetIndex = prevIndex;
        }
    }

    LogEntry* entry = &g_eventLog[targetIndex];

    entry->timestamp = (timestamp != 0) ? timestamp : GetTickCount();
    entry->type = type;
    entry->spriteID = spriteID;

    if (src) {
        strcpy(entry->message, src);
    }
    else {
        entry->message[0] = '\0';
    }

    entry->flag = isReplacement;
    entry->value = value;
    entry->unusedTail = 0;

    g_eventLogCtr++;

    return (int)&entry->flag;
}

void EventLog::ShouldntBeHere(const char* message) {
	printf("\n#####\nASSERTION: %s\n#####\n", message);
	AddEventInfoToLog(12, -1, message, 0, FALSE, 0);
}

int __cdecl EventLog::OutputEventLog(bool flag)
{
	//So this handles the.. stageplay writing? Flag is to control if it writes to the file or not?
	return 0;
}
