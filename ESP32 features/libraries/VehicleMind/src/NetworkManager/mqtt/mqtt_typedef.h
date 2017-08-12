//---------------------------------------------------
//INCLUDE GUARD
#ifndef MQTT_TYPEDEF_H_INCLUDED
#define MQTT_TYPEDEF_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPENDENCIES
#include <stdbool.h>
#include <stddef.h>
#include "../utils/common_types.h"
#include "../utils/timer.h"

//---------------------------------------------------
//HEADER
// #define DEBUG

//FORWARD DEP
typedef struct lwmqtt_client_t lwmqtt_client_t;

//LWMQTT Definitions
typedef struct
{
    uint16_t len;
    const char *data;
} lwmqtt_string_t;

typedef enum {
    LWMQTT_SUCCESS = 0,
    LWMQTT_BUFFER_TOO_SHORT = -1,
    LWMQTT_VARNUM_OVERFLOW = -2,
    LWMQTT_NETWORK_FAILED_CONNECT = -3,
    LWMQTT_NETWORK_TIMEOUT = -4,
    LWMQTT_NETWORK_FAILED_READ = -5,
    LWMQTT_NETWORK_FAILED_WRITE = -6,
    LWMQTT_REMAINING_LENGTH_OVERFLOW = -7,
    LWMQTT_REMAINING_LENGTH_MISMATCH = -8,
    LWMQTT_MISSING_OR_WRONG_PACKET = -9,
    LWMQTT_CONNECTION_DENIED = -10,
    LWMQTT_FAILED_SUBSCRIPTION = -11,
    LWMQTT_SUBACK_ARRAY_OVERFLOW = -12,
    LWMQTT_PONG_TIMEOUT = -13,
} lwmqtt_err_t;

typedef enum {
    LWMQTT_QOS0 = 0,
    LWMQTT_QOS1 = 1,
    LWMQTT_QOS2 = 2,
    LWMQTT_QOS_FAILURE = 128
} lwmqtt_qos_t;

typedef struct
{
    lwmqtt_string_t topic;
    lwmqtt_qos_t qos;
    bool retained;
    lwmqtt_string_t payload;
} lwmqtt_will_t;

typedef enum {
    LWMQTT_CONNECTION_ACCEPTED = 0,
    LWMQTT_UNACCEPTABLE_PROTOCOL = 1,
    LWMQTT_IDENTIFIER_REJECTED = 2,
    LWMQTT_SERVER_UNAVAILABLE = 3,
    LWMQTT_BAD_USERNAME_OR_PASSWORD = 4,
    LWMQTT_NOT_AUTHORIZED = 5,
    LWMQTT_UNKNOWN_RETURN_CODE = 6
} lwmqtt_return_code_t;

typedef struct
{
    lwmqtt_string_t client_id;
    uint16_t keep_alive;
    bool clean_session;
    lwmqtt_string_t username;
    lwmqtt_string_t password;
} lwmqtt_options_t;

typedef struct
{
    lwmqtt_qos_t qos;
    bool retained;
    uint8_t *payload;
    size_t payload_len;
} lwmqtt_message_t;

typedef enum {
    LWMQTT_NO_PACKET = 0,
    LWMQTT_CONNECT_PACKET = 1,
    LWMQTT_CONNACK_PACKET,
    LWMQTT_PUBLISH_PACKET,
    LWMQTT_PUBACK_PACKET,
    LWMQTT_PUBREC_PACKET,
    LWMQTT_PUBREL_PACKET,
    LWMQTT_PUBCOMP_PACKET,
    LWMQTT_SUBSCRIBE_PACKET,
    LWMQTT_SUBACK_PACKET,
    LWMQTT_UNSUBSCRIBE_PACKET,
    LWMQTT_UNSUBACK_PACKET,
    LWMQTT_PINGREQ_PACKET,
    LWMQTT_PINGRESP_PACKET,
    LWMQTT_DISCONNECT_PACKET
} lwmqtt_packet_type_t;

//forward incoming messages
typedef void (*lwmqtt_callback_t)(lwmqtt_string_t str, lwmqtt_message_t msg);

//Read from network object
typedef lwmqtt_err_t (*lwmqtt_network_read_t)(void *ref, uint8_t *buf, size_t len, size_t *read, uint32_t timeout);

//Write to network object
typedef lwmqtt_err_t (*lwmqtt_network_write_t)(void *ref, uint8_t *buf, size_t len, size_t *sent, uint32_t timeout);

// typedef void (*lwmqtt_timer_set_t)(int32_t *ref, uint32_t timeout);

// typedef int32_t (*lwmqtt_timer_get_t)(int32_t *ref);

struct lwmqtt_client_t
{
    uint16_t last_packet_id;
    uint32_t keep_alive_interval;
    bool pong_pending;

    size_t write_buf_size, read_buf_size;
    uint8_t *write_buf, *read_buf;

    lwmqtt_callback_t callback;

    void *network_read_ref;
    lwmqtt_network_read_t network_read;

    void *network_write_ref;
    lwmqtt_network_write_t network_write;

    Timer *keep_alive_timer;
    Timer *command_timer;
};


#endif
