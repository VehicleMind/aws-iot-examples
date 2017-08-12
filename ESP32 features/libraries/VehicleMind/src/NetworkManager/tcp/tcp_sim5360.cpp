#include "tcp_sim5360.h"

//PUBLIC FUNCTIONS ==================================

//---------------------------------------------------
//CONSTRUCTOR
TCPSIM5360::TCPSIM5360(int p_connection_timeout) : v(true, "TCP") {}

//---------------------------------------------------
//CONNECTION FUNCTIONS

//Runs a full connection cycle
bool TCPSIM5360::connectNetwork(const char *p_apn, const char *p_cellusr, const char *p_cellpwd)
{

  resetSockets();

  //Run modem power on cycle
#ifdef DEBUG
  v.vpl(1, "MODEM START PHASE:");
#endif
  if (routerSetup())
  {
#ifdef DEBUG
    v.vpl(1, "Router Configured");
#endif
  }
  else
  {
#ifdef DEBUG
    v.vpl(1, "Router Configuration failed - @@@@ERROR@@@@");
#endif
    return false;
  };

  //Run Cell network configuration for router
#ifdef DEBUG
  v.vpl(1, "SORCOM CONNECTION PHASE:");
#endif
  if (!networkSetup(p_apn, p_cellusr, p_cellpwd))
  {
#ifdef DEBUG
    v.vpl(1, "Connection to Sorocom failed - @@@@ERROR@@@@");
#endif
    return false;
  };

  //This assures the network is open. If it gets called again, it'll return an error saying network already open. The method will still complete though.
  return sendATCommand("AT+NETOPEN\r", 20000, "+NETOPEN: 0");
}

// Opens a socket to send data over TCP
bool TCPSIM5360::openTCPSocket(const char *p_ip, uint16_t p_port, bool p_dns, unsigned int p_socket)
{
#ifdef DEBUG
  v.vpl(2, "Attmepting to open TCP socket: ");
#endif
  //Check if the socket is already open
  if (m_sockets[p_socket].active)
  {
    return false;
#ifdef DEBUG
    v.vpl(2, "Socket Taken, cannot open");
#endif
  }

  //Test if net is open. If open, return true
  bool netopen = sendATCommand("AT+NETOPEN?\r", 5000, "+NETOPEN: 1");
  if (!netopen)
  {
#ifdef DEBUG
    v.vpl(2, "NET is not open. Net must be opened to create sockets");
#endif
    return false;
  }

  //If Required, attmept a DNS lookut
  char ip_address[16];
  if (p_dns)
  {
#ifdef DEBUG
    v.vp(2, "--DNS Lookup (Get Destination IP): ");
#endif

    String ip = TCPQueryIP(p_ip);
    if (ip.length())
    {
      strncpy(ip_address, ip.c_str(), sizeof(ip_address));
    }
    else
    {
#ifdef DEBUG
      v.p("FAILED");
      v.l();
#endif
      return false;
    }
#ifdef DEBUG
    v.p(ip_address);
    v.l();
#endif
  }
  else
  {
    strncpy(ip_address, p_ip, sizeof(ip_address));
  }

  //Open the socket
  int timeout = 500;

  do
  {
    bool connection_failed = false;

    for (int i = 0; i < TCP_SETUP_OPEN_SOCKET_RETRY; i++)
    {
      sprintf(m_buffer, "AT+CIPOPEN=%u,\"TCP\",\"%s\",%u\r", p_socket, ip_address, p_port);
#ifdef DEBUG
      v.vp(2, "--Create TCP Connection ");
      v.p(p_socket);
      v.p(": ");
#endif

      if (sendATCommand(m_buffer, timeout * (i + 1)))
      {
#ifdef DEBUG
        v.p("SUCCESS");
        v.l();
#endif
        break;
      }
      else if (i == 9)
      {
        connection_failed = true;
        break;
      }
#ifdef DEBUG
      v.p("FAILED...TRY AGAIN");
      v.l();
#endif
    }

    if (connection_failed)
      break;

    m_sockets[p_socket].active = true;
    m_sockets[p_socket].tcp = true;

    return true;
  } while (0);

#ifdef DEBUG
  v.p("FAILED");
  v.l();
#endif
  return false;
}

//Opens a socket to send data using UDP
bool TCPSIM5360::openUDPSocket(uint16_t p_port, unsigned int p_socket)
{
#ifdef DEBUG
  v.vpl(2, "Attmepting to open UDP socket: ");
#endif
  //Check if the socket is already open
  if (m_sockets[p_socket].active)
  {
    return false;
#ifdef DEBUG
    v.vpl(2, "Socket Taken, cannot open");
#endif
  }

  //Test if net is open. If open, return true
  bool netopen = sendATCommand("AT+NETOPEN?\r", 5000, "+NETOPEN: 1");
  if (!netopen)
  {
#ifdef DEBUG
    v.vpl(2, "NET is not open. Net must be opened to create sockets");
#endif
    return false;
  }

  //Open the socket
  int timeout = 500;

  do
  {
    bool connection_failed = false;

    for (int i = 0; i < TCP_SETUP_OPEN_SOCKET_RETRY; i++)
    {
      sprintf(m_buffer, "AT+CIPOPEN=%u,\"UDP\",,,%u\r", p_socket, p_port);
#ifdef DEBUG
      v.vp(2, "--Create UDP Connection ");
      v.p(p_socket);
      v.p(": ");
#endif

      if (sendATCommand(m_buffer, timeout * (i + 1)))
      {
#ifdef DEBUG
        v.p("SUCCESS");
        v.l();
#endif
        break;
      }
      else if (i == 9)
      {
        connection_failed = true;
        break;
      }
#ifdef DEBUG
      v.p("FAILED...TRY AGAIN");
      v.l();
#endif
    }

    if (connection_failed)
      break;

    m_sockets[p_socket].active = true;
    m_sockets[p_socket].tcp = false;

    return true;
  } while (0);

#ifdef DEBUG
  v.p("FAILED");
  v.l();
#endif
  return false;
}

//Helper function that prints connections, you need to have XBEE DEBUG defined in FreematicsONE
void TCPSIM5360::printConnections()
{
  sendATCommand("AT+CIPOPEN?\r");
}

//---------------------------------------------------
//DATA SENDING

bool TCPSIM5360::sendTCPData(const char *p_data, unsigned int p_len, unsigned int p_socket)
{
#ifdef DEBUG
  v.vp(2, "Attempting to send TCP data over socket: ");
  v.p(p_socket);
  v.l();
#endif

  //Check that socket is open
  if (!m_sockets[p_socket].active || !m_sockets[p_socket].tcp)
  {
#ifdef DEBUG
    v.vpl(2, "Socket not open for TCP communication");
#endif
    return false;
  }

  //Try to send the TCP data
  for (int i = 0; i < SEND_DATA_RETRIES; i++)
  {
    sprintf(m_buffer, "AT+CIPSEND=%u,%u\r", p_socket, p_len); //p394 AT Command Manual - Load buffer with CIPSEND send data AT Command

    if (sendATCommand(m_buffer, SEND_DATA_CARROT_TIMEOUT, ">"))
    { //Wait for > symbol to give data to send

#ifdef DEBUG
      v.vp(4, "The > symbol was returned, now we write data to send using xbWrite. Return success: ");
#endif
      xbWrite(p_data, p_len); //Write data to UART tx buffer

      bool suc = sendATCommand(0, SEND_DATA_LAST_OK_TIMEOUT); //Try to recieve OK from hte rx buffer. Indicates send successful
#ifdef DEBUG
      v.p(suc);
      v.l();
#endif
      return suc;
    }
#ifdef DEBUG
    v.vpl(4, "Reattempt - no > detected");
#endif
  }

#ifdef DEBUG
  v.vpl(4, "! The > symbol was never returned on time. Send failed");
#endif
  return false;
}

bool TCPSIM5360::sendUDPData(const char *p_data, unsigned int p_len, const char *p_ip, uint16_t p_port, bool p_dns, unsigned int p_socket)
{
#ifdef DEBUG
  v.vp(2, "Attempting to send UDP data over socket: ");
  v.p(p_socket);
  v.l();
#endif

  //Check that socket is open
  if (!m_sockets[p_socket].active || m_sockets[p_socket].tcp)
  {
#ifdef DEBUG
    v.vpl(2, "Socket not open for UDP communication");
#endif
    return false;
  }

  //Do DNS lookup if necessary
  char ip_address[16];
  if (p_dns)
  {
#ifdef DEBUG
    v.vp(2, "--DNS Lookup (Get Destination IP): ");
#endif

    String ip = TCPQueryIP(p_ip);
    if (ip.length())
    {
      strncpy(ip_address, ip.c_str(), sizeof(ip_address));
    }
    else
    {
#ifdef DEBUG
      v.p("FAILED");
      v.l();
#endif
      return false;
    }
#ifdef DEBUG
    v.p(ip_address);
    v.l();
#endif
  }
  else
  {
    strncpy(ip_address, p_ip, sizeof(ip_address));
  }

  //Try to send the UDP data
  for (int i = 0; i < SEND_DATA_RETRIES; i++)
  {
    sprintf(m_buffer, "AT+CIPSEND=%u,%u,\"%s\",%u\r", p_socket, p_len, ip_address, p_port); //p394 AT Command Manual - Load buffer with CIPSEND send data AT Command

    if (sendATCommand(m_buffer, SEND_DATA_CARROT_TIMEOUT, ">"))
    { //Wait for > symbol to give data to send
#ifdef DEBUG
      v.vp(4, "The > symbol was returned, now we write data to send using xbWrite. Return success: ");
      Serial.println("");
#endif


#ifdef DEBUG
      for (int i = 0; i < p_len; i++)
      {
        Serial.print(p_data[i], HEX);
        Serial.print(" ");
      }

      Serial.println("");
#endif

      xbWrite(p_data, p_len); //Write data to UART tx buffer

      bool suc = sendATCommand(0, SEND_DATA_LAST_OK_TIMEOUT); //Try to recieve OK from hte rx buffer. Indicates send successful
#ifdef DEBUG
      v.p(suc);
      v.l();
#endif
      return suc;
    }
#ifdef DEBUG
    v.vpl(4, "Reattempt - no > detected");
#endif
  }

#ifdef DEBUG
  v.vpl(4, "! The > symbol was never returned on time. Send failed");
#endif
  return false;
}

bool TCPSIM5360::closeSocket(unsigned int p_socket)
{
  char localbuffer[50];
  sprintf(localbuffer, "AT+CIPCLOSE=%u\r", p_socket);

  bool closed = sendATCommand(localbuffer);
  if (closed)
    m_sockets[p_socket].active = false;
  return closed;
}

//---------------------------------------------------
//DATA RECEIVING

//Returns a char* to recieved data. finds the data using +IPD and returns a pointer to it
char *TCPSIM5360::recieveData(int *p_bytes, unsigned int p_timeout)
{
  if (sendATCommand(0, p_timeout, "+IPD"))
  {

    char *p = strstr(m_buffer, "+IPD"); //Search for +IPD in UART rx buffer

    if (!p)
      return 0; //if not there, return 0

    int len = atoi(p + 4); //Get the length. +IPD for mat is +IPD32 for 32 bytes recieved, +IPD## for ## bytes recieved
    if (p_bytes)
      *p_bytes = len;

    p = strchr(p, '\n'); //After +IPD## there is a \n. After the \n there are len bytes of data
    if (p)
    {
      *(++p + len) = 0;
      return p;
    }
  }
  return 0;
}

// Tells you if any data is sitting the UART rx buffer
bool TCPSIM5360::recDataAvailable(size_t *o_size)
{ //Gets the amount fo bytes of data in the rx buffer of the UART directly from UART
  uart_get_buffered_data_len(BEE_UART_NUM, o_size);
  return *o_size > 0;
}

//Finds the data using +IPD, but instead of returning it, it gives the data to the extraction callback which processes it.
//Useful for more complex data reception situations.
void TCPSIM5360::extractRecievedData(void *p_ref, void (*p_use_extraction)(void *p_refrence, char *p_extraction, int len))
{

  if (p_use_extraction == NULL)
    xbPurge(); //If there is no extraction callback, flush uart

  //memset(m_buffer, 0, TCP_SENDREC_BUFFER_SIZE);
  xbRead(m_buffer, 500, TCP_SENDREC_BUFFER_SIZE);

  //Holds pointer to position in buffer currently interested in
  char *extract = m_buffer;
  char process_buffer[256];

  do
  {

    Serial.println("Rec Data Loop");

    extract = strstr(extract, "+IPD"); //Get pointer to +IPD
    if (!extract)
      break; //If nullptr end extraction since no match found

    int len = atoi(extract + 4);         //atoi converts next string number to int. This is length
    extract = strstr(extract, "\n") + 1; //\n marks start of message. Move pointer 1 passed this.

    if (len < TCP_SENDREC_BUFFER_SIZE)
    {                                               //If length is less than buffer, we have space to deal with extraction
      memcpy(process_buffer, extract, len);         //Copy to process buffer
      process_buffer[len] = 0;                      //Make last character 0 for end of string
      p_use_extraction(p_ref, process_buffer, len); //use callback to deal with what we received. Needs a reference to an object passed also
    }

    extract += len; //Move pointer passed data just dealt with.
  } while (1);

  xbPurge();
}

//---------------------------------------------------
//HELPER FUCNTIONS

int TCPSIM5360::networkSignal()
{
#ifdef DEBUG
  v.vpl(2, "Getting Signal Strength: ");
#endif

  if (sendATCommand("AT+CSQ\r", 500))
  { //Get current signal strength of chip
    char *p = strchr(m_buffer, ':');
    if (p)
    {
      p += 2;
      int db = atoi(p) * 10; //Converts the string number signal to int
      p = strchr(p, '.');
      if (p)
        db += *(p + 1) - '0';
#ifdef DEBUG
      v.vpl(2, db);
#endif
      return db;
    }
  }
#ifdef DEBUG
  v.vpl(2, "FAILED");
#endif
  return -1;
}

char *TCPSIM5360::networkOperator()
{
#ifdef DEBUG
  v.vp(2, "Getting Operator Name: ");
#endif

  // display operator name
  if (sendATCommand("AT+COPS?\r") == 1)
  {
    char *p = strstr(m_buffer, ",\"");
    if (p)
    {
      p += 2;
      char *s = strchr(p, '\"');
      if (s)
        *s = 0;
#ifdef DEBUG
      v.p(p);
      v.l();
#endif
      return p;
    }
  }
  v.vpl(2, "FAILED");
  return (char *)"";
}

char *TCPSIM5360::TCPGetIP()
{
  uint32_t t = millis();
#ifdef DEBUG
  v.vp(2, "Aquiring socket ip: ");
#endif
  do
  {
#ifdef DEBUG
    v.p(".");
#endif
    if (sendATCommand("AT+IPADDR\r", 5000, "\r\nOK\r\n", true))
    { //Gets the IP address of current PDP address socket
      char *p = strstr(m_buffer, "+IPADDR:");
      if (p)
      {
        char *ip = p + 9;
        if (*ip != '0')
        {
          char *q = strchr(ip, '\r');
          if (q)
            *q = 0;
#ifdef DEBUG
          v.p(ip);
          v.l();
#endif
          return ip;
        }
      }
    }
    sleep(500);
  } while (millis() - t < 15000);
#ifdef DEBUG
  v.p(m_buffer);
  v.l();
#endif

#ifdef DEBUG
  v.vpl(2, "NOT FOUND");
#endif
  return (char *)"";
}

String TCPSIM5360::TCPQueryIP(const char *host)
{
  sprintf(m_buffer, "AT+CDNSGIP=\"%s\"\r", host);
  if (sendATCommand(m_buffer, 10000))
  {
    char *p = strstr(m_buffer, host);
    if (p)
    {
      p = strstr(p, ",\"");
      if (p)
      {
        char *ip = p + 2;
        p = strchr(ip, '\"');
        if (p)
          *p = 0;
        Serial.println(ip);
        return ip;
      }
    }
  }
  return "";
}

//PROTECTED FUNCTIONS ==================================

//---------------------------------------------------
//AT Commands - Used to send AT commands directly to the router. Can use the expected char* to check response. Returns true if expected occurs in the response.
bool TCPSIM5360::sendATCommand(const char *cmd, unsigned int timeout, const char *expected, bool terminated)
{
  if (cmd)
  {
    xbWrite(cmd);
  }
  m_buffer[0] = 0;
  byte ret = xbReceive(m_buffer, sizeof(m_buffer), timeout, &expected, 1);
  if (ret)
  {
    if (terminated)
    {
      char *p = strstr(m_buffer, expected);
      if (p)
        *p = 0;
    }
    return true;
  }
  else
  {
    return false;
  }
}

//PRIVATE FUNCTIONS ==================================

//---------------------------------------------------
//NETWORK SETUP FUNCTIONS
bool TCPSIM5360::routerSetup()
{
#ifdef DEBUG
  v.vp(2, "Setting Router Baudate: ");
  v.vpl(2, XBEE_BAUDRATE);
#endif

  if (m_router_stage == 0)
  {
    xbBegin(XBEE_BAUDRATE); //Set the baudrate of the UART port that will be used to communicate with the SIMCOM chip
    m_router_stage = 1;
  }

#ifdef DEBUG
  v.vp(2, "Removing Stale Data: ");
#endif

  for (byte n = 0; n < ROUTER_RETRY; n++)
  { //Get rid of stale data and get ready to set up network info

#ifdef DEBUG
    v.vp(2, ".");
#endif

    xbTogglePower();
    sleep(3000);
    xbPurge();
    for (byte m = 0; m < 5; m++)
    {
      if (sendATCommand("AT\r"))
      { //Send an AT Command. If OK is recieved then SIMCOM is ready
        m_router_stage = 2;

#ifdef DEBUG
        v.l();
#endif

        return true;
      }
    }
  }
#ifdef DEBUG
  v.l();
#endif
  return false;
}

bool TCPSIM5360::networkSetup(const char *p_apn, const char *p_cellusr, const char *p_cellpwd /*, bool p_only3G*/)
{
  uint32_t t = millis();
  bool success = false;

#ifdef DEBUG
  v.vp(2, "Setting up Network using APN: ");
  v.p((char *)p_apn);
  v.p(", Cell User: ");
  v.p((char *)p_cellusr);
  v.p(", Cell Password: ");
  v.p((char *)p_cellpwd);
  v.l();
#endif

  m_apn = p_apn;
  m_cellusr = p_cellusr;
  m_cellpwd = p_cellpwd;

  sendATCommand("ATE0\r");
  //if (p_only3G) sendATCommand("AT+CNMP=14\r"); // use WCDMA only
  do
  {

#ifdef DEBUG
    v.vp(2, "Testing UE system is Online with service: ");
#endif
    do
    {
#ifdef DEBUG
      v.p(".");
#endif
      sleep(500);
      success = sendATCommand("AT+CPSI?\r", 1000, "Online"); //Testing UE system is online
      if (success)
      {
        if (!strstr(m_buffer, "NO SERVICE"))
          break;
        success = false;
      }
      else
      {
        if (strstr(m_buffer, "Off"))
          break;
      }
    } while (millis() - t < NETWORK_CONNECT_TIMEOUT);
    if (!success)
      break;
#ifdef DEBUG
    v.p(" OK\n");
#endif

#ifdef DEBUG
    v.vp(2, "Checking Signal quality: ");
#endif
    do
    {
#ifdef DEBUG
      v.p(".");
#endif
      success = sendATCommand("AT+CSQ\r", 5000, "+CSQ:"); //Checking signal quality
      if (success)
      {
        //CSQ gives back a signal strength, should probably put a check in here to make sure it's between the right numbers
      }
    } while (!success && millis() - t < NETWORK_CONNECT_TIMEOUT);
    if (!success)
      break;
#ifdef DEBUG
    v.p(" OK\n");
#endif

#ifdef DEBUG
    v.vp(2, "Checking network registration = registered, roaming: ");
#endif
    do
    {
#ifdef DEBUG
      v.p(".");
#endif
      success = sendATCommand("AT+CREG?\r", 5000, "+CREG: 0,5"); //Registering the network
    } while (!success && millis() - t < NETWORK_CONNECT_TIMEOUT);
    if (!success)
      break;
#ifdef DEBUG
    v.p(" OK\n");
#endif

#ifdef DEBUG
    v.vp(2, "Checking GPRS network registration = registered, roaming: ");
#endif
    do
    {
#ifdef DEBUG
      v.p(".");
#endif
      success = sendATCommand("AT+CGREG?\r", 1000, "+CGREG: 0,5"); //GPRS network registration
    } while (!success && millis() - t < NETWORK_CONNECT_TIMEOUT);
    if (!success)
      break;
#ifdef DEBUG
    v.p(" OK\n");
#endif

#ifdef DEBUG
    v.vp(2, "Define socket PDP context, APN: ");
    v.p((char *)p_apn);
#endif

    do
    {
#ifdef DEBUG
      v.p(".");
#endif
      sprintf(m_buffer, "AT+CGSOCKCONT=1,\"IP\",\"%s\"\r", p_apn); //Define PDP context (IP4)
      success = sendATCommand(m_buffer);
    } while (!success && millis() - t < NETWORK_CONNECT_TIMEOUT);
    if (!success)
      break;
#ifdef DEBUG
    v.p(" OK \n");
#endif

#ifdef DEBUG
    v.vp(2, "Setup up authentication for the PDP-IP socket");
#endif
    do
    {
#ifdef DEBUG
      v.p(".");
#endif
      sprintf(m_buffer, "AT+CSOCKAUTH=1,1,\"%s\",\"%s\"\r", p_cellpwd, p_cellusr); //Set PDP-IP authentication type
      success = sendATCommand(m_buffer);
    } while (!success && millis() - t < NETWORK_CONNECT_TIMEOUT);
    if (!success)
      break;
#ifdef DEBUG
    v.p(" OK\n");
#endif

#ifdef DEBUG
    v.vpl(2, "Set active PDP context profile number = 1");
#endif
    sendATCommand("AT+CSOCKSETPN=1\r"); //Set active PDP context profile number

#ifdef DEBUG
    v.vpl(2, "Set TCPIP application mode to Command mode = 0");
#endif
    sendATCommand("AT+CIPMODE=0\r"); //Select TCPIP applicaitno mode (data or command, in this case data)

    //sendATCommand("AT+NETOPEN\r", 10000);
  } while (0);
  if (!success)
  {
#ifdef DEBUG
    v.vpl(2, m_buffer);
    v.vpl(2, "__Sorocom Network Failed");
#endif
    return false;
  }

#ifdef DEBUG
  v.vpl(2, "__Sorocom Network Established");
#endif
  return true;
}

void TCPSIM5360::resetSockets()
{
  for (int i = 0; i < 10; i++)
  {
    m_sockets[i].active = false;
  }
}

bool TCPSIM5360::checkConnection(){
  if(sendATCommand("AT+IPADDR\r", 5000, "\r\nOK\r\n", true)){
#ifdef DEBUG
    v.vpl(2, "Connection exists");
#endif
    return true;
  }
#ifdef DEBUG
  v.vpl(2, "Connection not exist");
#endif
  return false;
}