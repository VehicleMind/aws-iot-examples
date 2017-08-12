#include "EEPROM.h"

bool fail = true;
#define EEPROM_SIZE 64
void setup()
{
  Serial.begin(115200);
  if (EEPROM.begin(EEPROM_SIZE))  {
    fail=false;
    EEPROM.write(0,1);
    EEPROM.write(1,2);
    EEPROM.write(2,3);
    EEPROM.write(3,4);
    EEPROM.commit();
  }
}xsz
void loop() {}
