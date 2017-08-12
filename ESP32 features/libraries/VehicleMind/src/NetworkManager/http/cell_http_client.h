#ifndef CELL_HTTP_CLIENT_H_INCLUDED
#define CELL_HTTP_CLIENT_H_INCLUDED

#include "../tcp/tcp_sim5360.h"
#include "../utils/url_parser.h"
#include "../../Tools/Verbose.h"

#define DEBUG

typedef enum{
    HTTP_GET = 0,
    HTTP_POST,
} HTTP_METHOD;

typedef enum{
    TXT_EXT = 0,
    BIN_EXT,
} FILE_EXT;

#define MAX_CONN_TIME 5000
#define XBEE_BAUDRATE 115200
#define PAYLOAD_SIZE 1024 + 150
class CellHTTPClient : public TCPSIM5360{

    public:
        CellHTTPClient(int p_connection_timeout) : TCPSIM5360(p_connection_timeout), v(true, "HTTP") {}
        ~CellHTTPClient()
        {
            free(m_serverUrl);
            free(m_serverPath);
        }

        bool begin(const char *serverUrl);
        bool end();
        bool GETrequest();
        int getSize();
        int httpReceive();
        char* getContentType();
        char* getStreamPtr();
        char* RecieveData(int &TempNum);
        bool reset();

    private:
        bool httpClose();
        bool httpStop();
        bool httpStart();
        bool httpOpen();

        bool httpSend(HTTP_METHOD method, bool keepAlive, const char* payload = 0, int payloadSize = 0);
        unsigned int genHttpHeader(HTTP_METHOD method, bool keepAlive, const char* payload, int payloadSize);
        byte checkbuffer(const char* expected, unsigned int timeout = 2000);

        Verbose v;
        int m_serverPort = 80;
        char m_payload[PAYLOAD_SIZE] = {0};
        char* m_serverUrl;
        char* m_serverPath;
        uint32_t checkTimer;
};

#endif
