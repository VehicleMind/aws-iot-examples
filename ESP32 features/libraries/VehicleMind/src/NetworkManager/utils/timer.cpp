#include "timer.h"

void Timer::setTimeout(int32_t p_timeout){
    m_end = (int32_t)millis() + p_timeout;
}

int32_t Timer::getTimeLeft(){
    return m_end - (int32_t)millis();
}

