//---------------------------------------------------
//INCLUDE DEPS
// Serial Communication
#include <HardwareSerial.h>
// Dependent libs
#include "VehicleMind.h"
MPU9250_9DOF mems;
int16_t temp;
float acc[3] = {0};
float gyr[3] = {0};
float mag[3] = {0};
ORIENTATION ori;
void setup(){
  delay(3000);
  Serial.begin(115200);
  Serial.println("Entering Setup");
  if(mems.begin(1)){
    Serial.println("MEMS INIT SUCCEEDED");
  }
  else{
    Serial.println("MEMS INIT FAILED");
    ESP.restart();
  }
}
void loop(){
  //Serial.println("### MEMS Data ###");
  if(mems.read(acc, gyr, mag, &temp, &ori)){
    //Serial.print("Acceleraion 0: ");
    Serial.print(acc[0]); Serial.print("\t");
    // Serial.println(" g");
    // Serial.print("Acceleraion 1: ");
    Serial.print(acc[1]); Serial.print("\t");
    // Serial.println(" g");
    // Serial.print("Acceleraion 2: ");
    Serial.print(acc[2]); Serial.print("\t");
    // Serial.println(" g");
    // Serial.print("Gyro 0: ");
    Serial.print(gyr[0]); Serial.print("\t");
    // Serial.println(" dps");
    // Serial.print("Gyro 1: ");
    Serial.print(gyr[1]); Serial.print("\t");
    // Serial.println(" dps");
    // Serial.print("Gyro 2: ");
    Serial.print(gyr[2]); Serial.print("\t");
    // Serial.println(" dps");
    // Serial.print("Mag 0: ");
    Serial.print(mag[0]); Serial.print("\t");
    // Serial.println(" mT");
    // Serial.print("Mag 1: ");
    Serial.print(mag[1]); Serial.print("\t");
    // Serial.println(" mT");
    // Serial.print("Mag 2: ");
    Serial.print(mag[2]); Serial.print("\n");
    // Serial.println(" mT");
    // Serial.print("Roll: ");
    // Serial.println(ori.roll);
    // Serial.print("Pitch: ");
    // Serial.println(ori.pitch);
    // Serial.print("Yaw: ");
    // Serial.println(ori.yaw);
    // Serial.print("Temperature: ");
    // Serial.print(temp);
    // Serial.println(" degree of Celsius");
  }
  delay(200);
}
