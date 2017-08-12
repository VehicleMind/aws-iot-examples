#include <WiFi.h>
#include "src/HttpOTA/HttpOTA.h"
#include "src/FreematicsPlus/FreematicsSD.h"
#include "src/SIM5360/SIM5360.h"
#include "src/CellHTTPClient/CellHTTPClient.h"

SDClass SD;
SDLib::File sdfile;
SIM5360 net;
CellHTTPClient http;

byte errors = 0;

#define HTTP_SERVER_URL "awsm.dvlprz.com"
#define HTTP_SERVER_PORT 80
const char* ssid = "Impavidus";
const char* password = "fearless";

// #define FILE_READ SD_O_READ
// #define FILE_WRITE (SD_O_READ | SD_O_WRITE | SD_O_CREAT)

void saveData(uint8_t *buffer, int bytes){
  sdfile.write(buffer, bytes);
}
int readData(uint8_t *buffer, int bytes){
  return sdfile.read(buffer, bytes);
}
void progress(DlState state, int percent){
  Serial.printf("state = %d - percent = %d\n", state, percent);
}
void error(char *message){
  printf("%s\n", message);
}

void startDl(void){
  //write bin file to sdcard
  SD.remove("fw.bin");
  sdfile = SD.open("fw.bin", SD_FILE_WRITE);
}
void endDl(void){
  sdfile.close();
}
void startFl(void){
  //write bin file to sdcard
  sdfile = SD.open("fw.bin", SD_FILE_READ);
}
void endFl(void){
  sdfile.close();
}

void setup() {

  delay(5000);
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Serial.print("SD ");
  uint16_t volsize = SD.initSD();
  if (volsize) {
    Serial.print(volsize);
    Serial.println("MB");
  } else {
  Serial.println("NO");
  }

  // this will init SPI communication
  // initialize SIM5360 xBee module (if present)
  Serial.print("Init SIM5360...");

  if (net.init())
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("Sim NO");
    for (;;)
      ;
  }

  Serial.print("Connecting network");
  if (net.setup())
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("Connection NO");
    for (;;)
      ;
  }

  Serial.print("Operator:");
  Serial.println(net.getOperatorName());

  Serial.print("Obtaining IP address...");
  Serial.println(net.getIP());

  int signal = net.getSignal();
  if (signal > 0)
  {
    Serial.print("CSQ:");
    Serial.print((float)signal / 10, 1);
    Serial.println("dB");
  }

  Serial.print("Init HTTP...");
  if (http.begin("https://awsm.dvlprz.com/firmware.bin"))
  {
    Serial.println("HTTP OK");
  }
  else
  {
    Serial.println("HTTP Error connecting");
    Serial.println(net.buffer);
    errors++;
    return;
  }

  if (errors > 0)
  {
    http.end();
    if (errors > 3)
    {
      // re-initialize 3G module
      setup();
      errors = 0;
    }
  }

// send HTTP request
  Serial.print("Sending HTTP request...");
  if (!http.GET())
  {
    Serial.println("HTTP failed send request");
    http.end();
    errors++;
    return;
  }
  else
  {
    Serial.println("HTTP send and recv OK");
  }

  int recdBytes = http.getSize();
  if (recdBytes)
  {
    Serial.print(recdBytes); Serial.println(" Bytes");
    errors = 0;
  }
  else
  {
    Serial.println("HTTP receive failed");
    errors++;
  }

  char *stream = http.getStreamPtr();
  Serial.println(stream);

  char *contenttype = http.getContentType();
  Serial.print(contenttype);

  

  // Serial.println("recv once more ...");
  // char *payload;
  // if (recdBytes = http.httpReceive(&payload))
  // {
  //   Serial.print(recdBytes); Serial.println(" Bytes");
  //   Serial.println("-----HTTP RESPONSE-----");
  //   Serial.println(payload);
  //   Serial.println("-----------------------");
  //   errors = 0;
  // }
  // else
  // {
  //   Serial.println("HTTP receive failed");
  //   errors++;
  // }

  // Serial.print("Connecting to ");
  // Serial.print(ssid);
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("");
  // DlInfo info;
  // info.url = "http://192.168.10.18/upload/firmware.bin";
  // info.md5 = "2a797c818c87b354cbce7724c72e202d";
  // info.startDownloadCallback =  startDl;
  // info.endDownloadCallback =    endDl;
  // info.startFlashingCallback =  startFl;
  // info.endFlashingCallback =    endFl;

  // info.saveDataCallback = saveData;
  // info.readDataCallback = readData;
  // info.progressCallback  = progress;
  // info.errorCallback     = error;

  // httpOTA.start(info);

}

void loop () {}
