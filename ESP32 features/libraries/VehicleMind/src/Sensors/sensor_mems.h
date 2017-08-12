//---------------------------------------------------
//INCLUDE GUARD
#ifndef SENSOR_MANAGER_H_INCLUDED
#define SENSOR_MANAGER_H_INCLUDED

//---------------------------------------------------
//DEPS
#include "FreematicsONE.h"
#include "../Tools/binary_packager.h"
#include "../NetworkManager/sntp/sntp_udp.h"
#include "../Tools/data_transmission.h"
#include "sensor_config.h"

//---------------------------------------------------
//HEADER

/**************************************
* MEMS sensors
**************************************/
#ifndef MEMS_MODE
#define MEMS_MODE MEMS_DMP
#endif

class SensorMEMS : protected COBDSPI {

public:
    bool initialize();
    bool readSingle(uint8_t pid, Sample* p_sensor_buffer);
    bool readGroup(Sample* p_sensor_buffer);
    int16_t readTemp();

    const static int MEMS_NUMBER_OF_PIDS = 4;                 // Number of MEMS PID to fetch

private:
    void calibrateMEMS();
#if MEMS_MODE == MEMS_9DOF
    MPU9250_9DOF mems;
#elif MEMS_MODE == MEMS_ACC
    MPU9250_ACC mems;
#elif MEMS_MODE == MEMS_DMP
    MPU9250_DMP mems;
#endif

    float accBias[3] = {0};

    const uint8_t m_pids[MEMS_NUMBER_OF_PIDS] = {             // {Grouped PIDs}
        PID_MEMS_ACC,                    
        PID_MEMS_GYRO,                     
        // PID_MEMS_COMPASS,
        PID_MEMS_TEMP,               
        // PID_MEMS_BATTERY_VOLTAGE,
#if MEMS_MODE == MEMS_DMP        
        PID_MEMS_ORIENTATION_EULER
#elif MEMS_MODE == MEMS_9DOF
        PID_MEMS_ORIENTATION_QUATERNION
#endif
    };
};


















#endif
