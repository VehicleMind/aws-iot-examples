//---------------------------------------------------
//INCLUDE GUARD
#ifndef MQTT_CLIENT_H_INCLUDED
#define MQTT_CLIENT_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPS
#include "mqtt_packet.h"
#include "../../Tools/Verbose.h"


//---------------------------------------------------
//HEADER
// #define DEBUG
#define VM_MQTT_READ_BUFFER_SIZE 350
#define VM_MQTT_WRITE_BUFFER_SIZE 350

class MQTTClient
{

public:
  //Construction. Need to call all these functions to set up properly. Maybe in future we should make this 1 constructor
  MQTTClient();
  void lwmqtt_set_network(void *p_read_ref, lwmqtt_network_read_t p_read, void *p_write_ref, lwmqtt_network_write_t p_write);
  void lwmqtt_set_timers(Timer *p_keep_alive_timer, Timer *p_network_timer);
  void lwmqtt_set_callback(lwmqtt_callback_t p_callback);

  //MQTT commands
  lwmqtt_err_t lwmqtt_connect(lwmqtt_options_t p_options, lwmqtt_will_t *p_will, lwmqtt_return_code_t *p_return_code, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_subscribe(int p_count, lwmqtt_string_t *p_topic_filter, lwmqtt_qos_t *p_qos, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_subscribe_one(lwmqtt_string_t p_topic_filter, lwmqtt_qos_t p_qos, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_unsubscribe(int p_count, lwmqtt_string_t *p_topic_filter, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_unsubscribe_one(lwmqtt_string_t p_topic_filter, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_publish(lwmqtt_string_t p_topic, lwmqtt_message_t p_message, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_disconnect(uint32_t p_timeout);

  //These commands have special meanings. Yield is supposed to be used to receive packets
  //but because of the way reciving packets works from SIMCOM modem I have written
  //lwmqtt_process_packet which should be used instead of yeild for now.
  lwmqtt_err_t lwmqtt_yield(size_t p_available, uint32_t p_timeout);
  lwmqtt_err_t lwmqtt_keep_alive(uint32_t timeout);
  void lwmqtt_process_packet(char *p_data, int p_len); //When there is data avalable in the UART use this to try to extract an MQTT packet

private:
  Verbose v;

  //Client Variables
  lwmqtt_client_t m_client;

  uint8_t read_buffer[VM_MQTT_READ_BUFFER_SIZE];
  uint8_t write_buffer[VM_MQTT_WRITE_BUFFER_SIZE];

  //Network and lifecycle functions
  uint16_t lwmqtt_get_next_packet_id();
  lwmqtt_err_t lwmqtt_read_from_network(size_t p_offset, size_t p_len);
  lwmqtt_err_t lwmqtt_read_packet_in_buffer(size_t *o_read, lwmqtt_packet_type_t *o_packet_type);
  lwmqtt_err_t lwmqtt_send_packet_in_buffer(size_t p_length);
  lwmqtt_err_t lwmqtt_cycle(size_t *p_read, lwmqtt_packet_type_t *p_packet_type);
  lwmqtt_err_t lwmqtt_cycle_until(lwmqtt_packet_type_t *p_packet_type, size_t p_available, lwmqtt_packet_type_t p_needle);

  lwmqtt_err_t lwmqtt_packet_type_interpret(lwmqtt_packet_type_t p_packet_type);
};

#endif