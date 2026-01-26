#include "Controller.h"

Controller::~Controller() {

}

Controller::Controller() {
    m_valuePtr = nullptr;
    m_isActive = false;
}

int Controller::InitController(float* valAddr, float minVal, float maxVal, int type) {
    m_valuePtr = valAddr;
    m_min = minVal;
    m_max = maxVal;
    m_useSquareSine = (bool)type;
    return type;
}

float Controller::GetVal() {
    return *m_valuePtr;
}

void Controller::StopMoving() {
    m_isActive = false;
}