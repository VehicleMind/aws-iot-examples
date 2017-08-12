#include "data_transmission.h"

Sample::Sample(){
    
}

Sample::~Sample(){

}

// Should always be called at the end of a Sample lifespan
void Sample::releasePIDs(){
    for(int i = 0; i < this->pidNumb; i++)
        delete[] this->pids[i].value;
    delete[] this->pids;
}

void Sample::makeHeader(const uint8_t pidClass, const uint8_t pidNumb){
    this->timestamp = SNTPUDP::getTimestamp();
    this->pidClass = pidClass;
    this->pidNumb = pidNumb;
    this->pids = new pidNode_t[this->pidNumb];
}

bool Sample::addPID(const uint8_t pid, const uint8_t* value, const uint8_t mode){
    uint8_t temp[4];

    if(!pidGetInf(pid, temp, mode)){ 
        delete[] this->pids;
        return false;
    }

    this->pids[pidsIndex].pid = (((uint16_t)temp[1]) << 8) | temp[0];
    this->pids[pidsIndex].type = temp[2];
    this->pids[pidsIndex].length = temp[3];
    this->pids[pidsIndex].value = new uint8_t[this->pids[pidsIndex].length];

    for(int i = 0; i < this->pids[pidsIndex].length; i++)
        this->pids[pidsIndex].value[i] = value[i];

    pidsIndex++;
    return true;
}

int Sample::encodeBin(uint8_t *o_buffer){
    int index = 0;

    index += BinaryPackager::addBytesAsInt8(o_buffer + index, &(this->timestamp), sizeof(this->timestamp));
    *(o_buffer + index++) = this->pidClass;
    *(o_buffer + index++) = this->pidNumb;

    for(int i = 0; i < this->pidNumb; i++){
        index += BinaryPackager::addBytesAsInt8(o_buffer + index, &(this->pids[i].pid), sizeof(this->pids[i].pid));
        *(o_buffer + index++) = this->pids[i].type;
        *(o_buffer + index++) = this->pids[i].length;
        for(int j = 0; j < this->pids[i].length; j++){
            *(o_buffer + index++) = this->pids[i].value[j];
        }
    }

    return index;
}

void Sample::print(){
    switch(this->pidClass){
        case PID_OBD:
            Serial.print("OBD ----  ");
            break;

        case PID_GPS:
            Serial.print("GPS ----  ");
            break;

        case PID_MEMS:
            Serial.print("MEMS ----  ");
            break;

        default:
            Serial.print("Other ----  ");
            break;
    }

    Serial.print("TIMESTAMP: ");
    SNTPUDP::printTimestamp(this->timestamp);
    Serial.println("");

    Serial.print("PID Class: ");
	Serial.println(this->pidClass, HEX);
	Serial.print("Number of PIDs: ");
	Serial.println(this->pidNumb);

	for(int i = 0; i < this->pidNumb; i++){
        Serial.printf("pid %d: ", i);
        Serial.println(this->pids[i].pid, HEX);
        Serial.print("pid type: ");
        Serial.println((type_t)this->pids[i].type);
        Serial.print("pid len: ");
        Serial.println(this->pids[i].length);
        Serial.print("pid val: ");
        for(int j = 0; j < this->pids[i].length; j++){
            Serial.print(this->pids[i].value[j], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

// ######

void dtcDecoder(uint16_t dtc, char* result){
    String res = "";

    byte firstNib = (dtc & 0xF000) >> 12;
    byte letter = (firstNib & 0b00001100) >> 2;
    byte firstDigit = firstNib & 0b00000011;
    uint16_t other = dtc & 0x0FFF;
    byte secondDigit = (other & 0x0F00) >> 8;
    byte thirdDigit = (other & 0x00F0) >> 4;
    byte fourthDigit = other & 0x000F;

    switch(letter){
        case 0b00:
            res += 'P';
            break;

        case 0b01:
            res += 'C';
            break;

        case 0b10:
            res += 'B';
            break;

        case 0b11:
            res += 'U';
            break;

        default:
            break;
    }

    res += String(firstDigit);
    res += String(secondDigit, HEX);
    res += String(thirdDigit, HEX);
    res += String(fourthDigit, HEX);
    
    res.toCharArray(result, 6);
}

int64_t byte2int(pidNode_t node){
    int16_t retVal = 0;
    uint8_t temp[4];
    if(!pidGetInf(((node.pid << 8) >> 8), temp, (node.pid >> 8))) return false;
    
    for(int i = 0; i < temp[3]; i++){
        retVal = retVal | (((uint16_t)node.value[i]) << i*8);
    }
    return retVal;
}

bool pidGetInf(const uint8_t pid, uint8_t* pidInf, uint8_t mode){
    for(int i = 0; i < NUMBER_OF_PIDS; i++)
        if(pidList[i][0] == pid){
            if(pidList[i][1] == mode){
                pidInf[0] = pidList[i][0];  // pid
                pidInf[1] = pidList[i][1];  // mode
                pidInf[2] = pidList[i][2];  // type
                pidInf[3] = pidList[i][3];  // len
                return true;
            }
        }
    return false;
}