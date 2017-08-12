#include "sensor_mems.h"

bool SensorMEMS::initialize(){
    bool suc = false;

#if MEMS_MODE == MEMS_ACC
    suc = mems.begin(ENABLE_ORIENTATION);
#elif MEMS_MODE == MEMS_9DOF
    suc = mems.begin(ENABLE_ORIENTATION);
#elif MEMS_MODE == MEMS_DMP
    suc = mems.begin(ENABLE_ORIENTATION, 10);
#endif
    
    if(suc) this->calibrateMEMS();

    return suc;
}

void SensorMEMS::calibrateMEMS(){
    accBias[0] = 0;
    accBias[1] = 0;
    accBias[2] = 0;
    int n;
    for (n = 0; n < 100; n++) {
      float acc[3] = {0};
      mems.read(acc);
      accBias[0] += acc[0];
      accBias[1] += acc[1];
      accBias[2] += acc[2];
      delay(10);
    }
    accBias[0] /= n;
    accBias[1] /= n;
    accBias[2] /= n;
    Serial.print("ACC Bias:");
    Serial.print(accBias[0]);
    Serial.print('/');
    Serial.print(accBias[1]);
    Serial.print('/');
    Serial.println(accBias[2]);
}

int16_t SensorMEMS::readTemp(){
    int16_t temp;
    
    bool suc = mems.read(0, 0, 0, &temp, 0);
    if(!suc) return -1;

    return temp;
}

bool SensorMEMS::readSingle(uint8_t pid, Sample* p_sensor_buffer){
    int16_t temp;
    uint8_t batt_volt;
    float acc[3] = {0};
    float gyr[3] = {0};
    float mag[3] = {0};
    float temp_ori[3] = {0};
    ORIENTATION ori;

    bool suc = mems.read(acc, gyr, mag, &temp, &ori);
    if (!suc) return suc;

// ##############
    for(int i = 0; i < 3; i++)
        acc[i] -= accBias[i]; 

    temp_ori[0] = ori.pitch;
    temp_ori[1] = ori.roll;
    temp_ori[2] = ori.yaw;

    uint8_t tempVal[16];
    int index = 0;

    p_sensor_buffer->makeHeader(PID_MEMS, 1);
    index = 0;
        
    switch(pid){
        case PID_MEMS_ACC:
            for(int i = 0; i < 3; i++)
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(acc[i]), sizeof(acc[i])); 
            break;
              
        case PID_MEMS_GYRO:
            for(int i = 0; i < 3; i++)
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gyr[i]), sizeof(gyr[i])); 
            break;
     
        case PID_MEMS_COMPASS:
            for(int i = 0; i < 3; i++)
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(mag[i]), sizeof(mag[i])); 
            break;
     
        case PID_MEMS_TEMP:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(temp), sizeof(temp));  
            break;
    
        case PID_MEMS_BATTERY_VOLTAGE:
            *(tempVal + index++) = batt_volt;
            break;
    
#if MEMS_MODE == MEMS_DMP
        case PID_MEMS_ORIENTATION_EULER:
            for(int i = 0; i < 3; i++)
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(temp_ori[i]), sizeof(temp_ori[i])); 
            break;
#else    
        case PID_MEMS_ORIENTATION_QUATERNION:
            for(int i = 0; i < 3; i++)
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(temp_ori[i]), sizeof(temp_ori[i]));
            break;
#endif
                
        default:
            // for(int k = 0; k < 3; k++)
            //     tempVal[k].fNumber = 0; 
            break;
    }

    p_sensor_buffer->addPID(pid, tempVal);

    return suc;
}

bool SensorMEMS::readGroup(Sample* p_sensor_buffer){
    int16_t temp;
    uint8_t batt_volt;
    float acc[3];
    float gyr[3];
    float mag[3];
    float temp_ori[3];
    ORIENTATION ori;

    bool suc = mems.read(acc, gyr, mag, &temp, &ori);
    if (!suc) return suc;

// ##############
    for(int i = 0; i < 3; i++)
        acc[i] -= accBias[i];

    temp_ori[0] = ori.pitch;
    temp_ori[1] = ori.roll;
    temp_ori[2] = ori.yaw;

    uint8_t tempVal[16];
    int index = 0;

    p_sensor_buffer->makeHeader(PID_MEMS, MEMS_NUMBER_OF_PIDS);
    for(int i = 0; i < MEMS_NUMBER_OF_PIDS; i++){
        index = 0;
        
        switch(m_pids[i]){
            case PID_MEMS_ACC:
                for(int i = 0; i < 3; i++)
                    index += BinaryPackager::addBytesAsInt8(tempVal + index, &(acc[i]), sizeof(acc[i])); 
                break;
                
            case PID_MEMS_GYRO:
                for(int i = 0; i < 3; i++)
                    index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gyr[i]), sizeof(gyr[i])); 
                break;
     
            case PID_MEMS_COMPASS:
                for(int i = 0; i < 3; i++)
                    index += BinaryPackager::addBytesAsInt8(tempVal + index, &(mag[i]), sizeof(mag[i])); 
                break;
     
            case PID_MEMS_TEMP:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(temp), sizeof(temp));  
                break;
    
            case PID_MEMS_BATTERY_VOLTAGE:
                *(tempVal + index++) = batt_volt;
                break;

#if MEMS_MODE == MEMS_DMP
            case PID_MEMS_ORIENTATION_EULER:
                for(int i = 0; i < 3; i++)
                    index += BinaryPackager::addBytesAsInt8(tempVal + index, &(temp_ori[i]), sizeof(temp_ori[i])); 
                break;
#else    
            case PID_MEMS_ORIENTATION_QUATERNION:
                for(int i = 0; i < 3; i++)
                    index += BinaryPackager::addBytesAsInt8(tempVal + index, &(temp_ori[i]), sizeof(temp_ori[i]));
                break;
#endif
                
            default:
                // for(int k = 0; k < 3; k++)
                //     tempVal[k].fNumber = 0; 
                break;
        }
        if(!p_sensor_buffer->addPID(m_pids[i], tempVal)) return false;
    }
    return suc;
}