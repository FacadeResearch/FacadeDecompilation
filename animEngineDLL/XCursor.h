#pragma once
#include <Windows.h>
#include <cstdint>
#include "Stack.h"

class XCursor {
	public:
		virtual ~XCursor();
        uint8_t m_isOverridden;
        int32_t m_frameCounts[3];
        int32_t m_startFrames[3];
        int32_t m_endFrames[3];
        int32_t m_currentID;
        int32_t m_unknown12;
        Stack* m_pStack;
        int32_t m_targetID;
        int32_t m_timer;
        int32_t m_stackIndex;
        int32_t m_rate;
        int32_t m_counter;
        static XCursor* theirCursor;
		XCursor();
        int __cdecl Push(int a1, int a2);
        int __cdecl ResetStack(DWORD* a1, int a2);
        XCursor* __cdecl SetRate(XCursor* xCursor, int a2);
        int __cdecl SetCursorPosition(int a1, unsigned __int16* a2);
        int __cdecl ChangeCursorImage(int a1, int a2);
        void InitCursor();
        void RenderCursor();
        void OffsetPointALittle();
        bool __cdecl IsCursorStackEmpty(XCursor* xCursor);
        int __cdecl JumpToCursor(DWORD* a1, int a2, char a3);
        int __cdecl Override(int a1, int a2);
        int __cdecl Restore(XCursor* xCursor);
        int *__cdecl Update(XCursor *xCursor);
        int __cdecl PushTransitionToNeutral(int a1, int a2);
        int __cdecl PushStoredAction(int a1, int a2, int a3);
};