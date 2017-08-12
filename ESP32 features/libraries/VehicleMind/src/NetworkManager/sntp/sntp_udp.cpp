#include "sntp_udp.h"

void SNTPUDP::constructRequest(uint8_t o_packet[48])
{

    //set all bits to 0;
    memset(o_packet, 0, 48);

    // LI = 3, Version = 4, Mode = 3 (client)
    o_packet[0] = 0b11100011;

    //Stratum set to 0

    //Poll set to 6
    o_packet[2] = 6;

    //Precision set to 18
    o_packet[3] = 0xEC;

    //Everything else set to 0.
}

void SNTPUDP::setDeviceTime(uint8_t o_packet[48])
{

    m_time_set = false;

    //According to the Time protocol in RFC 868 the time between 1900 and 1970 in seconds is it is 2208988800L.
    uint64_t sec_since_70 = (uint64_t)2208988800UL << 32;

    //extract appropriate times
    m_cal_times.receive = protocolTimeInt64(o_packet + 32) - sec_since_70;
    m_cal_times.transmit = protocolTimeInt64(o_packet + 40) - sec_since_70;

    //to calculate time, just
    setDestination();

    //How much time did it take from device send to device recieve
    uint64_t device_period = m_cal_times.destination - m_cal_times.originate;
   
    //How much time did it take from server receieve to server respond
    uint64_t server_period = m_cal_times.transmit - m_cal_times.receive;
  
    //Get the the total time intransit between device and server ( halved because we need only 1 directions time )
    uint64_t transit_time = (device_period - server_period) / 2;

    Serial.print("Current Time: ");
    //Work out the current time (Almost)
    uint64_t current_time = m_cal_times.transmit + transit_time;

    //Add the amount of time it took to do these calculations
    uint64_t old_dest = m_cal_times.destination;
    setDestination();
    current_time += m_cal_times.destination - old_dest;

    //First brign the current time down one order of magnitude (>> 1) to fit in unsigned space
    //After this manipulation SNTP represents the fractional part in 1/2147483648th of a second
    //ESP 32 needs 1/1000000 of a second (micro second)
    //2147483648/1000000 = 2147(rounded for int calculations)
    uint32_t usecs = ((uint32_t)(current_time >> 1)) / 2147;

    uint32_t secs = (uint32_t)(current_time >> 32);

    struct timeval tv = {.tv_sec = secs, .tv_usec = usecs};

    settimeofday(&tv, NULL);

    printTime();

    //Midnight Dec 1st 2017 in unix epoch is 512000000
    if (tv.tv_sec < 512000000)
        return;

    m_time_set = true;
}

uint64_t SNTPUDP::deviceTimeInt64(time_t p_secs, suseconds_t p_usec)
{

    //The secs should only be 32 bits, move the lower values up
    uint64_t secs = ((uint64_t)p_secs) << 32;

    //The u secs need there most significant 4 bytes deleting. Moving 32 bits left then right will do this
    uint64_t usec = (((uint64_t)p_usec) << 32) >> 32;

    return secs + usec;
}

uint64_t SNTPUDP::protocolTimeInt64(uint8_t *p_timestamp)
{

    //protocol time if big endian, need to return little endian uint64_t
    uint8_t temp_buffer[8];

    for (int i = 0; i < 8; i++)
    {
        temp_buffer[7 - i] = p_timestamp[i];
    }

    return *((uint64_t *)&temp_buffer);
}

void SNTPUDP::setOriginate()
{
    gettimeofday(&tim, &tzone);
    m_cal_times.originate = deviceTimeInt64(tim.tv_sec, tim.tv_usec);
}

void SNTPUDP::setDestination()
{
    gettimeofday(&tim, &tzone);
    m_cal_times.destination = deviceTimeInt64(tim.tv_sec, tim.tv_usec);
}

uint64_t SNTPUDP::getTimestamp()
{
    struct timeval now;
    gettimeofday(&now, NULL);

    return (uint64_t)now.tv_usec + ((uint64_t)now.tv_sec << 32);
}

void SNTPUDP::printTimestamp(uint64_t p_timestamp)
{
    Serial.print((int)(p_timestamp >> 32));
    Serial.print(" ");
    Serial.print((int)(p_timestamp));
}

void SNTPUDP::printInt8Array(uint8_t *p_array, unsigned int p_length)
{

    for (int i = 0; i < p_length; i++)
    {
        Serial.print(p_array[i], HEX);
        Serial.print(" ");
    }

    Serial.println("");
}

void SNTPUDP::printTime()
{
    time_t now;
    struct tm timeinfo;
    time(&now);

    setenv("TZ", "EST5EDT", 1);
    tzset();

    localtime_r(&now, &timeinfo);

    char buffer[40];
    strftime(buffer, 40, "%a %b %d %G, %I:%M:%S %p", &timeinfo);

    Serial.println(buffer);
}