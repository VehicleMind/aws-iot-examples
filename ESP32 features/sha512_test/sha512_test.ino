#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "sha512.h"
#include "hwcrypto/sha.h"
#include "mbedtls/sha1.h"
 
using std::string;
using std::cout;
using std::endl;
 
void setup()
{
  uint8_t mac_addr[8] = {0};
  esp_err_t ret = ESP_OK;
  char sha1in[] = "abc";
  char sha1out[20] = {0};
  uint64_t _chipmacid;

  Serial.begin(115200);
  delay(5000);
  
  //esp_efuse_mac_get_default((uint8_t*) (&_chipmacid));

  ret = esp_efuse_mac_get_default(mac_addr);
  // ret = esp_efuse_mac_get_custom(mac_addr);

  Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], mac_addr[6], mac_addr[7]);
  
    string input = "6c40a24865357020193957";
    //string input = "grape";
    string output1 = sha512(input);

  
    Serial.print("Input: "); Serial.println(input.c_str());
    Serial.print("Output: "); Serial.println(output1.c_str());
    Serial.print("Size: "); Serial.println(output1.size());

    Serial.print("I threw a random die and got ");
    Serial.print(random(1,7));

    Serial.print(". Then I threw a TrueRandom die and got ");
    Serial.println(esp_random());

    // usage as String
  // SHA1:a94a8fe5ccb19ba61c4c0873d391e987982fbbd3

  Serial.print("SHA1:");
  mbedtls_sha1((unsigned char*)sha1in, strlen(sha1in), (unsigned char*)sha1out);

//  // usage as ptr
//  // SHA1:a94a8fe5ccb19ba61c4c0873d391e987982fbbd3
//  uint8_t hash[20];
//  sha1("abc", &hash[0]);
//
//  Serial.print("SHA1:");
  for (uint16_t i = 0; i < 20; i++) {
    Serial.printf("%02x", sha1out[i]);
  }
  Serial.println();
}

void loop()
{
}
