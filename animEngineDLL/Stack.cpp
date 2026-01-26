#include "Stack.h"
#include <cstring>
#include <cstdlib>

Stack::Stack() {
    m_size = 0;
    m_cursor = 0;
    m_capacity = 0;
    m_savedPtrCount = 0;
    m_val1 = -1;
    m_val2 = -1;
    m_data = nullptr;
    m_stateCount = 0;

    for (int i = 0; i < 10; ++i) {
        m_savedStates[i] = nullptr;
        m_savedData[i] = nullptr;
    }

    memset(m_unknownData, 0, sizeof(m_unknownData));
}

Stack::~Stack() {
    //to-do
}

int Stack::InitStack(int capacity)
{
    return 0;
}

void Stack::Skip(int count)
{
}

void Stack::Compress() {
    //to-do
}

void* Stack::SaveState() {
    //to-do
    return nullptr;
}

void Stack::RestoreState()
{

}

int Stack::InsertElement(int value, int index)
{
    return 0;
}

int Stack::ResolveRand() {
    //to-do
    return 0;
}