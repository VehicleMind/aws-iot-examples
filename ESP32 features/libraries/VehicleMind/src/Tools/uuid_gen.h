//---------------------------------------------------
//INCLUDE GUARD
#ifndef UUID_GEN_H_INCLUDED
#define UUID_GEN_H_INCLUDED

//---------------------------------------------------
//DEPS
#include <Arduino.h>
#include <esp_system.h>
#include "mbedtls/sha1.h"

#include <time.h>
#include <sys/time.h>
#include "WString.h"

//---------------------------------------------------
//HEADER

class UUIDGEN{

public:
    static void genUUIDv4(char o_uuid[37]);
    static void genUUIDv5(char o_uuid[37], char* namespaceId, char* nameId);

private:
    static char nibbleToHex(uint8_t);
};

    void printUuid(uint8_t* uuidNumber);
    void uuidToString(uint8_t* uuidNumber, char* uuidStr);
    void printHex(uint8_t number);
    void uuidFromString(char* uuidNumber, char* uuidStr);

#endif