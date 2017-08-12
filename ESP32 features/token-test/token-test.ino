#include <stdio.h>
#include <string.h>

void setup() {

  char uuid_in[] = "e728b802-0fff-5b7b-bf47-51bacb";
  char uuid_out[33] = {0};
  char * token;
  char * pos = uuid_out;
  
  delay(5000);
  Serial.begin(115200);

  Serial.println("tokenizing string ...");

  token = strtok(uuid_in, "-");
  
  while (token != NULL) {
    strncpy(pos, token, strlen(token));
    pos += strlen(token);
    token = strtok(NULL, "-");    
  }

  Serial.print("UUID: ");
  Serial.println(uuid_out);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
