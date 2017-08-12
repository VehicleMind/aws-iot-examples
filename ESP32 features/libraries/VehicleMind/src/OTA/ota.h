#ifndef OTA_H_INCLUDED
#define OTA_H_INCLUDED

#include "../Tools/Verbose.h"

#include <Update.h>
#include <FreematicsSD.h>
#include <FreematicsONE.h>
#include <SPIFFS.h>
#include <MD5Builder.h>

#include "../NetworkManager/http/cell_http_client.h"
#include "../Tools/hex_txt.h"

#define DEBUG

class OTA{
    public:
        OTA(): http(100), v(true, "OTA") {};
        ~OTA(){};

        bool begin(const int storage = 2);
        bool getHTTPDocument(const char* url);
        bool getFirmwareNotes(const char* url);
        bool downloadTxtFileFromHTTP(const char* url);
        bool convertTxtToBin();
        bool updatePartition();
        bool end();

        char* getVersion();
        char* getMD5code();
        bool changeStorageMethod(const int storage);

        CellHTTPClient getHTTPClient();

    private:
        bool downloadTxtFileFromHTTP_SD(const char* url);
        bool downloadTxtFileFromHTTP_SPIFFS(const char* url);

        bool convertTxtToBin_SD();
        bool convertTxtToBin_SPIFFS();

        bool updatePartition_SD();
        bool updatePartition_SPIFFS();

        Verbose v;

        char path[24] = "/FIRMWARE/firmware.txt";
        char binPath[24] = "/FIRMWARE/firmware.bin";

        char md5Code[33];
        char version[4];
        int storageMethod;

        CellHTTPClient http; // = CellHTTPClient(100);
        HexTxt Conveter;
        UpdateClass Updater;
        SDClass SD;
};

#endif 