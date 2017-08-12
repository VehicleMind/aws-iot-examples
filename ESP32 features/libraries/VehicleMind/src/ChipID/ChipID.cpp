#include "ChipID.h"

/**
* Returns a unique chip id composed of the MAC Address and the
* SIM5360 IMEI number.
* The network must be connected before calling this function.
*/

void ChipID::getChipID(char o_buffer[28])
{
  // retrieve the MAC address stored in efuse
  uint8_t macID[6];
  char macBuffer[13];
  esp_efuse_mac_get_default((uint8_t*) (&macID));
  sprintf(macBuffer, "%02x%02x%02x%02x%02x%02x", macID[0], macID[1], macID[2], macID[3], macID[4], macID[5]);

  //Send a CGSN AT command and parse the 15 digit IMEI number
  m_tcp_connector.sendATCommand("AT+CGSN\r");
  char *IMEI = strstr(m_tcp_connector.m_buffer, "\r\nOK\r\n");
  *(IMEI - 2) = 0;
  IMEI -= 17;

  memcpy(o_buffer, macBuffer, 12);
  memcpy(o_buffer+12, IMEI, 15);
  o_buffer[27] = 0;
}

void ChipID::getSimIMEI(char o_buffer[16]){
  m_tcp_connector.sendATCommand("AT+CGSN\r");
  char *IMEI = strstr(m_tcp_connector.m_buffer, "\r\nOK\r\n");
  *(IMEI - 2) = 0;
  IMEI -= 17;

  memcpy(o_buffer, IMEI, 15);
  o_buffer[16] = 0;
}

void ChipID::getMACAddress(char o_buffer[13]){
  uint8_t macID[6];
  char macBuffer[13];
  esp_efuse_mac_get_default((uint8_t*) (&macID));
  sprintf(macBuffer, "%02x%02x%02x%02x%02x%02x", macID[0], macID[1], macID[2], macID[3], macID[4], macID[5]);

  memcpy(o_buffer, macBuffer, 12);
  o_buffer[12] = 0;
}
