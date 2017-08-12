#include "uuid_gen.h"


void UUIDGEN::genUUIDv4(char o_uuid[37]){

    //Add the null terminating character
    o_uuid[36] = 0;

    //Create 128 bits of random numbers
    uint32_t randos[4];

    for(int i = 0; i<4; i++){
        randos[i] = esp_random();
    }

    //Convert them to bytes
    uint8_t* rando_bytes = (uint8_t*) randos;


    //Track which byte you're and the nibble you've extracted
    int byte_index = 0;
    bool is_low_nibble = false;

    //for the 36 characters in a uuid
    for(int i = 0; i<36; i++){

        //Add in the '-' in the correct places
        if(i == 8 || i == 13 || i == 18 || i == 23){
            o_uuid[i] = '-';
            continue;
        }

        if(is_low_nibble){  //if low nibble, just do a bit mask and get the hex character
            o_uuid[i] = ( nibbleToHex( (rando_bytes[byte_index] & 0x0F) ) );  
            is_low_nibble = false;  
            byte_index++;
        } else {            //if high nibble, move it to low nibble position and mask then get hex character
            o_uuid[i] = ( nibbleToHex( (rando_bytes[byte_index] >> 4) & 0x0F  ) );          
            is_low_nibble = true;  
        }
    }
}

void UUIDGEN::genUUIDv5(char* o_uuid, char* namespaceId, char* nameId){
    unsigned char octets[31] = {0};
    unsigned char sha1out[20] = {0};
    char name_space[33] = {0};
    
    uuidFromString(name_space, namespaceId);
    
    // convert the namespace characters to bytes
    for (int i = 0; i < 16; i++) {
        sscanf(name_space + 2*i, "%2hhx", &octets[i]);
    }

    // use the name string as it is, concatanate namespace_byes + name
    for (int i = 0; i < 16; i++) {
        octets[i+16] = (unsigned char)nameId[i];
    }

    // calculate SHA1 digest
    mbedtls_sha1((unsigned char*)octets, strlen(octets), (unsigned char*)sha1out);

    // convert SHA1 to uuid according to RFC 
    for (uint16_t i = 0; i < 16; i++) {
        o_uuid[i] = sha1out[i];
    }

    o_uuid[6] &= 0x0F; 
    o_uuid[6] |= 0x50;
    o_uuid[8] &= 0x3F; 
    o_uuid[8] |= 0x80; 
}

void uuidFromString(char* uuidIn, char* namespaceId){
    char* uuid = namespaceId;
    char* token;
    char* pos = uuidIn;

    token = strtok(uuid, "-");

    while(token != NULL){
        strncpy(pos, token, strlen(token));
        pos += strlen(token);
        token = strtok(NULL, "-");
    }
}

void printHex(uint8_t number) {
    int topDigit = number >> 4;
    int bottomDigit = number & 0x0f;
    // Print high hex digit
    Serial.print( "0123456789ABCDEF"[topDigit] );
    // Low hex digit
    Serial.print( "0123456789ABCDEF"[bottomDigit] );
}

void printUuid(uint8_t* uuidNumber) {
    int i;
    for (i=0; i<16; i++) {
        if (i==4) Serial.print("-");
        if (i==6) Serial.print("-");
        if (i==8) Serial.print("-");
        if (i==10) Serial.print("-");
        printHex(uuidNumber[i]);
    }
}

void uuidToString(uint8_t* uuidNumber, char* uuidStr) {
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

char UUIDGEN::nibbleToHex(uint8_t p_nibble){
    char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C','D', 'E','F'};
    return hex[p_nibble];
}