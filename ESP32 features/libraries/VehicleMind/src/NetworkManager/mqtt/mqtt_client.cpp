#include <HardwareSerial.h>
#include "mqtt_client.h"

//PUBLIC FUNCTIONS ==================================

//---------------------------------------------------
//CONSTRUCTOR and INIT FUNCTIONS

//Sets up the object without callbacks and network functions
MQTTClient::MQTTClient():v(false, "MQT")
{
  m_client.last_packet_id = 1;
  m_client.keep_alive_interval = 0;
  m_client.pong_pending = false;

  m_client.write_buf = write_buffer;
  m_client.write_buf_size = VM_MQTT_WRITE_BUFFER_SIZE;
  m_client.read_buf = read_buffer;
  m_client.read_buf_size = VM_MQTT_READ_BUFFER_SIZE;

  m_client.callback = NULL;

  m_client.network_read = NULL;
  m_client.network_write = NULL;

  m_client.keep_alive_timer = NULL;
  m_client.command_timer = NULL;
}

//Sets up object references and callback functions for network reads and writes
void MQTTClient::lwmqtt_set_network(void *p_read_ref, lwmqtt_network_read_t p_read, void *p_write_ref, lwmqtt_network_write_t p_write)
{
  m_client.network_read = p_read;
  m_client.network_write = p_write;
  m_client.network_read_ref = p_read_ref;
  m_client.network_write_ref = p_write_ref;
}

//Sets up the timer objects
void MQTTClient::lwmqtt_set_timers(Timer *p_keep_alive_timer, Timer *p_command_timer)
{
  m_client.keep_alive_timer = p_keep_alive_timer;
  m_client.command_timer = p_command_timer;
}

//Sets up the callbakc. This is used to deal with recieved MQTT packets that have been processed
void MQTTClient::lwmqtt_set_callback(lwmqtt_callback_t p_callback)
{
  m_client.callback = p_callback;
}


//---------------------------------------------------
//Basic MQTT Commands


//Send a connect packet
lwmqtt_err_t MQTTClient::lwmqtt_connect(lwmqtt_options_t p_options, lwmqtt_will_t *p_will, lwmqtt_return_code_t *p_return_code, uint32_t p_timeout)
{
  // set timer to command timeout
  m_client.command_timer->setTimeout(p_timeout);

  // save keep alive interval (take 75% to be a little earlier than actually needed)
  m_client.keep_alive_interval = (uint32_t)(p_options.keep_alive * 750);

  // set keep alive timer
  m_client.keep_alive_timer->setTimeout(m_client.keep_alive_interval);
  // reset pong pending flag
  m_client.pong_pending = false;

#ifdef DEBUG
  v.vpl(2, "Enconding Connect Packet: ");
#endif
  // encode connect packet
  size_t len;
  // lwmqtt_err_t err = lwmqtt_encode_connect(m_client.write_buf, m_client.write_buf_size, &len, options, will);
  lwmqtt_err_t err = Packet::lwmqtt_encode_connect(m_client.write_buf, m_client.write_buf_size, &len, p_options, NULL);

  if (err != LWMQTT_SUCCESS)
  {
#ifdef DEBUG
    v.vp(2, "ERROR: ", err);
#endif
    return err;
  }

#ifndef DEBUG
  v.vpl(2, "Sending packet in buffer(connect packet)");
#endif
  // send packet
  err = lwmqtt_send_packet_in_buffer(len);
  if (err != LWMQTT_SUCCESS)
  {
#ifdef DEBUG
    v.vp(2, "! ERROR: ", err);
#endif
    return err;
  }


  // wait for connack packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(&packet_type, 0, LWMQTT_CONNACK_PACKET);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }
  else if (packet_type != LWMQTT_CONNACK_PACKET)
  {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode connack packet
  bool session_present;
  err = Packet::lwmqtt_decode_connack(m_client.read_buf, m_client.read_buf_size, &session_present, p_return_code);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // return error if connection was not accepted
  if (*p_return_code != LWMQTT_CONNECTION_ACCEPTED)
  {
    return LWMQTT_CONNECTION_DENIED;
  }

  return LWMQTT_SUCCESS;
}



lwmqtt_err_t MQTTClient::lwmqtt_subscribe(int p_count, lwmqtt_string_t *p_topic_filter, lwmqtt_qos_t *p_qos, uint32_t p_timeout)
{
  // set timeout
  m_client.command_timer->setTimeout(p_timeout);

  uint8_t buft[300];

  // encode subscribe packet
  size_t len;
  lwmqtt_err_t err = Packet::lwmqtt_encode_subscribe(m_client.write_buf, m_client.write_buf_size, &len, lwmqtt_get_next_packet_id(), p_count, p_topic_filter, p_qos);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // send packet
  err = lwmqtt_send_packet_in_buffer(len);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // wait for suback packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(&packet_type, 0, LWMQTT_SUBACK_PACKET);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }
  else if (packet_type != LWMQTT_SUBACK_PACKET)
  {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode packet
  int suback_count = 0;
  lwmqtt_qos_t granted_qos[p_count];
  uint16_t packet_id;
  err = Packet::lwmqtt_decode_suback(m_client.read_buf, m_client.read_buf_size, &packet_id, p_count, &suback_count, granted_qos);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // check suback codes
  for (int i = 0; i < suback_count; i++)
  {
    if (granted_qos[i] == LWMQTT_QOS_FAILURE)
    {
      return LWMQTT_FAILED_SUBSCRIPTION;
    }
  }
  return LWMQTT_SUCCESS;
}



lwmqtt_err_t MQTTClient::lwmqtt_subscribe_one(lwmqtt_string_t p_topic_filter, lwmqtt_qos_t p_qos, uint32_t p_timeout)
{
  return lwmqtt_subscribe(1, &p_topic_filter, &p_qos, p_timeout);
}



lwmqtt_err_t MQTTClient::lwmqtt_unsubscribe(int p_count, lwmqtt_string_t *p_topic_filter, uint32_t p_timeout)
{
  // set timer
  m_client.command_timer->setTimeout(p_timeout);

  // encode unsubscribe packet
  size_t len;
  lwmqtt_err_t err = Packet::lwmqtt_encode_unsubscribe(m_client.write_buf, m_client.write_buf_size, &len,
                                                       lwmqtt_get_next_packet_id(), p_count, p_topic_filter);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // send unsubscribe packet
  err = lwmqtt_send_packet_in_buffer(len);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // wait for unsuback packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(&packet_type, 0, LWMQTT_UNSUBACK_PACKET);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }
  else if (packet_type != LWMQTT_UNSUBACK_PACKET)
  {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode unsuback packet
  bool dup;
  uint16_t packet_id;
  err = Packet::lwmqtt_decode_ack(m_client.read_buf, m_client.read_buf_size, packet_type, &dup, &packet_id);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  return LWMQTT_SUCCESS;
}



lwmqtt_err_t MQTTClient::lwmqtt_unsubscribe_one(lwmqtt_string_t p_topic_filter, uint32_t p_timeout)
{
  return lwmqtt_unsubscribe(1, &p_topic_filter, p_timeout);
}



lwmqtt_err_t MQTTClient::lwmqtt_publish(lwmqtt_string_t p_topic, lwmqtt_message_t p_message, uint32_t p_timeout)
{
#ifdef DEBUG
  v.vpl(2, "Publishing Packet");
#endif
  // set timer
  m_client.command_timer->setTimeout(p_timeout);

  // add packet id if at least qos 1
  uint16_t packet_id = 0;
  if (p_message.qos == LWMQTT_QOS1 || p_message.qos == LWMQTT_QOS2)
  {
#ifdef DEBUG
    v.vp(2, "QOS 1 or 2... Getting packet id: ");
#endif
    packet_id = lwmqtt_get_next_packet_id();
#ifdef DEBUG
    v.p((int)packet_id); v.l();
#endif
  }

#ifdef DEBUG
  v.vpl(2, "Encoding the publish packet");
#endif
  // encode publish packet
  size_t len = 0;
  lwmqtt_err_t err = Packet::lwmqtt_encode_publish(m_client.write_buf, m_client.write_buf_size, &len, 0, packet_id, p_topic, p_message);
  if (err != LWMQTT_SUCCESS)
  {
#ifdef DEBUG
    v.vpl(2, "MQ --! ERROR: ", err);
#endif
    return err;
  }

#ifdef DEBUG
  v.vpl(2, "MQ -- Sending the publish packet");
#endif
  // send packet
  err = lwmqtt_send_packet_in_buffer(len);
  if (err != LWMQTT_SUCCESS)
  {
#ifdef DEBUG
    v.vpl(2, "MQ --! ERROR: ", err);
#endif
    return err;
  }

  // immediately return on qos zero
  if (p_message.qos == LWMQTT_QOS0)
  {
    return LWMQTT_SUCCESS;
  }

  // define ack packet
  lwmqtt_packet_type_t ack_type = LWMQTT_NO_PACKET;
  if (p_message.qos == LWMQTT_QOS1)
  {
    ack_type = LWMQTT_PUBACK_PACKET;
  }
  else if (p_message.qos == LWMQTT_QOS2)
  {
    ack_type = LWMQTT_PUBCOMP_PACKET;
  }

  // wait for ack packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(&packet_type, 0, ack_type);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }
  else if (packet_type != ack_type)
  {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode ack packet
  bool dup;
  err = Packet::lwmqtt_decode_ack(m_client.read_buf, m_client.read_buf_size, packet_type, &dup, &packet_id);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  return LWMQTT_SUCCESS;
}



lwmqtt_err_t MQTTClient::lwmqtt_disconnect(uint32_t p_timeout)
{
  // set timer
  m_client.command_timer->setTimeout(p_timeout);

  // encode disconnect packet
  size_t len;
  lwmqtt_err_t err = Packet::lwmqtt_encode_zero(m_client.write_buf, m_client.write_buf_size, &len, LWMQTT_DISCONNECT_PACKET);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // send disconnected packet
  err = lwmqtt_send_packet_in_buffer(len);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  return LWMQTT_SUCCESS;
}




//---------------------------------------------------
//Special Commands

//Yield is used to receieve packets, Basically it just does an until cycle and recieves whatever it can
lwmqtt_err_t MQTTClient::lwmqtt_yield(size_t p_available, uint32_t p_timeout)
{
  // set timeout
  m_client.command_timer->setTimeout(p_timeout);

  // cycle until timeout has been reached
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  lwmqtt_err_t err = lwmqtt_cycle_until(&packet_type, p_available, LWMQTT_NO_PACKET);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  m_client.read_buf[0] = 0;

  return LWMQTT_SUCCESS;
}


//Bascially does a ping pong handshake to keep session alive
lwmqtt_err_t MQTTClient::lwmqtt_keep_alive(uint32_t timeout)
{
  // set timer
  m_client.command_timer->setTimeout(timeout);

  // return immediately if keep alive interval is zero
  if (m_client.keep_alive_interval == 0)
  {
    return LWMQTT_SUCCESS;
  }

  // return immediately if no ping is due
  if (m_client.keep_alive_timer->getTimeLeft() > 0)
  {
    return LWMQTT_SUCCESS;
  }

  // a ping is due

  // fail immediately if a pong is already pending
  if (m_client.pong_pending)
  {
    return LWMQTT_PONG_TIMEOUT;
  }

  // encode pingreq packet
  size_t len;
  lwmqtt_err_t err = Packet::lwmqtt_encode_zero(m_client.write_buf, m_client.write_buf_size, &len, LWMQTT_PINGREQ_PACKET);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // send packet
  err = lwmqtt_send_packet_in_buffer(len);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // set flag
  m_client.pong_pending = true;

  return LWMQTT_SUCCESS;
}


//This has some copy pasted code with lwmqtt_yeild. Might want to extract as a private function to simplify later
void MQTTClient::lwmqtt_process_packet(char *p_data, int p_len)
{ 
  if(!(p_len < VM_MQTT_READ_BUFFER_SIZE)) return;     //if data too big just don't process

  memcpy(read_buffer, p_data, p_len);     //cpy data to buffer

  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;    
  lwmqtt_err_t err = Packet::lwmqtt_detect_packet_type(m_client.read_buf, 1, &packet_type);   //detect the packet type

  if (err != LWMQTT_SUCCESS)
  {
    return;
  }

  lwmqtt_packet_type_interpret(packet_type);
}
















//PRIVATE FUNCTIONS ==================================

//---------------------------------------------------
//Network and LifeCycle functions

//Generates a packet id. 
uint16_t MQTTClient::lwmqtt_get_next_packet_id()
{
  // check overflow
  if (m_client.last_packet_id == 65535)
  {
    m_client.last_packet_id = 1;
    return 1;
  }

  // increment packet id
  m_client.last_packet_id++;

  return m_client.last_packet_id;
}



//Does a simple network read (using network callback) then returns error status
lwmqtt_err_t MQTTClient::lwmqtt_read_from_network(size_t p_offset, size_t p_len)
{
  size_t readBytes = 0;
  lwmqtt_err_t err = m_client.network_read(m_client.network_read_ref, m_client.read_buf + p_offset, p_len, &readBytes, 2000);

  return err;
}



//Assumes a packet is stored in buffer. outputs bytes read and the packet type 
lwmqtt_err_t MQTTClient::lwmqtt_read_packet_in_buffer(size_t *o_read, lwmqtt_packet_type_t *o_packet_type)
{
  // read or wait for header byte
  lwmqtt_err_t err = MQTTClient::lwmqtt_read_from_network(0, VM_MQTT_READ_BUFFER_SIZE);
  if (err == LWMQTT_NETWORK_TIMEOUT)
  {
    // this is ok as no data has been read at all
    return LWMQTT_SUCCESS;
  }
  else if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  // preset packet type
  *o_packet_type = LWMQTT_NO_PACKET;

  // detect packet type
  err = Packet::lwmqtt_detect_packet_type(m_client.read_buf, 1, o_packet_type);

  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }

  return LWMQTT_SUCCESS;
}


//Assumes a packet is stored in the buffer. SEnds over the network
lwmqtt_err_t MQTTClient::lwmqtt_send_packet_in_buffer(size_t p_length)
{
  v.vpl(4, "Use network write callback to send buffered data over network");

  // write to network
  size_t written = 0;
  lwmqtt_err_t err = m_client.network_write(m_client.network_write_ref, m_client.write_buf, p_length, &written, m_client.command_timer->getTimeLeft());

  if (err != LWMQTT_SUCCESS)
  {
    v.vpl(4, "! ERROR: ", err);
    return err;
  }

  // reset keep alive timer
  m_client.keep_alive_timer->setTimeout(m_client.keep_alive_interval);

  return LWMQTT_SUCCESS;
}



//Simple cycle function that first reads a packet then decodes and uses it however needed
lwmqtt_err_t MQTTClient::lwmqtt_cycle(size_t *p_read, lwmqtt_packet_type_t *p_packet_type)
{
  // read next packet from the network
  lwmqtt_err_t err = lwmqtt_read_packet_in_buffer(p_read, p_packet_type);
  if (err != LWMQTT_SUCCESS)
  {
    return err;
  }
  else if (*p_packet_type == LWMQTT_NO_PACKET)
  {
    return LWMQTT_SUCCESS;
  }

  //Based on the packet type read, decode
  return lwmqtt_packet_type_interpret(*p_packet_type);
}


//Simple cycle function that first reads a packet then decodes and uses it however needed
lwmqtt_err_t MQTTClient::lwmqtt_cycle_until(lwmqtt_packet_type_t *p_packet_type, size_t p_available, lwmqtt_packet_type_t p_needle)
{
  // prepare counter
  size_t read = 0;

  // loop until timeout has been reached
  do
  {
    // do one cycle
    lwmqtt_err_t err = lwmqtt_cycle(&read, p_packet_type);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // return when one packet has been successfully read when no availability has been given
    if (p_needle == LWMQTT_NO_PACKET && p_available == 0)
    {
      return LWMQTT_SUCCESS;
    }

    // otherwise check if needle has been found
    if (*p_packet_type == p_needle)
    {
      return LWMQTT_SUCCESS;
    }
  } while (m_client.command_timer->getTimeLeft() > 0 && (p_available == 0 || read < p_available));

  return LWMQTT_SUCCESS;
}


//Assumes a packet is in the read buffer. Interprets the packet. 
lwmqtt_err_t MQTTClient::lwmqtt_packet_type_interpret(lwmqtt_packet_type_t p_packet_type){

  lwmqtt_err_t err;

  //Based on the packet type read, decode
  switch (p_packet_type)
  {
  // handle publish packets
  case LWMQTT_PUBLISH_PACKET:
  {
    // decode publish packet
    bool dup;
    uint16_t packet_id;
    lwmqtt_string_t topic;
    lwmqtt_message_t msg;
    err = Packet::lwmqtt_decode_publish(m_client.read_buf, m_client.read_buf_size, &dup, &packet_id, &topic, &msg);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // call callback if set
    if (m_client.callback != NULL)
    {
      m_client.callback(topic, msg);
    }

    // break early on qos zero
    if (msg.qos == LWMQTT_QOS0)
    {
      break;
    }

    // define ack packet
    lwmqtt_packet_type_t ack_type = LWMQTT_NO_PACKET;
    if (msg.qos == LWMQTT_QOS1)
    {
      ack_type = LWMQTT_PUBREC_PACKET;
    }
    else if (msg.qos == LWMQTT_QOS2)
    {
      ack_type = LWMQTT_PUBREL_PACKET;
    }

    // encode ack packet
    size_t len;
    err = Packet::lwmqtt_encode_ack(m_client.write_buf, m_client.write_buf_size, &len, ack_type, false, packet_id);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // send ack packet
    err = lwmqtt_send_packet_in_buffer(len);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    break;
  }

  // handle pubrec packets
  case LWMQTT_PUBREC_PACKET:
  {
    // decode pubrec packet
    bool dup;
    uint16_t packet_id;
    err = Packet::lwmqtt_decode_ack(m_client.read_buf, m_client.read_buf_size, p_packet_type, &dup, &packet_id);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // encode pubrel packet
    size_t len;
    err = Packet::lwmqtt_encode_ack(m_client.write_buf, m_client.write_buf_size, &len, LWMQTT_PUBREL_PACKET, 0, packet_id);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // send pubrel packet
    err = lwmqtt_send_packet_in_buffer(len);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    break;
  }

  // handle pubrel packets
  case LWMQTT_PUBREL_PACKET:
  {
    // decode pubrec packet
    bool dup;
    uint16_t packet_id;
    err = Packet::lwmqtt_decode_ack(m_client.read_buf, m_client.read_buf_size, p_packet_type, &dup, &packet_id);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // encode pubcomp packet
    size_t len;
    err = Packet::lwmqtt_encode_ack(m_client.write_buf, m_client.write_buf_size, &len, LWMQTT_PUBCOMP_PACKET, 0, packet_id);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    // send pubcomp packet
    err = lwmqtt_send_packet_in_buffer(len);
    if (err != LWMQTT_SUCCESS)
    {
      return err;
    }

    break;
  }

  // handle pingresp packets
  case LWMQTT_PINGRESP_PACKET:
  {
    // set flag
    m_client.pong_pending = false;

    break;
  }

  // handle all other packets
  default:
  {
    break;
  }
  }

  return LWMQTT_SUCCESS;

}
