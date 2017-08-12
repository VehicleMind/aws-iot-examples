//---------------------------------------------------
//INCLUDE GUARD
#ifndef SNTP_UDP_H_INCLUDED
#define SNTP_UDP_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPS
#include <stdint.h>
#include <cstring>
#include <HardwareSerial.h>
#include <time.h>
#include <sys/time.h>

//---------------------------------------------------
//HEADER
// #define DEBUG

typedef struct
{
    uint64_t originate;   // Client sends
    uint64_t receive;     // Server receives
    uint64_t transmit;    // Server Sends
    uint64_t destination; // Client Receives
} sntp_times_t;

class SNTPUDP
{

  public:
    void constructRequest(uint8_t o_packet[48]); //outputs the SNTP request
    void setDeviceTime(uint8_t o_packet[48]);    //uses the returned packet to set the time

    void setOriginate();
    void setDestination();

    uint64_t deviceTimeInt64(time_t p_secs, suseconds_t p_usec);
    uint64_t protocolTimeInt64(uint8_t *p_timestamp);

    static uint64_t getTimestamp();
    static void printTimestamp(uint64_t p_timestamp);

    void printInt8Array(uint8_t *p_array, unsigned int p_length);
    void printTime();

    bool isTimeSet() { return m_time_set; }
    void timeReset() { m_time_set = false; }

  private:
    sntp_times_t m_cal_times;
    timeval tim;
    timezone tzone = {0, 0}; //Can set the timezone if you want it auto translated

    bool m_time_set = false;
};

#endif