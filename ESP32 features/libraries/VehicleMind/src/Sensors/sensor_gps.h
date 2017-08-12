//---------------------------------------------------
//INCLUDE GUARD
#ifndef SENSOR_GPS_H_INCLUDED
#define SENSOR_GPS_H_INCLUDED

//---------------------------------------------------
//DEPS
#include "FreematicsONE.h"
#include "../Tools/binary_packager.h"
#include "../NetworkManager/sntp/sntp_udp.h"
#include "../Tools/data_transmission.h"

#include <string.h>
#include <time.h>
#include <sys/time.h>

//Needed to start the gps decode timer
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

//---------------------------------------------------
//HEADER

/**************************************
* GPS
**************************************/
#ifndef ENABLE_GPS
// change the following line to enable (1)/disable (0) GPS
#define ENABLE_GPS 1
#endif
#define GPS_SERIAL_BAUDRATE 115200L


class SensorGPS : public COBDSPI {

public:
    bool initialize();
    bool readSingle(uint8_t pid, Sample* p_gps_buffer);
    bool readGroup(Sample* p_gps_buffer);

    const static int GPS_NUMBER_OF_PIDS = 6;             //  Number of GPS PID to fetch

private:
    static void gpsDecodeTask(TimerHandle_t p_timer);

    TimerHandle_t m_gps_decode_timer;

    const uint8_t m_pids[GPS_NUMBER_OF_PIDS] = {        // {Grouped PIDs}
        PID_GPS_LATITUDE,                   
        PID_GPS_LONGITUDE,                  
        PID_GPS_ALTITUDE,                   
        PID_GPS_SPEED,
        PID_GPS_HEADING,
        PID_GPS_SAT_COUNT
        // PID_GPS_TIME,
        // PID_GPS_DATE, 
    };
};



#endif
