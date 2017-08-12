//---------------------------------------------------
//INCLUDE GUARD
#ifndef MQTT_STRING_HELPERS_H_INCLUDED
#define MQTT_STRING_HELPERS_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPENDENCIES
#include <string.h>
#include "mqtt_typedef.h"

//---------------------------------------------------
//HEADER
class StringHelpers{
  
public:
  static lwmqtt_string_t lwmqtt_string(const char *str);
  static int lwmqtt_strcmp(lwmqtt_string_t a, const char *b);

};


#endif