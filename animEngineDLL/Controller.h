#pragma once

class Controller {
public:
    virtual ~Controller();
    float* m_valuePtr;
    float m_min;
    float m_max;
    bool m_isActive;
    float m_target;
    float m_initial;
    float m_rate;
    int m_totalFrames;
    int m_elapsedFrames;
    bool m_useSquareSine;
    int m_type;
    Controller();
    int InitController(float* valAddr, float minVal, float maxVal, int type);
    float GetVal();
    void StopMoving();
};