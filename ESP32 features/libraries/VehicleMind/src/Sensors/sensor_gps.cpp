#include "sensor_gps.h"

bool SensorGPS::initialize(){
    begin();

    return gpsInit(GPS_SERIAL_BAUDRATE);
}

bool SensorGPS::readSingle(uint8_t pid, Sample* p_gps_buffer){
    static uint16_t lastUTC = 0;
    static uint8_t lastGPSDay = 0;

    GPS_DATA gd = {0};
    // read parsed GPS data
    if (!gpsGetData(&gd))
        return false;
    
    if (gd.date && lastUTC != (uint16_t)gd.time)
    {
        byte day = gd.date / 10000;
        if (lastGPSDay != day)
            lastGPSDay = day;
        lastUTC = (uint16_t)gd.time;
#ifdef DEBUG
        char buf[32];
        sprintf(buf, "UTC:%08lu SAT:%u", gd.time, (unsigned int)gd.sat);

        Serial.println(buf);
#endif
    }

    uint8_t tempVal[4];
    int index = 0;

    p_gps_buffer->makeHeader(PID_GPS, 1);
    index = 0;

    switch(pid){
        case PID_GPS_LATITUDE:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.lat), sizeof(gd.lat));
            break;
            
        case PID_GPS_LONGITUDE:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.lng), sizeof(gd.lng));
            break;
            
        case PID_GPS_ALTITUDE:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.alt), sizeof(gd.alt));                
            break;
            
        case PID_GPS_SPEED:
            *(tempVal + index++) = gd.speed;
            break;
            
        case PID_GPS_HEADING:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.heading), sizeof(gd.heading));
            break;
            
        case PID_GPS_SAT_COUNT:
            *(tempVal + index++) = gd.sat;
            break;
            
        case PID_GPS_TIME:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.time), sizeof(gd.time));
            break;

        case PID_GPS_DATE:
            index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.date), sizeof(gd.date));
            break;

        default:
            // tempVal = 0;
            break;
    }

    p_gps_buffer->addPID(pid, tempVal);
    return true;
}

bool SensorGPS::readGroup(Sample* p_gps_buffer){
    static uint16_t lastUTC = 0;
    static uint8_t lastGPSDay = 0;

    GPS_DATA gd = {0};
    // read parsed GPS data
    if (!gpsGetData(&gd))
        return false;
    
    if (gd.date && lastUTC != (uint16_t)gd.time)
    {
        byte day = gd.date / 10000;
        if (lastGPSDay != day)
            lastGPSDay = day;
        lastUTC = (uint16_t)gd.time;
#ifdef DEBUG
        char buf[32];
        sprintf(buf, "UTC:%08lu SAT:%u", gd.time, (unsigned int)gd.sat);

        Serial.println(buf);
#endif
    }

    uint8_t tempVal[4];
    int index = 0;

    p_gps_buffer->makeHeader(PID_GPS, GPS_NUMBER_OF_PIDS);
    for(int i = 0; i < GPS_NUMBER_OF_PIDS; i++){
        index = 0;

        switch(m_pids[i]){
            case PID_GPS_LATITUDE:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.lat), sizeof(gd.lat));
                break;
            
            case PID_GPS_LONGITUDE:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.lng), sizeof(gd.lng));
                break;
            
            case PID_GPS_ALTITUDE:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.alt), sizeof(gd.alt));
                break;
            
            case PID_GPS_SPEED:
                *(tempVal + index++) = gd.speed;
                break;
            
            case PID_GPS_HEADING:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.heading), sizeof(gd.heading));
                break;
            
            case PID_GPS_SAT_COUNT:
                *(tempVal + index++) = gd.sat;
                break;
            
            case PID_GPS_TIME:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.time), sizeof(gd.time));
                break;

            case PID_GPS_DATE:
                index += BinaryPackager::addBytesAsInt8(tempVal + index, &(gd.date), sizeof(gd.date));
                break;

            default:
                // tempVal = 0;
                break;
        }
        if(!p_gps_buffer->addPID(m_pids[i], tempVal)) return false;
    }
    return true;
}

void SensorGPS::gpsDecodeTask(TimerHandle_t p_timer){
    gps_decode_task(0);
}