#include "Controller.h"

Controller::~Controller() {

}

Controller::Controller() {
    m_valuePtr = nullptr;
    m_isActive = false;
    m_min = 0;
    m_max = 0;
    m_target = 0;
    m_initial = 0;
    m_rate = 0;
    m_totalFrames = 0;
    m_elapsedFrames = 0;
    m_useSquareSine = false;
    m_type = 0;
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