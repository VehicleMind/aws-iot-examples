#ifndef __CELL_HTTP_CLIENT_H
#define __CELL_HTTP_CLIENT_H

#include "../SIM5360/SIM5360.h"

typedef enum {
  HTTP_GET = 0,
  HTTP_POST,
} HTTP_METHOD;

#define MAX_CONN_TIME 5000
#define XBEE_BAUDRATE 115200

class CellHTTPClient : public SIM5360 {

  public:

    CellHTTPClient() {}
    ~CellHTTPClient() {
      free(m_serverUrl);
      free(m_serverPath);
    }

    bool begin(char *serverUrl);

    bool end() {
      bool ret = httpClose();
      return ret;
    }

    bool GET()
    {
      bool ret = httpSend(HTTP_GET, true);
      if (int recdBytes = httpReceive())
        return ret;
    }

    int getSize();

    char *getContentType();

    char *getStreamPtr();

    // int GET();
    // WiFiClient * getStreamPtr();
    // bool connected();
    // void end();

  private:

    char *m_serverUrl;
    int m_serverPort = 80;
    char *m_serverPath;
    char m_payload[1024] = {0};

    bool httpOpen()
    {
      return sendCommand("AT+CHTTPSSTART\r", 3000);
    }

    bool httpClose()
    {
      return sendCommand("AT+CHTTPSCLSE\r");
    }

    bool httpConnect()
    {
        sprintf_P(buffer, PSTR("AT+CHTTPSOPSE=\"%s\",%u,2\r"), m_serverUrl, m_serverPort);
        sys.xbPurge();
	      return sendCommand(buffer, MAX_CONN_TIME);
    }

    bool httpSend(HTTP_METHOD method, bool keepAlive, const char *payload = 0, int payloadSize = 0);

    unsigned int genHttpHeader(HTTP_METHOD method, bool keepAlive, const char *payload, int payloadSize);

    byte checkbuffer(const char *expected, unsigned int timeout = 2000);

    int httpReceive();

};

#endif /* __CELL_HTTP_CLIENT_H */