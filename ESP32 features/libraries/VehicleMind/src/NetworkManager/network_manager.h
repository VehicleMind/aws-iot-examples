//---------------------------------------------------
//INCLUDE GUARD
#ifndef NETWORK_MANAGER_H_INCLUDED
#define NETWORK_MANAGER_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPS
#include "tcp/tcp_sim5360.h"
#include "mqtt/mqtt_client.h"
#include "utils/timer.h"
#include "../Tools/Verbose.h"
#include "sntp/sntp_udp.h"
#include "../Tools/uuid_gen.h"
#include "../ChipID/ChipID.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_task_wdt.h"

//---------------------------------------------------
//FORWARD DEPS

//---------------------------------------------------
//HEADER
// #define DEBUG

#define CELL_APN "soracom.io"
#define CELL_USER "sora"
#define CELL_PASSWORD "sora"

#define TCP_IP "beam.soracom.io"
#define TCP_PORT 1883

// uuid v5 nemespace for generating deviceId based on IMEI
#define NAMESPACEID "e728b802-0fff-5b7b-bf47-51bacb45d446" 

// //UBUNTU
// #define TCP_IP "52.90.1.66"
// #define TCP_PORT 1883

// #define TCP_IP "23.233.109.22"
// #define TCP_PORT 1883


#define MQTT_CONNNECT_TIMEOUT 30000
#define MQTT_CONNNECT_ATTEMPTS 5

#define MQTT_PUBLISH_TIMEOUT 5000
#define MQTT_PUBLISH_ATTEMPTS 5

//I put the << 0 in the indicate it's a bit. NE stands for Network Event
#define NE_CONNECTED (1 << 0) 


//Network manager class
class NetworkManager
{

  public:
    NetworkManager( );

    // repair Connection var (temp)
    uint32_t totalAttempts;

    bool connect(unsigned int p_retries); // Handles initial connection to the network and the MQTT broker
    bool disconnect();
    
    void send(const char *p_type, const char *p_message);                     //Send a type of data then that data as a string
    void send(const char *p_type, uint8_t *p_values, unsigned int p_size);    //Send a type of data then that data as a byte array

    bool isConnected(){ return m_is_connected; }

  private:
    //FreeRTOS Task Variables
    EventGroupHandle_t m_network_events;    //Event group holding network events
       

    //TCP COnnection Variables 
    TCPSIM5360 m_tcp_connector;  

    SemaphoreHandle_t m_uart_sema;          //Semaphore used to control UART access
    TaskHandle_t m_recTask;         //Handles and task callbacks    

    bool m_is_connected;
    

    //MQTT Variables
    MQTTClient m_mqtt_client; 
    char m_mqtt_client_id[37];

    Timer m_command_timer;        //Timers used by the MQTT client
    Timer m_keepalive_timer;

    lwmqtt_return_code_t m_last_connection_return_code; //Used in MQTT Stuff

    void setMqttClientId();


    //SNTP     
    SNTPUDP m_sntp;
    uint8_t m_sntp_packet[48];
    static void sntp_extract(void* p_ref, char* p_extract, int p_len);

    //Methods --------

    //Task callbacks
    static void recTaskCallback(void* p_manager_ref);
    static void reconTaskCallback(void* p_manager_ref);
    
    //Network Functions    
    bool repairConnection();      //Repairs connections on breaking
    bool protocolConnect();       //MQTT Connect packet code    
    bool subscribe(const char* topic);  //Subscribes to topics for recieving commands
    bool protocolSend(int p_channel, const char *p_topic, const char *p_message);   //Abstracted from protocol(ish), eventually remove topic completely
    bool protocolSend(int p_channel, const char *p_topic, uint8_t *p_message, unsigned int p_message_size);   //Abstracted from protocol(ish), eventually remove topic completely
    
    //These 2 functions are used to extract revieced messages properly. It's a bit of a hack right now. Will be make better later
    void extractLoop();
    static void dealWithExtraction(void* p_ref, char* p_extraction, int p_length);
    
    //Callbacks for the MQTT client
    static lwmqtt_err_t mqtt_read(void* ref, uint8_t *buf, size_t len, size_t *read, int32_t timeout);    

    //Need special wrappers to write to sockets properly
    static lwmqtt_err_t mqtt_write(void* ref, int buf, size_t len, size_t *sent, int32_t timeout, unsigned int p_socket);
    
    static lwmqtt_err_t mqtt_write0(void* ref, int buf, size_t len, size_t *sent, int32_t timeout){
      return mqtt_write(ref, buf, len, sent, timeout, 0);
    };
    
    static void mqtt_callback(lwmqtt_string_t str, lwmqtt_message_t msg);
 
    
    Verbose v;  //This object is a debugging tool and will be removed eventually
};




#endif





