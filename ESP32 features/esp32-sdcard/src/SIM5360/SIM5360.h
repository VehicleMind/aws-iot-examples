#ifndef __SIM5360_H
#define __SIM5360_H

#include "../FreematicsPlus/FreematicsPlus.h"

class SIM5360 {
  public:

    char buffer[1024];
    uint32_t checkTimer;
    FreematicsESP32 sys;

    SIM5360() { buffer[0] = 0; }
    bool init();
    bool setup(const char *apn = "soracom.io", const char* cellusr = "sora", const char* cellpwd = "sora", bool only3G = false, bool roaming = true);
    bool initGPS();
    char * getIP();
    int getSignal();
    char * getOperatorName();
    bool sendCommand(const char *cmd, unsigned int timeout = 2000, const char *expected = "\r\nOK\r\n", bool terminated = false);

};

#endif  /* __SIM5360_H */