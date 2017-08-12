#ifndef __CELL_UDP_CLIENT_H
#define __CELL_UDP_CLIENT_H

#include "../SIM5360/SIM5360.h"

class CellUDPClient : public SIM5360 {
  public:

    CellUDPClient();
    ~CellUDPClient();

    bool udpInit()
    {
      return sendCommand("AT+CIPOPEN=0,\"UDP\",,,8000\r", 3000);
    }
    bool udpSend(const char* ip, uint16_t port, const char* data, unsigned int len)
    {
      sprintf_P(buffer, PSTR("AT+CIPSEND=0,%u,\"%s\",%u\r"), len, ip, port);
      if (sendCommand(buffer, 100, ">")) {
        sys.xbWrite(data, len);
        return sendCommand(0, 1000);
      } else {
        Serial.println(buffer);
      }
      return false;
    }
    char* udpReceive(int* pbytes = 0)
    {
      if (sendCommand(0, 3000, "+IPD")) {
        char *p = strstr(buffer, "+IPD");
        if (!p) return 0;
        int len = atoi(p + 4);
        if (pbytes) *pbytes = len;
        p = strchr(p, '\n');
        if (p) {
          *(++p + len) = 0;
          return p;
        }
      }
      return 0;
    }

};

#endif /* __CELL_UDP_CLIENT_H */