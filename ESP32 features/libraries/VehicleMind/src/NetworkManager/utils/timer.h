//---------------------------------------------------
//INCLUDE GUARD
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPS
#include <HardwareSerial.h>
#include "../utils/common_types.h"

class Timer
{
  public:
    void setTimeout(int32_t p_timeout);
    int32_t getTimeLeft();

  private:
    int32_t m_end = 0;
};

#endif