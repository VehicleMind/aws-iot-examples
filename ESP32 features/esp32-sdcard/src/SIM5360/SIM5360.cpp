#include "SIM5360.h"

#define XBEE_BAUDRATE 115200
#define MAX_CONN_TIME 5000

bool SIM5360::init()
    {

      sys.xbBegin(XBEE_BAUDRATE);

      // if (sendCommand("ATI\r", 50, "SIM5360")) {
      //   return true;
      // }
      for (byte n = 0; n < 10; n++) {
        // try turning on module
        sys.xbTogglePower();
        delay(3000);
        // discard any stale data
        sys.xbPurge();
        for (byte m = 0; m < 3; m++) {
          if (sendCommand("ATI\r"))
            return true;
        }
      }
      return false;
    }

bool SIM5360::setup(const char* apn, const char* cellusr, const char* cellpwd, bool only3G, bool roaming)
    {
      uint32_t t = millis();
      bool success = false;

      sendCommand("ATE0\r");
      if (only3G) sendCommand("AT+CNMP=14\r"); // use WCDMA only
      do {
        do {
          Serial.print('.');
          delay(500);
          success = sendCommand("AT+CPSI?\r", 1000, "Online");
          if (success) {
            if (!strstr_P(buffer, PSTR("NO SERVICE")))
              break;
            success = false;
          } else {
            if (strstr_P(buffer, PSTR("Off"))) break;
          }
        } while (millis() - t < 60000);
        if (!success) break;

        t = millis();
        do {
          success = sendCommand("AT+CREG?\r", 5000, roaming ? "+CREG: 0,5" : "+CREG: 0,1");
        } while (!success && millis() - t < 30000);
        if (!success) break;

        do {
          success = sendCommand("AT+CGREG?\r",1000, roaming ? "+CGREG: 0,5" : "+CGREG: 0,1");
        } while (!success && millis() - t < 30000);
        if (!success) break;

        do {
          sprintf_P(buffer, PSTR("AT+CGSOCKCONT=1,\"IP\",\"%s\"\r"), apn);
          success = sendCommand(buffer);
        } while (!success && millis() - t < 30000);
        if (!success) break;

      do
      {
        sprintf(buffer, "AT+CSOCKAUTH=1,1,\"%s\",\"%s\"\r", cellpwd, cellusr); //Set PDP-IP authentication type
        success = sendCommand(buffer);
      } while (!success && millis() - t < 30000);
      if (!success)
        break;

        success = sendCommand("AT+CSOCKSETPN=1\r");
        if (!success) break;

        success = sendCommand("AT+CIPMODE=0\r");
        if (!success) break;

        sendCommand("AT+NETOPEN\r");
      } while(0);
      if (!success) Serial.println(buffer);
      return success;
    }

bool SIM5360::initGPS()
    {
      for (;;) {
        Serial.println("INIT GPS");
        if (sendCommand("AT+CGPS=1\r")) break;
        Serial.println(buffer);
        sendCommand("AT+CGPS=0\r");
        delay(3000);
      }

      Serial.println(buffer);

      for (;;) {
        sendCommand("AT+CGPSINFO\r");
        Serial.println(buffer);
        delay(3000);
      }
      return true;
    }

char * SIM5360::getIP()
    {
      uint32_t t = millis();
      char *ip = 0;
      do {
        if (sendCommand("AT+IPADDR\r", 5000, "\r\nOK\r\n", true)) {
          char *p = strstr(buffer, "+IPADDR:");
          if (p)
          {
            ip = p + 9;
            if (*ip != '0') {
              break;
            }
          }
        }
        delay(500);
        ip = 0;
      } while (millis() - t < 15000);
      return strdup(ip);
    }

int SIM5360::getSignal()
    {
        if (sendCommand("AT+CSQ\r", 500)) {
            char *p = strchr(buffer, ':');
            if (p) {
              p += 2;
              int db = atoi(p) * 10;
              p = strchr(p, '.');
              if (p) db += *(p + 1) - '0';
              return db;
            }
        }
        return -1;
    }

char * SIM5360::getOperatorName()
    {
        // display operator name
        if (sendCommand("AT+COPS?\r") == 1) {
            char *p = strstr(buffer, ",\"");
            if (p) {
                p += 2;
                char *s = strchr(p, '\"');
                if (s) *s = 0;
                strcpy(buffer, p);
                return strdup(buffer);
            }
        }
        return "NOOP";
    }

bool SIM5360::sendCommand(const char* cmd, unsigned int timeout, const char* expected, bool terminated)
    {
      if (cmd) {
        sys.xbWrite(cmd);
      }
      buffer[0] = 0;
      //memset(&buffer[0], '\0', sizeof(buffer));
      byte ret = sys.xbReceive(buffer, sizeof(buffer), timeout, &expected, 1);
      if (ret) {
        if (terminated) {
          char *p = strstr(buffer, expected);
          if (p) *p = 0;
        }
        return true;
      } else {
        return false;
      }
    }
