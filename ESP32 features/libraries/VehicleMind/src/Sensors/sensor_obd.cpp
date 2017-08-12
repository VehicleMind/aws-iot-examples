#include "sensor_obd.h"

bool SensorOBD::initialize()
{
#ifdef DEBUG
    Serial.println("Going to begin obd");
#endif
    begin();
#ifdef DEBUG
    Serial.println("Going to init obd");
#endif
    bool initSuc = init();
#ifdef DEBUG
    Serial.println("Init Complete");
    Serial.println(initSuc);
#endif
    return initSuc;
}

bool SensorOBD::read(const uint8_t p_datamode, const uint8_t p_pid, Sample* p_obd_buffer){
    uint8_t tempVal[32];
    if(!readPID(p_datamode, p_pid, tempVal)){
        return false;
    }

    p_obd_buffer->makeHeader(PID_OBD, 1);  // Serial OBD data fetch
    if(!p_obd_buffer->addPID(p_pid, tempVal, p_datamode)){
        return false;
    }

    return true;
}

bool SensorOBD::readNext(Sample* p_obd_buffer){
    bool successfully_read = false;
    
    if(m_pids[m_pid_index][1] == PID_MODE_9)
        successfully_read = read(PID_MODE_9, m_pids[m_pid_index++][0], p_obd_buffer);
    else
        successfully_read = read(PID_MODE_1, m_pids[m_pid_index++][0], p_obd_buffer);

    if(m_pid_index == OBD_NUMBER_OF_PIDS) m_pid_index = 0;

    return successfully_read;
}

bool SensorOBD::readVIN(char* buffer){
  char buff[128];
  if (getVIN(buff, sizeof(buff))){
      return true;
  } else {
    return false;
  }
}

bool SensorOBD::checkDTC(){
  char buffer[128];
  if (pollDTC(buffer, sizeof(buffer))){
    switch (buffer[0]){
      case '0':
        return false;
      case '1':
          return true;
    }
  }
  return false;
}

bool SensorOBD::checkErrors(){
    return (this->errors > MAX_OBD_ERRORS);
}