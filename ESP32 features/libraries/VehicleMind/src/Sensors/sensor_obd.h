//---------------------------------------------------
//INCLUDE GUARD
#ifndef SENSOR_OBD_H_INCLUDED
#define SENSOR_OBD_H_INCLUDED



//---------------------------------------------------
//DEPS
#include "FreematicsONE.h"
#include "../Tools/binary_packager.h"
#include "../NetworkManager/sntp/sntp_udp.h"
#include "../Tools/data_transmission.h"

//---------------------------------------------------
//HEADER
/**************************************
* OBD-II configurations
**************************************/
#ifndef ENABLE_OBD
#define ENABLE_OBD 1
#endif
// maximum consecutive OBD-II access errors before entering standby
// small values cause not having response for short time to be detected as ignition off
#define MAX_OBD_ERRORS 5 
// maximum allowed time for re-establishing OBD connection
#define MAX_OBD_RETRY_TIME 15000 /* ms */

// VIN used when real one unavailable
#ifdef DEVICE_ID
#define DEFAULT_VIN DEVICE_ID
#else
#define DEFAULT_VIN "DEFAULT_VIN"
#endif


class SensorOBD : public COBDSPI {

public:
    bool initialize();
    bool read(const uint8_t p_datamode, const uint8_t p_pid, Sample* p_obd_buffer);
    bool readNext(Sample* p_obd_buffer);
    bool readVIN(char* buffer);
    bool checkDTC();
    bool checkErrors();

    const static int OBD_NUMBER_OF_PIDS = 26;               // Number of OBD PID to fetch 
    
private:
    int m_pid_index = 0;

    const uint8_t m_pids[OBD_NUMBER_OF_PIDS][2] = {            // {PID, Data Bytes Returned}
        {PID_FUEL_LEVEL, PID_MODE_1},                     
        {PID_RPM, PID_MODE_1},
        {PID_MAF_FLOW, PID_MODE_1},
        {PID_INTAKE_MAP, PID_MODE_1},
        {PID_INTAKE_TEMP, PID_MODE_1},
        {PID_COOLANT_TEMP, PID_MODE_1},
        {PID_ENGINE_OIL_TEMP, PID_MODE_1},
        {PID_THROTTLE, PID_MODE_1},
        {PID_ENGINE_LOAD, PID_MODE_1},
        {PID_ABSOLUTE_ENGINE_LOAD, PID_MODE_1},
        {PID_SPEED, PID_MODE_1},
        {PID_FUEL_PRESSURE, PID_MODE_1},
        {PID_RELATIVE_THROTTLE_POS, PID_MODE_1},
        {PID_AMBIENT_TEMP, PID_MODE_1},
        {PID_CONTROL_MODULE_VOLTAGE, PID_MODE_1},         
        {PID_BAROMETRIC, PID_MODE_1},
        {PID_RUNTIME, PID_MODE_1},
        {PID_ENGINE_FUEL_RATE, PID_MODE_1},
        {PID_ENGINE_TORQUE_PERCENTAGE, PID_MODE_1},
        {PID_MIL_STATUS, PID_MODE_1},                          // Encoded
        {PID_TIME_WITH_MIL, PID_MODE_1},
        {PID_DISTANCE_WITH_MIL, PID_MODE_1},
        {PID_AIR_FUEL_EQUIV_RATIO, PID_MODE_1},
        {PID_WARMS_UPS, PID_MODE_1},
        {PID_DISTANCE_DTC_CLEARED, PID_MODE_1},            
        {PID_TIME_SINCE_CODES_CLEARED, PID_MODE_1}
    };
};



#endif
