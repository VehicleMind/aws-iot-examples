//---------------------------------------------------
//INCLUDE GUARD
#ifndef TCP_SIM5360_H_INCLUDED
#define TCP_SIM5360_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPS
#include "FreematicsONE.h"
#include "../../Tools/Verbose.h"
#include "./driver/uart.h"

//---------------------------------------------------
//HEADER
// #define DEBUG
#define XBEE_BAUDRATE 115200

#define BEE_UART_PIN_RXD (16)
#define BEE_UART_PIN_TXD (17)
#define BEE_UART_NUM UART_NUM_1

//My Definitions
#define NETWORK_CONNECT_TIMEOUT 15000

#define ROUTER_RETRY 2

#define TCP_SETUP_OPEN_SOCKET_RETRY 4

#define SEND_DATA_RETRIES 3
#define SEND_DATA_CARROT_TIMEOUT 250
#define SEND_DATA_LAST_OK_TIMEOUT 100

#define TCP_SENDREC_BUFFER_SIZE 1400

typedef struct
{
    bool active;
    bool tcp;
} socket_status_t;

class TCPSIM5360 : public COBDSPI
{

  public:
    //CONSTRUCTORS
    TCPSIM5360(int p_connection_timeout);

    //CONNECTION FUNCTIONS
    bool connectNetwork(const char *p_apn, const char *p_cellusr, const char *p_cellpwd); //Runs a full connection cycle

    bool openTCPSocket(const char *p_host, uint16_t p_port, bool p_dns, unsigned int p_socket); //Open a TCP connection to host
    bool openUDPSocket(uint16_t p_local_port, unsigned int p_socket);

    bool closeSocket(unsigned int p_socket);

    //DATA SENDING
    bool sendTCPData(const char *p_data, unsigned int p_len, unsigned int p_socket);                                                  //Sends string of data over the network
    bool sendUDPData(const char *p_data, unsigned int p_len, const char *p_host, uint16_t p_port, bool p_dns, unsigned int p_socket); //Sends string of data over the network

    void printConnections();

    //TCP UDP LAYER
    socket_status_t getSocketInfo(unsigned int p_socket) { return m_sockets[p_socket]; }

    //RECEIEVE DATA
    char *recieveData(int *p_bytes = 0, unsigned int p_timeout = 5000);                                                       //Recieve a packet of data
    bool recDataAvailable(size_t *o_size);                                                                                    //Data in bytes waiting in the UART rx buffer
    void extractRecievedData(void *p_ref, void (*p_use_extraction)(void *p_reference, char *p_extraction, int p_len) = NULL); //

    //HELPER FUNCTIONS
    int networkSignal();                 // get the current signal to the ISP
    char *networkOperator();             // get the operator of your network
    char *TCPGetIP();                    // get our IP from the ISP
    String TCPQueryIP(const char *host); // do a DNS lookup for the IP of a host by domain name
    bool checkConnection();              // check network connection 

    bool routerSetup(); //Setup the router
    bool sendATCommand(const char *cmd, unsigned int timeout = 1000, const char *expected = "\r\nOK\r\n", bool terminated = false);

    char m_buffer[TCP_SENDREC_BUFFER_SIZE]; //Buffer used for sending and receiving data

  protected:
    uint8_t m_router_stage = 0;             //Used during router setup

    const char *m_apn;
    const char *m_cellusr;
    const char *m_cellpwd;

  private:
    Verbose v;

    socket_status_t m_sockets[10];

    int m_repair_stage = 0;
    int m_failed_repair_attempts = 0;

    bool networkSetup(const char *apn, const char *cellusr, const char *cellpwd); //Setup the network connection

    void resetSockets();
};

#endif
