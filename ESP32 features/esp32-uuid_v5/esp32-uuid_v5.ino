#include "mbedtls/sha1.h"

void printHex(byte number) {
  int topDigit = number >> 4;
  int bottomDigit = number & 0x0f;
  // Print high hex digit
  Serial.print( "0123456789ABCDEF"[topDigit] );
  // Low hex digit
  Serial.print( "0123456789ABCDEF"[bottomDigit] );
}

void printUuid(byte* uuidNumber) {
  int i;
  for (i=0; i<16; i++) {
    if (i==4) Serial.print("-");
    if (i==6) Serial.print("-");
    if (i==8) Serial.print("-");
    if (i==10) Serial.print("-");
    printHex(uuidNumber[i]);
  }
}

void uuidToString(byte* uuidNumber, char* uuidStr) {
  char temp_uuid[33];
  char *pos = temp_uuid;
  char seg1[9] = {0};
  char seg2[5] = {0};
  char seg3[5] = {0};
  char seg4[5] = {0};
  char seg5[13] = {0};

  for (int i=0; i<16; i++) {
    sprintf(temp_uuid + 2*i, "%02x", uuidNumber[i]);
  }
  temp_uuid[33]= '\0';
  
  strncpy(seg1, pos, 8);
  strncpy(seg2, pos+8, 4);
  strncpy(seg3, pos + 12, 4);
  strncpy(seg4, pos + 16, 4);
  strncpy(seg5, pos + 20, 12);

  strcat(uuidStr, seg1);
  strcat(uuidStr, "-");
  strcat(uuidStr, seg2);
  strcat(uuidStr, "-");
  strcat(uuidStr, seg3);
  strcat(uuidStr, "-");
  strcat(uuidStr, seg4);
  strcat(uuidStr, "-");
  strcat(uuidStr, seg5);
}

void uuidFromString(char* uuidNumber, char* uuidStr) {
  char * uuid_in = uuidStr;
  char * token;
  char * pos = uuidNumber;

  token = strtok(uuid_in, "-");
  
  while (token != NULL) {
    strncpy(pos, token, strlen(token));
    pos += strlen(token);
    token = strtok(NULL, "-");    
  }
}

void createUUIDv5Binary(char* uuid_v5, char* uuid_in, char* the_name) {
  unsigned char octets[31] = {0};
  unsigned char sha1out[20] = {0}; // store SHA1 digest
  char name_space[33] = {0};
  
  uuidFromString(name_space, uuid_in); // namespace is without "-"
  
  // convert the namespace characters to bytes
  for (int i = 0; i < 16; i++) {
    sscanf(name_space + 2*i, "%2hhx", &octets[i]);
  }

  // use the name string as it is, concatanate namespace_byes + name
  for (int i = 0; i < 16; i++) {
    octets[i+16] = (unsigned char)the_name[i];
  }

  // calculate SHA1 digest
  mbedtls_sha1((unsigned char*)octets, strlen(octets), (unsigned char*)sha1out);

  // convert SHA1 to uuid according to RFC 
  for (uint16_t i = 0; i < 16; i++) {
    uuid_v5[i] = sha1out[i];
  }

  uuid_v5[6] &= 0x0F; 
  uuid_v5[6] |= 0x50;
  uuid_v5[8] &= 0x3F; 
  uuid_v5[8] |= 0x80;
}

void setup()
{

  char name_space[] = "e728b802-0fff-5b7b-bf47-51bacb45d446"; // uuid to use for namespace
  char the_name[] = "865357020165831"; // use only 15 digit IMEI number
  
  unsigned char uuid_v5[16] = {0}; // uuid v5 binary
  char uuid_v5_str[37] = {0}; // uuid v5 string

  Serial.begin(115200);
  delay(5000);

  // generare UUID v5
  createUUIDv5Binary(uuid_v5, name_space, the_name);

  Serial.println("UUID v5 num: ");
  printUuid((byte *)uuid_v5);
  Serial.println();

  // convert to string and print (both should be identical)
  uuidToString((byte *)uuid_v5, uuid_v5_str);
  Serial.println("UUID v5 str: ");
  Serial.println(uuid_v5_str);
}

void loop()
{
}
