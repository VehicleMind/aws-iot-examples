//---------------------------------------------------
//INCLUDE GUARD
#ifndef DATA_TRANSMISSION_H_INCLUDED
#define DATA_TRANSMISSION_H_INCLUDED

//---------------------------------------------------
//DEPS
#include "FreematicsONE.h"
#include "./binary_packager.h"
#include "../NetworkManager/sntp/sntp_udp.h"

#include <stdint.h>

//---------------------------------------------------
//HEADER
#define NUMBER_OF_PIDS 84

typedef struct{
    uint16_t pid;  // 0x0030 ---> mode: 0x00 , pid: 0x30
    uint8_t type;
    uint8_t length;
    uint8_t* value;
} pidNode_t; // Size: 2 + 1 + 1 + length = 3 + length

enum type_t {tUINT64, tINT64, tUINT32, tINT32, tUINT16, tINT16, tUINT8, tINT8, tDOUBLE, tFLOAT, tSTRING, tVOID};


class Sample{
    public:
        Sample();
        ~Sample();

        void print();
        int encodeBin(uint8_t *o_buffer);
        void makeHeader(const uint8_t pidClass, const uint8_t pidNumb);
        bool addPID(const uint8_t pid, const uint8_t* value, const uint8_t mode = 0);
        void releasePIDs(); // Should always be called at the end of a Sample lifespan

        uint64_t timestamp;
        uint8_t pidClass;
        uint8_t pidNumb;
        pidNode_t* pids;

    private:
        int pidsIndex = 0;
};

void dtcDecoder(uint16_t dtc, char* result);
int64_t byte2int(pidNode_t node);
bool pidGetInf(const uint8_t pid, uint8_t* pidInf, const uint8_t mode = 0);

const uint8_t pidList[NUMBER_OF_PIDS][4] = {    // {pid, mode, type, len}

    {PID_OBD, PID_MODE_0, tVOID, 0},
    {PID_MODE_SUPP0, PID_MODE_1, tUINT32, 4},
    {PID_MODE_SUPP2, PID_MODE_1, tUINT32, 4},
    {PID_MODE_SUPP4, PID_MODE_1, tUINT32, 4},
    {PID_MODE_SUPP6, PID_MODE_1, tUINT32, 4},
    {PID_MODE_SUPP8, PID_MODE_1, tUINT32, 4},
    {PID_MODE_SUPPA, PID_MODE_1, tUINT32, 4},
    {PID_MODE_SUPPC, PID_MODE_1, tUINT32, 4},
    {PID_MIL_STATUS, PID_MODE_1, tUINT16, 2},                          // Encoded
    {PID_FREEZE_DTC, PID_MODE_1, tUINT16, 2},
    {PID_FUEL_SYS_STATUS, PID_MODE_1, tUINT16, 2},
    {PID_ENGINE_LOAD, PID_MODE_1, tUINT8, 1},         
    {PID_COOLANT_TEMP, PID_MODE_1, tINT8, 1},                   
    {PID_SHORT_TERM_FUEL_TRIM_1, PID_MODE_1, tINT8, 1},      // pid returns 1 byte but at the time of decoding, it is devided by 1.28 and subtracted by 100
    {PID_LONG_TERM_FUEL_TRIM_1, PID_MODE_1, tINT8, 1},       // pid returns 1 byte but at the time of decoding, it is devided by 1.28 and subtracted by 100
    {PID_SHORT_TERM_FUEL_TRIM_2, PID_MODE_1, tINT8, 1},      // pid returns 1 byte but at the time of decoding, it is devided by 1.28 and subtracted by 100
    {PID_LONG_TERM_FUEL_TRIM_2, PID_MODE_1, tINT8, 1},       // pid returns 1 byte but at the time of decoding, it is devided by 1.28 and subtracted by 100
    {PID_FUEL_PRESSURE, PID_MODE_1, tUINT16, 2},             // pid returns 1 byte but at the time of decoding, it is multiplied by 3 
    {PID_INTAKE_MAP, PID_MODE_1, tUINT8, 1},
    {PID_RPM, PID_MODE_1, tUINT16, 2},                            
    {PID_SPEED, PID_MODE_1, tUINT8, 1},         
    {PID_TIMING_ADVANCE, PID_MODE_1, tINT8, 1},              // pid returns 1 byte but at the time of decoding, it is devided by 2 and subtracted by 64
    {PID_INTAKE_TEMP, PID_MODE_1, tINT8, 1},
    {PID_MAF_FLOW, PID_MODE_1, tUINT16, 2},                  // pid returns 2 bytes but at the time of decoding, it is devided by 100                  
    {PID_THROTTLE, PID_MODE_1, tUINT8, 1},  
    {PID_AUX_INPUT, PID_MODE_1, tUINT8, 1},
    {PID_RUNTIME, PID_MODE_1, tUINT16, 2}, 
    {PID_DISTANCE_WITH_MIL, PID_MODE_1, tUINT16, 2}, 
    {PID_COMMANDED_EGR, PID_MODE_1, tUINT8, 1},  
    {PID_EGR_ERROR, PID_MODE_1, tINT8, 1},                   // pid returns 1 byte but at the time of decoding, it is devided by 1.28 and subtracted by 100
    {PID_COMMANDED_EVAPORATIVE_PURGE, PID_MODE_1, tUINT8, 1},  
    {PID_FUEL_LEVEL, PID_MODE_1, tUINT8, 1},
    {PID_WARMS_UPS, PID_MODE_1, tUINT8, 1},                     
    {PID_DISTANCE_DTC_CLEARED, PID_MODE_1, tUINT16, 2}, 
    {PID_EVAP_SYS_VAPOR_PRESSURE, PID_MODE_1, tUINT16, 2},
    {PID_BAROMETRIC, PID_MODE_1, tUINT8, 1},
    {PID_CATALYST_TEMP_B1S1, PID_MODE_1, tINT16, 2},         // pid returns 1 byte but at the time of decoding, it is devided by 10 and subtracted by 40
    {PID_CATALYST_TEMP_B2S1, PID_MODE_1, tINT16, 2},         // pid returns 1 byte but at the time of decoding, it is devided by 10 and subtracted by 40
    {PID_CATALYST_TEMP_B1S2, PID_MODE_1, tINT16, 2},         // pid returns 1 byte but at the time of decoding, it is devided by 10 and subtracted by 40
    {PID_CATALYST_TEMP_B2S2, PID_MODE_1, tINT16, 2},         // pid returns 1 byte but at the time of decoding, it is devided by 10 and subtracted by 40
    {PID_CONTROL_MODULE_VOLTAGE, PID_MODE_1, tUINT16, 2},    
    {PID_ABSOLUTE_ENGINE_LOAD, PID_MODE_1, tUINT16, 2},     
    {PID_AIR_FUEL_EQUIV_RATIO, PID_MODE_1, tUINT8, 1},       // pid returns 2 bytes but at the time of decoding, it is devided by 2/65536 
    {PID_RELATIVE_THROTTLE_POS, PID_MODE_1, tUINT8, 1},  
    {PID_AMBIENT_TEMP, PID_MODE_1, tINT8, 1},                                           
    {PID_ABSOLUTE_THROTTLE_POS_B, PID_MODE_1, tUINT8, 1},  
    {PID_ABSOLUTE_THROTTLE_POS_C, PID_MODE_1, tUINT8, 1},  
    {PID_ACC_PEDAL_POS_D, PID_MODE_1, tUINT8, 1},  
    {PID_ACC_PEDAL_POS_E, PID_MODE_1, tUINT8, 1},  
    {PID_ACC_PEDAL_POS_F, PID_MODE_1, tUINT8, 1},  
    {PID_COMMANDED_THROTTLE_ACTUATOR, PID_MODE_1, tUINT8, 1},  
    {PID_TIME_WITH_MIL, PID_MODE_1, tUINT16, 2},
    {PID_TIME_SINCE_CODES_CLEARED, PID_MODE_1, tUINT16, 2},
    {PID_FUEL_TYPE, PID_MODE_1, tUINT8, 1},  
    {PID_ETHANOL_FUEL, PID_MODE_1, tUINT8, 1},  
    {PID_FUEL_RAIL_PRESSURE, PID_MODE_1, tUINT16, 2},        // pid returns 2 bytes but at the time of decoding, it is multiplied by 10
    {PID_HYBRID_BATTERY_PERCENTAGE, PID_MODE_1, tUINT8, 1},  
    {PID_ENGINE_OIL_TEMP, PID_MODE_1, tINT8, 1},  
    {PID_FUEL_INJECTION_TIMING, PID_MODE_1, tINT16, 2},      // pid returns 1 byte but at the time of decoding, it is devided by 128 and subtracted by 210
    {PID_ENGINE_FUEL_RATE, PID_MODE_1, tUINT16, 2},      
    {PID_EMS_REQ, PID_MODE_1, tUINT8, 1},
    {PID_ENGINE_TORQUE_DEMANDED, PID_MODE_1, tINT8, 1},          
    {PID_ENGINE_TORQUE_PERCENTAGE, PID_MODE_1, tINT8, 1},
    {PID_ENGINE_REF_TORQUE, PID_MODE_1, tUINT16, 2},       
    {PID_MODE_SUPP0, PID_MODE_9, tUINT32, 4},
    {PID_VIN_MSG_CNT, PID_MODE_9, tUINT8, 1},
    {PID_VIN_NUMB, PID_MODE_9, tSTRING, 17},
    {PID_GPS, PID_MODE_0, tVOID, 0},                   
    {PID_GPS_LATITUDE, PID_MODE_0, tINT32, 4},                   
    {PID_GPS_LONGITUDE, PID_MODE_0, tINT32, 4},                  
    {PID_GPS_ALTITUDE, PID_MODE_0, tINT16, 2},                   
    {PID_GPS_SPEED, PID_MODE_0, tUINT8, 1},                      
    {PID_GPS_HEADING, PID_MODE_0, tINT16, 2},                    
    {PID_GPS_SAT_COUNT, PID_MODE_0, tUINT8, 1},                   
    {PID_GPS_TIME, PID_MODE_0, tUINT32, 4},                    
    {PID_GPS_DATE, PID_MODE_0, tUINT32, 4}, 
    {PID_MEMS, PID_MODE_0, tVOID, 0},
    {PID_MEMS_ACC, PID_MODE_0, tFLOAT, 12},                    
    {PID_MEMS_GYRO, PID_MODE_0, tFLOAT, 12},                     
    {PID_MEMS_COMPASS, PID_MODE_0, tFLOAT, 12},                  
    {PID_MEMS_TEMP, PID_MODE_0, tINT16, 2},                      
    {PID_MEMS_BATTERY_VOLTAGE, PID_MODE_0, tUINT8, 1},         
    {PID_MEMS_ORIENTATION_EULER, PID_MODE_0, tFLOAT, 12},                              
    {PID_MEMS_ORIENTATION_QUATERNION, PID_MODE_0, tFLOAT, 12}

};

#endif