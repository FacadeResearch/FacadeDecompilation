#pragma once
#include <Windows.h>
#include <cstdint>

class Stack {
	public:
		virtual ~Stack();
        int32_t m_stateCount;
        void* m_savedStates[10];
        void* m_savedData[10];
        int32_t m_unknownData[11];
        int32_t m_savedPtrCount;
        int32_t m_val1;
        int32_t m_val2;
        int32_t m_size;
        int32_t m_capacity;
        int32_t m_cursor;
        int32_t* m_data;
        Stack();
        int  InitStack(int capacity);
        void Skip(int count);
        void Compress();
        void* SaveState();
        void RestoreState();
        int  InsertElement(int value, int index);
        int  ResolveRand();
};