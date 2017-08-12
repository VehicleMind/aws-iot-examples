#include "cell_http_client.h"

bool CellHTTPClient::begin(const char *serverUrl){
#ifdef DEBUG
      v.vpl(2, serverUrl);
#endif
    struct parsed_url* purl = parse_url(serverUrl);
    size_t hlen = strlen(purl->host);
    m_serverUrl = (char *)malloc(hlen + 1);
    strcpy(m_serverUrl, purl->host);
    m_serverUrl[hlen] = '\0';
    size_t plen = strlen(purl->path);
    m_serverPath = (char *)malloc(plen + 1 + 1); /* one for "/", one for trailing zero */
    strcpy(m_serverPath, "/");
    strcat(m_serverPath, purl->path);
    m_serverPath[plen + 1] = '\0';

    if(purl->port){
        m_serverPort = atoi(purl->port);
    }
    else{
        if(strcmp("https", purl->scheme) == 0){
            m_serverPort = 443;
        }
    }
#ifdef DEBUG
    v.vpl(2, "Starting HTTP Client");
#endif
    while(!httpStart()){
        // delay(1000);
#ifdef DEBUG
        v.vpl(2, "Trying to start...");
#endif
    }

#ifdef DEBUG
    v.vpl(2, "HTTP client started");
#endif

    while(!httpOpen()){
#ifdef DEBUG
        v.vpl(2, "Trying to open...");
#endif
        // delay(2000);
    }

#ifdef DEBUG
    v.vpl(2, "HTTP client is opened");
#endif

    return true;
}

bool CellHTTPClient::httpClose(){
    return sendATCommand("AT+CHTTPSCLSE\r", 3000, "OK");
}

bool CellHTTPClient::httpStop(){
    return sendATCommand("AT+CHTTPSSTOP\r", 3000, "OK");
}

bool CellHTTPClient::httpStart(){
    return sendATCommand("AT+CHTTPSSTART\r", 3000, "OK");
}

bool CellHTTPClient::httpOpen(){
    sprintf_P(m_buffer, PSTR("AT+CHTTPSOPSE=\"%s\",%u,2\r"), m_serverUrl, m_serverPort);
    // sys.xbPurge();
    xbPurge();
    return sendATCommand(m_buffer, MAX_CONN_TIME, "\r\nOK\r\n", false);
}

// generate HTTP header
unsigned int CellHTTPClient::genHttpHeader(HTTP_METHOD method, bool keepAlive, const char* payload, int payloadSize){
    char* p = m_buffer;
    p += sprintf_P(p, PSTR("%s %s HTTP/1.1\r\nUser-Agent: ONE\r\nHost: %s\r\nConnection: %s\r\n"),
                   method == HTTP_GET ? "GET" : "POST", m_serverPath, m_serverUrl, keepAlive ? "keep-alive" : "close");
    if (method == HTTP_POST){
        p += sprintf_P(p, PSTR("Content-length: %u\r\n"), payloadSize);
    }
    p += sprintf_P(p, PSTR("\r\n\r"));
    return (unsigned int)(p - m_buffer);
}

bool CellHTTPClient::httpSend(HTTP_METHOD method, bool keepAlive, const char* payload, int payloadSize){
    unsigned int headerSize = genHttpHeader(method, keepAlive, payload, payloadSize);

#ifdef DEBUG
    v.vp(2, "The header size is: ");
    v.vpl(2, headerSize);
#endif
    sprintf_P(m_buffer, PSTR("AT+CHTTPSSEND=%u\r"), headerSize + payloadSize);
    if(!sendATCommand(m_buffer, 100, ">"))
    {
#ifdef DEBUG
        v.vpl(2, m_buffer);
        v.vpl(2, "Connection closed");
#endif
    }

    // send HTTP header
    genHttpHeader(method, keepAlive, payload, payloadSize);
    xbWrite(m_buffer);
    // send POST payload if any
    if (payload){
        xbWrite(m_buffer);
    }

    m_buffer[0] = 0;
    if(sendATCommand("AT+CHTTPSSEND\r")){
        checkTimer = millis();
        return true;
    }
    else{
#ifdef DEBUG
        v.vpl(2, "HTTP send returned false: ", m_buffer);
#endif
        return false;
    }
}

int CellHTTPClient::httpReceive(){
    int received = 0;

    if (sendATCommand("AT+CHTTPSRECV=1024\r", MAX_CONN_TIME, "\r\n+CHTTPSRECV: 0", true)){
        char* p = strstr(m_buffer, "+CHTTPSRECV:");
        if(p){
            p = strchr(p, ',');
            if(p){
                received = atoi(p + 1);
                if(m_payload){
                    char* q = strchr(p, '\n');
                    strcpy(m_payload, q ? (q + 1) : p);
                }
            }
        }
    }
    return received;
}

char* CellHTTPClient::RecieveData(int &TempNum){
    TempNum = 0;
    if (sendATCommand("AT+CHTTPSRECV=1024\r", 200, "\r\n+CHTTPSRECV: 0", true)){
        char* p = strstr(m_buffer, "+CHTTPSRECV:");
        if(p){
            p = strchr(p, ',');
            if(p){
                TempNum = atoi(p + 1);
                if (m_payload){
                    char *q = strchr(p, '\n');
                    strcpy(m_payload, q ? (q + 1) : p);
                }
            }
        }
        return m_payload;
    }
    else{
        return nullptr;
    }
}

byte CellHTTPClient::checkbuffer(const char *expected, unsigned int timeout){
    if(strstr(m_buffer, expected)){
        return 1;
    }
    // if not, receive a chunk of data from xBee module and look for expected string
    // COBDSPI::xbReceive(char* buffer, int bufsize, unsigned int timeout, const char** expected, byte expectedCount, const int xbTimeout)
    // byte ret = xbReceive(m_buffer, sizeof(m_buffer), timeout, &expected, 1, 100) != 0;
    byte ret = xbReceive(m_buffer, sizeof(m_buffer), timeout, &expected, 1) != 0;
    if(ret == 0){
        // timeout
        return (millis() - checkTimer < timeout) ? 0 : 2;
    }
    else{
        return ret;
    }
}

int CellHTTPClient::getSize(){

    Serial.println("Get Size:");
    Serial.println(m_buffer);

    int contentSize = 0;
    char* p = strstr(m_buffer, "Content-Length:");
    if(p){
        p = strchr(p, ' ');
        if(p){
            contentSize = atoi(p + 1);
        }
    }
#ifdef DEBUG
    v.vp(3, "Content Size: ");
    v.vpl(3, contentSize);
#endif
    return contentSize;
}

char* CellHTTPClient::getContentType(){
    char* p = strstr(m_buffer, "Content-Type:");
    if(p){
        p = strchr(p, ' ');
        if(p){
            char* q = strchr(p, '\n');
            return strndup(p + 1, (q - p));
        }
    }
    return nullptr;
}

char* CellHTTPClient::getStreamPtr(){
    Serial.println("Get Stream pointer, printed");
    Serial.println(m_payload);
    char* p = strstr(m_payload, "Content-Type:");
    if(p){
        p = strstr(p, "\r\n\r\n");
        if(p){
            return (p + 4);
        }
    }
    return nullptr;
}

bool CellHTTPClient::GETrequest(){
    bool ret = httpSend(HTTP_GET, true);
    // The delay is important
    delay(2000);
    if(int recdBytes = httpReceive()){
#ifdef DEBUG
        v.vp(2, "Received bytes: ");
        v.vpl(2, recdBytes);
#endif
    }

    return ret;
}

bool CellHTTPClient::reset(){
    bool ret = true;
    ret = httpClose();
    // ret = httpStop();
    int CloseIndex = 0;
    int StopIndex = 0;
    //////////////////////////////////
    /// HAVE SOMETHING TO CHECK FOR ERROR MESSAGE
    //////////////////////////////////
    while(!httpClose() && CloseIndex < 10){
        CloseIndex++;
#ifdef DEBUG
        v.vpl(2, "trying to Close");
#endif
        // delay(100);
    }

#ifdef DEBUG
    v.vpl(2, "http closed");
#endif

    while(!httpStop() && StopIndex < 10){
        StopIndex++;
#ifdef DEBUG
        v.vpl(2, "trying to Stop");
#endif
        // delay(100);
    }

#ifdef DEBUG
    v.vpl(2, "http Stopped");
#endif

    m_payload[0] = '\0';

    // httpOpen();
    // Serial.printf("HTTP closed, %u.\n", ret);
    return ret;
}

bool CellHTTPClient::end(){
    bool ret = httpClose();
    ret = httpStop();
    // Serial.printf("HTTP closed, %u.\n", ret);
    return ret;
}
