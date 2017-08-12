#include "ota.h"

bool OTA::updatePartition(){
    if(storageMethod == 1){
        return updatePartition_SD();
    }
    else if(storageMethod == 2){
        return updatePartition_SPIFFS();
    }
    return false;
}

bool OTA::begin(const int storage){
    storageMethod = storage;

#ifdef DEBUG
    v.vpl(1, "Checking network connection");
#endif
    if(http.checkConnection()){
#ifdef DEBUG
        v.vpl(1, "Network connection verified");
#endif
    }
    else{
#ifdef DEBUG
        v.vpl(1, "Connection failed.");
#endif
        return false;
    }

    if(storageMethod == 2){
        SPIFFS.begin(true);
    }

    version[3] = '\0';

    return true;
}

bool OTA::end(){
    Updater.end();
    SPIFFS.end();
    http.end();

    storageMethod = 0;
    version[0] = '\0';
    md5Code[0] = '\0';

    return true;
}

bool OTA::downloadTxtFileFromHTTP(const char* url){
    if(!getHTTPDocument(url)){
#ifdef DEBUG
        v.vpl(1, "Could not retrieve document.");
#endif
        return false;
    }

    char* contentType = http.getContentType();
    if(!strcmp(contentType, "text/plain\n")){
#ifdef DEBUG
        v.vp(1, "Content Type: ");
        v.vpl(1, contentType);
        v.vpl(1, "Incorrect content type. The file is not a text file.");
#endif
        http.reset();
        return false;
    }

    if(storageMethod == 1){
        return downloadTxtFileFromHTTP_SD(url);
    }
    else if(storageMethod == 2){
        return downloadTxtFileFromHTTP_SPIFFS(url);
    }

    http.reset();
    return false;
}

bool OTA::downloadTxtFileFromHTTP_SPIFFS(const char* url){
    if(!SPIFFS.begin(true)){
#ifdef DEBUG
        v.vpl(1, "SPIFFS could not be started.");
#endif
        http.reset();
        return false;
    }
    SPIFFS.remove(path);
    fs::File txtFile = SPIFFS.open(path, FILE_WRITE);
    if(!txtFile){
#ifdef DEBUG
        v.vpl(1, "The txt file failed to be opened.");
#endif
        http.reset();
        return false;
    }

    int RecievedDataSize = 0;
    int totalWritten = 0;
    int tempWritten = 0;
    int RecievedDataTotal = 0;

    int antiInfinityLoop = 0;

    char* stream = http.getStreamPtr();
    size_t size = strlen(stream);
#ifdef DEBUG
    v.vpl(1, "Size of Stream: %u\n", size);
#endif

    tempWritten = txtFile.write((const uint8_t*)stream, size);

    totalWritten += tempWritten;

    int httpReceiveFileSize = http.getSize();
#ifdef DEBUG
    v.vp(1, "\nSize of the File: ");
    v.vpl(1, httpReceiveFileSize);

    v.vpl(1, "----------------------------------------------------------");
#endif
    while(totalWritten < (httpReceiveFileSize)){
        stream = http.RecieveData(RecievedDataSize);

        RecievedDataTotal += RecievedDataSize;
        tempWritten = txtFile.write((const uint8_t*)stream, RecievedDataSize);
        totalWritten += tempWritten;

#ifdef DEBUG
        Serial.printf("Recieved Data Size: %u. Total Recieved Data: %u. temp Written: %u. Total Written Size: %u. Current File Size: %u.\n", RecievedDataSize, RecievedDataTotal, tempWritten, totalWritten, txtFile.size());
#endif
        if(RecievedDataSize == 0){
#ifdef DEBUG
            Serial.printf("\n%u / %u bytes retrieved.", totalWritten, httpReceiveFileSize);
#endif
        }
#ifdef DEBUG
        v.vp(1, ".");
#endif
        if(RecievedDataSize == 0){
            antiInfinityLoop++;
            if(antiInfinityLoop >= 40){
#ifdef DEBUG
                v.vpl(1, "Error: unable to retrieve file.");
#endif
                http.reset();
                return false;
            }
        }
        else{
            antiInfinityLoop = 0;
        }
    }

    txtFile.close();

    txtFile = SPIFFS.open(path, FILE_READ);
#ifdef DEBUG
    v.vpl(1, "The size of text the file is %u. \n", txtFile.size());
#endif

    MD5Builder md5;
    md5.begin();
    md5.addStream(txtFile, txtFile.size());
    md5.calculate();

#ifdef DEBUG
    v.vp(1, "MD5 code of the recieved file: ");
    v.vpl(1, md5.toString().c_str());
    Serial.printf("MD5 code retrieved from Notes: %s\n", md5Code);
#endif

    if(strncmp(md5.toString().c_str(), md5Code, 32)){
#ifdef DEBUG
        v.vpl(1, "The MD5 code is incorrect.");
#endif
        http.reset();
        return false;
    }

    txtFile.close();

    return true;
}

bool OTA::convertTxtToBin_SPIFFS(){
    if(!SPIFFS.begin(true)){
#ifdef DEBUG
        v.vpl(1, "SPIFFS not started!");
#endif
        http.reset();
        return false;
    }

    fs::File txtFile = SPIFFS.open(path, FILE_READ);

#ifdef DEBUG
    v.vpl(1, "txtFile size: %u\n", txtFile.size());
#endif
    if(!SPIFFS.exists(binPath)){
#ifdef DEBUG
        v.vpl(1, "The text file does not exist.");
#endif
    }

    fs::File binFile = SPIFFS.open(binPath, FILE_WRITE);
#ifdef DEBUG
    v.vpl(1, "binFile.size(): %u\n", binFile.size());
#endif

    if(!txtFile){
#ifdef DEBUG
        v.vpl(1, "The txt file failed to be opened.");
#endif
        http.reset();
        return false;
    }
    if(!binFile){
#ifdef DEBUG
        v.vpl(1, "The bin file failed to be opened.");
#endif
        http.reset();
        return false;
    }

#ifdef DEBUG
    v.vpl(1, "Conversion phase");
#endif

    Conveter.convert_text2binSPIFFS(txtFile, binFile);

#ifdef DEBUG
    v.vpl(1, "Conversion Ended.");
#endif

    binFile.close();
    txtFile.close();

    return true;
}

bool OTA::updatePartition_SPIFFS(){
    fs::File binFile = SPIFFS.open(binPath, FILE_READ);
#ifdef DEBUG
    v.vpl(1, "The size of the bin file is: %u\n", binFile.size());
#endif

    if(Updater.begin(binFile.size(), U_FLASH)){
        size_t written = Updater.writeStream(binFile);

        if(written == binFile.size()){
            bool result = false;
            if((result = Updater.end())){
#ifdef DEBUG
                v.vpl(1, "OTA done!");
#endif
                if(Updater.isFinished()){
#ifdef DEBUG
                    v.vpl(3, "Update successfully completed. Ready to reboot.");
#endif
                }
                else{
#ifdef DEBUG
                    v.vp(3, "OTA Error Occurred. Error #: ");
                    v.vpl(3, Updater.getError());
#endif              
                    http.reset();
                    return false;
                }
            }
            else{
#ifdef DEBUG
                v.vp(2, "OTA Error Occurred. Error #: ");
                v.vpl(2, Updater.getError());
#endif
                http.reset();
                return false;
            }
        }
        else{
#ifdef DEBUG
            v.vp(1, "OTA Error Occurred. Error #: ");
            v.vpl(1, Updater.getError());
#endif
            http.reset();
            return false;
        }
    }
    
    binFile.close();
    return true;
}

bool OTA::convertTxtToBin(){
    if(storageMethod == 1){
        return convertTxtToBin_SD();
    }
    else if(storageMethod == 2){
        return convertTxtToBin_SPIFFS();
    }

    http.reset();
    return false;
}

bool OTA::getFirmwareNotes(const char* url){
    if(!getHTTPDocument(url)){
#ifdef DEBUG
        v.vpl(1, "Could not retrieve document.");
#endif
        return false;
    }

    int recdBytes = http.getSize();
    if(recdBytes){
#ifdef DEBUG
        v.vp(1, recdBytes);
        v.vpl(1, " Bytes");
#endif
    }
    else{
#ifdef DEBUG
        v.vpl(1, "HTTP receive failed");
#endif
        http.reset();
        return false;
    }

    char* contentType = http.getContentType();
    if(!strcmp(contentType, "text/plain\n")){
#ifdef DEBUG
        v.vp(1, "Content Type:");
        v.vpl(1, contentType);
        v.vpl(1, "Incorrect content type. The file is not a text file.");
#endif
        http.reset();
        return false;
    }

    char* stream2 = http.getStreamPtr();
    char* p = strstr(stream2, "version:");
    if(p){
        p = strchr(p, ' ');
        if(p){
            memcpy(version, p + 1, 3);
            // version = atoi(p + 1);
        }
    }
    char* q = strstr(stream2, "md5:");
    if(q){
        q = strchr(q, ' ');
        if(q){
            q = q + 1;
            strncpy(md5Code, q, 33);
        }
    }

#ifdef DEBUG
    v.vp(1, "Version: ");
    v.p(version);
    v.l();
    Serial.printf("MD5 code: %s.\n", md5Code);
#endif
    http.reset();

    return true;
}

bool OTA::getHTTPDocument(const char* url){
    if(!http.begin(url)){
#ifdef DEBUG
        v.vpl(1, "HTTP Error connecting");
        v.vpl(1, http.m_buffer);
#endif
        http.reset();
        return false;
    }

    if(!http.GETrequest()){
#ifdef DEBUG
        v.vpl(1, "HTTP failed send request");
#endif
        http.reset();
        return false;
    }
    else{
#ifdef DEBUG
        v.vpl(1, "HTTP send OK");
#endif
    }

    return true;
}

char* OTA::getVersion(){
    return version;
}

char* OTA::getMD5code(){
    return md5Code;
}

bool OTA::changeStorageMethod(const int storage){
    storageMethod = storage;

    return true;
}


CellHTTPClient OTA::getHTTPClient(){
    return http;
}

// ///////////////////////////////////////////////////////////
//
//  SD specific mehtods. Not up to date
//  Trying to find a way to not have to write two different
//  that do the same thing but for SPIFFS or SD
//
// ///////////////////////////////////////////////////////////

bool OTA::downloadTxtFileFromHTTP_SD(const char* url){
    pinMode(PIN_SD_CS, OUTPUT);
    bool worked = SD.begin();

    if(worked){
#ifdef DEBUG       
        v.vpl(1, "Card Size %u.\n", SD.cardSize());
#endif
    }
    else{
#ifdef DEBUG
        v.vpl(1, "SD card could not be opened.");
#endif
        http.reset();
        return false;
    }

    if(SD.exists(path)){
        SD.remove(path);
    }
    else{
        SD.mkdir(path);
    }
    SDLib::File txtFile = SD.open(path, SD_FILE_WRITE);
    if (!txtFile)
    {
        Serial.println("Failed to open txt file.");
        return false;
    }

    if(SD.exists(binPath)){
        SD.remove(binPath);
    }
    else{
        SD.mkdir(binPath);
    }

    char* stream = http.getStreamPtr();

    int total = 0;
    int httpReceiveFileSize = http.getSize();

    int RecievedDataSize = 0;
    int totalWritten = 0;
    int tempWritten = 0;

    while(totalWritten < (httpReceiveFileSize)){
        stream = http.RecieveData(RecievedDataSize);

        total += RecievedDataSize;
        tempWritten = txtFile.write((const uint8_t*)stream, RecievedDataSize);
        totalWritten += tempWritten;

#ifdef DEBUG
        v.vpl(1, "Recieved Data Size: %u  ", RecievedDataSize);
        v.vpl(1, "temp Written Size: %u\n", tempWritten);
#else
        if(RecievedDataSize == 0){
#ifdef DEBUG
            v.vpl(1, "\n%u / %u bytes retrieved.", totalWritten, httpReceiveFileSize);
#endif
        }
#endif
        delay(2000);
#ifdef DEBUG
        v.vp(1, ".");
#endif
    }
    Serial.println();

    txtFile.close();

    txtFile = SD.open(path, SD_FILE_READ | SD_FILE_WRITE);
#ifdef DEBUG
    v.vpl(1, "The size of text the file is %u. \n", txtFile.size());
#endif
    txtFile.close();
    
    MD5Builder md5;
    md5.begin();
    md5.addStream(txtFile, txtFile.size());
    md5.calculate();
#ifdef DEBUG
    Serial.printf("MD5 code of the recieved file: %s\n", md5.toString());
    Serial.printf("MD5 code retrieved from Notes: %s\n", md5Code);
#endif

    if(strncmp(md5.toString().c_str(), md5Code, 32)){
#ifdef DEBUG
        v.vpl(1, "The MD5 code is incorrect.");
#endif
        http.reset();
        return false;
    }
    return true;
}

bool OTA::updatePartition_SD(){
    SDLib::File binFile = SD.open(path, SD_FILE_READ);

    if(Updater.begin(binFile.size(), U_FLASH)){
        size_t written = Updater.writeStream(binFile);

        if(written == binFile.size()){
            bool result = false;
            if((result = Updater.end())){
#ifdef DEBUG
                v.vpl(1, "OTA done!");
#endif
                if(Updater.isFinished()){
#ifdef DEBUG
                    v.vpl(3, "Update successfully completed. Ready to reboot.");
#endif
                }
                else{
#ifdef DEBUG
                    v.vp(3, "OTA Error Occurred. Error #: ");
                    v.vpl(3, Updater.getError());
#endif
                    http.reset();
                    return false;
                }
            }
            else{
#ifdef DEBUG
                v.vp(2, "OTA Error Occurred. Error #: ");
                v.vpl(2, Updater.getError());
#endif
                http.reset();
                return false;
            }
        }
        else{
#ifdef DEBUG
            v.vp(1, "OTA Error Occurred. Error #: ");
            v.vpl(1, Updater.getError());
#endif
            http.reset();
            return false;
        }
    }
    
    binFile.close();
    return true;
}

bool OTA::convertTxtToBin_SD(){
    SDLib::File txtFile = SD.open(path, SD_FILE_READ);
    if(SD.exists(binPath)){
        SD.remove(binPath);
    }
    else{
        SD.mkdir(binPath);
    }

    SDLib::File binFile = SD.open(binPath, SD_FILE_WRITE);

    if(!txtFile){
#ifdef DEBUG
        v.vpl(1, "The txt file failed to be opened.");
#endif
        http.reset();
        return false;
    }
    if(!binFile){
#ifdef DEBUG
        v.vpl(1, "The bin file failed to be opened.");
#endif
        http.reset();
        return false;
    }

#ifdef DEBUG
    v.vpl(1, "Conversion phase");
#endif

    Conveter.convert_text2binSD(txtFile, binFile);

#ifdef DEBUG
    v.vpl(1, "Conversion Ended.");
#endif

    binFile.close();
    txtFile.close();

    return false;
}