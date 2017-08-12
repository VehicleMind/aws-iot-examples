#include "CellHTTPClient.h"
#include "../UrlParser/UrlParser.h"

bool CellHTTPClient::begin(char *serverUrl) {
  struct parsed_url * purl = parse_url(serverUrl);
  size_t hlen = strlen(purl->host);
  m_serverUrl = (char *)malloc(hlen + 1);
  strcpy(m_serverUrl, purl->host);
  m_serverUrl[hlen] = '\0';
  size_t plen = strlen(purl->path);
  m_serverPath = (char *)malloc(plen  + 1 + 1); /* one for "/", one for trailing zero */
  strcpy(m_serverPath,"/");
  strcat(m_serverPath, purl->path);
  m_serverPath[plen+1] = '\0';

  if(purl->port) {
    m_serverPort = atoi(purl->port);
  } else {
    if (strcmp("https", purl->scheme) == 0)
      m_serverPort = 443;
  }

  CellHTTPClient::httpOpen();
  return CellHTTPClient::httpConnect();

}

unsigned int CellHTTPClient::genHttpHeader(HTTP_METHOD method, bool keepAlive, const char* payload, int payloadSize)
    {
        // generate HTTP header
        char *p = buffer;
        p += sprintf_P(p, PSTR("%s %s HTTP/1.1\r\nUser-Agent: ONE\r\nHost: %s\r\nConnection: %s\r\n"),
          method == HTTP_GET ? "GET" : "POST", m_serverPath, m_serverUrl, keepAlive ? "keep-alive" : "close");
        if (method == HTTP_POST) {
          p += sprintf_P(p, PSTR("Content-length: %u\r\n"), payloadSize);
        }
        p += sprintf_P(p, PSTR("\r\n\r"));
        return (unsigned int)(p - buffer);
    }

bool CellHTTPClient::httpSend(HTTP_METHOD method, bool keepAlive, const char* payload, int payloadSize)
    {
      unsigned int headerSize = genHttpHeader(method, keepAlive, payload, payloadSize);
      // issue HTTP send command
      sprintf_P(buffer, PSTR("AT+CHTTPSSEND=%u\r"), headerSize + payloadSize);
      if (!sendCommand(buffer, 100, ">")) {
        Serial.println(buffer);
        Serial.println("Connection closed");
      }
      // send HTTP header
      genHttpHeader(method, keepAlive, payload, payloadSize);
      sys.xbWrite(buffer);
      // send POST payload if any
      if (payload) sys.xbWrite(payload);
      buffer[0] = 0;
      if (sendCommand("AT+CHTTPSSEND\r")) {
        checkTimer = millis();
        return true;
      } else {
        Serial.println(buffer);
        return false;
      }
    }

int CellHTTPClient::httpReceive()
  {
    int received = 0;
    // wait for RECV EVENT
    delay(3000);
    checkbuffer("RECV EVENT", MAX_CONN_TIME);
    /*
      +CHTTPSRECV:XX\r\n
      [XX bytes from server]\r\n
       \r\n+CHTTPSRECV: 0\r\n
    */
    if (sendCommand("AT+CHTTPSRECV=1024\r", MAX_CONN_TIME, "\r\n+CHTTPSRECV: 0", true)) {
        char *p = strstr(buffer, "+CHTTPSRECV:");
        if (p)
        {
          p = strchr(p, ',');
          if (p) {
            received = atoi(p + 1);
            if (m_payload) {
              char *q = strchr(p, '\n');
                strcpy(m_payload, q ? (q + 1) : p);
            }
          }
        }
    }
     return received;
  }

byte CellHTTPClient::checkbuffer(const char* expected, unsigned int timeout)
    {
      // check if expected string is in reception buffer
      if (strstr(buffer, expected)) {
        return 1;
      }
      // if not, receive a chunk of data from xBee module and look for expected string
      byte ret = sys.xbReceive(buffer, sizeof(buffer), timeout, &expected, 1) != 0;
      if (ret == 0) {
        // timeout
        return (millis() - checkTimer < timeout) ? 0 : 2;
      } else {
        return ret;
      }
    }

int CellHTTPClient::getSize()
{
  int contentSize = 0;
  char *p = strstr(m_payload, "Content-Length:");
  if (p)
  {
    p = strchr(p, ' ');
    if (p)
    {
      contentSize = atoi(p + 1);
    }
  }
  return contentSize;
}

char * CellHTTPClient::getContentType()
{
  char *p = strstr(m_payload, "Content-Type:");
  if (p)
  {
    p = strchr(p, ' ');
    if (p)
    {
      char *q = strchr(p, '\n');
      return strndup(p + 1, (q - p));
    }
  }
}

char * CellHTTPClient::getStreamPtr()
{
  char *p = strstr(m_payload, "Content-Type:");
  if (p)
  {
    p = strstr(p, "\r\n\r\n");
    if (p)
    {
      return (p + 4);
    }
  }
}