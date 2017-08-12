#ifndef DeviceID
#define DeviceID

#include "../NetworkManager/tcp/tcp_sim5360.h"
#include "./Tools/Verbose.h"

using namespace std;

class ChipID{
  public:
    ChipID() : m_tcp_connector(6000), v(false, "CID   ") {}
    void getChipID(char o_buffer[28]);
    void getSimIMEI(char o_buffer[16]);
    void getMACAddress(char o_buffer[13]);

  private:
    Verbose v;
    TCPSIM5360 m_tcp_connector;
};

#endif
