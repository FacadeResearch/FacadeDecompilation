#include "XCursor.h"

XCursor* XCursor::theirCursor = nullptr;

XCursor::~XCursor()
{
	//to-do
}

XCursor::XCursor()
{

}

int __cdecl XCursor::Push(int a1, int a2)
{
	return 0;
}

int __cdecl XCursor::ResetStack(DWORD* a1, int a2)
{
	return 0;
}

XCursor* __cdecl XCursor::SetRate(XCursor* xCursor, int a2)
{
	return nullptr;
}

int __cdecl XCursor::SetCursorPosition(int a1, unsigned __int16* a2)
{
	return 0;
}

int __cdecl XCursor::ChangeCursorImage(int a1, int a2)
{
	return 0;
}

void XCursor::InitCursor()
{

}

void XCursor::RenderCursor()
{

}

void XCursor::OffsetPointALittle()
{
	//Never actually implemented, should we do the same?
}

bool __cdecl XCursor::IsCursorStackEmpty(XCursor* xCursor)
{
	return false;
}

int __cdecl XCursor::JumpToCursor(DWORD* a1, int a2, char a3)
{
	return 0;
}

int __cdecl XCursor::Override(int a1, int a2)
{
	return 0;
}

int __cdecl XCursor::Restore(XCursor* xCursor)
{
	return 0;
}

int* __cdecl XCursor::Update(XCursor* xCursor)
{
	return nullptr;
}

int __cdecl XCursor::PushTransitionToNeutral(int a1, int a2)
{
	return 0;
}

int __cdecl XCursor::PushStoredAction(int a1, int a2, int a3)
{
	return 0;
}