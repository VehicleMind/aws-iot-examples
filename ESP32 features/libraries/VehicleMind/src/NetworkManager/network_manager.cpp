#include "network_manager.h"

//PUBLIC FUNCTIONS ==================================

//---------------------------------------------------
//CONSTRUCTOR and INIT FUNCTIONS

NetworkManager::NetworkManager():m_tcp_connector(6000), m_mqtt_client(), v(true, "_NM"){
    //Make a network events group and a semaphore
    m_network_events = xEventGroupCreate(); //Network events
    xEventGroupClearBits(m_network_events, NE_CONNECTED );  //Make sure all bits are clear

    m_uart_sema = xSemaphoreCreateBinary(); //UART resource lock
    xSemaphoreGive(m_uart_sema);    //Must give to start using    

    m_is_connected = false;
    totalAttempts = 0;
};


//Connects to network, then sends connect packet. If returns true can now send MQTT stuff
bool NetworkManager::connect(unsigned int p_retries){

    xEventGroupClearBits( m_network_events , NE_CONNECTED); //Make sure connected is clear
    m_is_connected = false;
    bool connection_successful = false;

    //Get the network set up to connect to the internet
    for(int i = 0; i<p_retries;i++){   
        if(!m_tcp_connector.connectNetwork(CELL_APN, CELL_USER, CELL_PASSWORD)) continue;
        if(!m_tcp_connector.openTCPSocket(TCP_IP, TCP_PORT, true, 0)) continue;
        if(!m_tcp_connector.openUDPSocket(2123, 9)) continue;
#ifdef DEBUG
        v.vpl(2, "Sockets Opened");
#endif
        connection_successful = true;
        setMqttClientId();
        break;
    }

    if(!connection_successful){
#ifdef DEBUG
        v.vpl(2, "Connection to network failed");
#endif
        return false;
    }

    //Set up the time
    unsigned int time_fails = 0;

    while(time_fails < 5){

        m_sntp.timeReset();
        m_sntp.constructRequest(m_sntp_packet);

        m_sntp.setOriginate();
        m_tcp_connector.sendUDPData((char*)m_sntp_packet, 48, "pool.ntp.org", 123, true, 9);

        for(int i = 0; i<100; i++){
            vTaskDelay(10);
            m_tcp_connector.extractRecievedData(this, &sntp_extract);
            if(m_sntp.isTimeSet()) break;
        }

        if(m_sntp.isTimeSet()) break;
    }

    //Network connection has been established. Initialize MQTT client
    m_mqtt_client.lwmqtt_set_network(this, (lwmqtt_network_read_t)&mqtt_read, this, (lwmqtt_network_write_t)&mqtt_write0);
    m_mqtt_client.lwmqtt_set_timers( &m_command_timer, &m_keepalive_timer );
    m_mqtt_client.lwmqtt_set_callback((lwmqtt_callback_t)&mqtt_callback );

    //Send connect packet to broker
    m_is_connected = protocolConnect();

    if(m_is_connected) {
        xEventGroupSetBits(m_network_events, NE_CONNECTED);    //if connected use NE_CONSET to setup connected, not sending, not receiving flags
    }

    //Create the tasks
#ifdef DEBUG
    v.vpl(1, "Setting Tasks");
#endif
    // xTaskCreate(recTaskCallback, "rec_task", 1024, this, 0, &m_recTask);
    // xTaskCreate(reconTaskCallback, "recon_task", 1024*2, this, 0, &m_reconTask);
#ifdef DEBUG
    v.vpl(1, "Tasks Set");
#endif

    return m_is_connected;
}

void NetworkManager::setMqttClientId(){
    ChipID chipId;
    char idBuffer[28] = {0};
    char simIMEI[16] = {0};

    // Device Id
    uint8_t uuidv5[16] = {0};
    char uuidNamespace[] = NAMESPACEID;
    char deviceId[37] = {0};

    chipId.getChipID(idBuffer);
    memcpy(simIMEI, &idBuffer[12], 15);

    // Generating Device Id by using uuid v5
    UUIDGEN::genUUIDv5((char*)uuidv5, uuidNamespace, simIMEI);
    uuidToString((uint8_t*)uuidv5, deviceId);

    memcpy(m_mqtt_client_id, deviceId, 37);
}

// Responsible for sending disconnection packet to broker
bool NetworkManager::disconnect(){
    lwmqtt_err_t error = m_mqtt_client.lwmqtt_disconnect(1000);

    //No need to repair connection here, if we can't send disconnect then we're already disconnected
    if(error !=  LWMQTT_SUCCESS){
        Serial.print("Attempt to disconect was unsuccessful. lwqtt_err_code: ");
        Serial.print(error);
        m_is_connected = false;
        return false;
    }

    m_is_connected = false;
    xEventGroupClearBits(m_network_events, NE_CONNECTED );

    Serial.println("MQTT disconnection sent");

    return true;
}


void NetworkManager::send(const char *p_type, const char *p_message){
        bool usema = xSemaphoreTake( m_uart_sema, 2000);   
        if(!usema) return;
  
        //Wait for connectino bits
        EventBits_t bits = xEventGroupWaitBits( m_network_events, NE_CONNECTED, pdFALSE, pdTRUE, 2000);        
        do{
            //break and rewait if not connected still
            if( (bits & NE_CONNECTED ) != NE_CONNECTED ) break; 
            
            //Send the Data
            if(!protocolSend(0, p_type, p_message)){
                xEventGroupClearBits(m_network_events, NE_CONNECTED);       
                m_is_connected = false;
                repairConnection();
            }

        } while(false);
        xSemaphoreGive( m_uart_sema );
}

void NetworkManager::send(const char *p_type, uint8_t *p_values, unsigned int p_size){
        bool usema = xSemaphoreTake( m_uart_sema, 0);   
        if(!usema) return;

        //Wait for connectino bits
        EventBits_t bits = xEventGroupWaitBits( m_network_events, NE_CONNECTED, pdFALSE, pdTRUE, 0);        
        do{
            //break and rewait if not connected still
            if( (bits & NE_CONNECTED ) != NE_CONNECTED ) break; 
            
            //Send the Data
            if(!protocolSend(0, p_type, p_values, p_size)){
                xEventGroupClearBits(m_network_events, NE_CONNECTED);       
                m_is_connected = false;
                repairConnection();
            }

        } while(false);
        xSemaphoreGive( m_uart_sema );
}




//PRIVATE FUNCTIONS ==================================

//---------------------------------------------------
//TASK CALLBACKS


//Receieve Task
void NetworkManager::recTaskCallback(void* p_ref){
    //Set ref to networkManager
    NetworkManager* manager = (NetworkManager*)p_ref;
    const TickType_t xTicksToWait = 1000;

    while(1){
        // esp_task_wdt_feed();
        esp_task_wdt_feed();
        EventBits_t bits = xEventGroupWaitBits( manager->m_network_events, NE_CONNECTED, pdFALSE, pdTRUE, xTicksToWait);    

        do{
            bool sema = xSemaphoreTake( manager->m_uart_sema, 2000);

            if( (bits & NE_CONNECTED) != NE_CONNECTED ) break;
            
            size_t data_size;
            if(manager->m_tcp_connector.recDataAvailable(&data_size)){ 
                Serial.println("Extract"); 
                manager->m_tcp_connector.extractRecievedData((void *)manager, &(manager->dealWithExtraction));
            }

        } while(false);
        xSemaphoreGive( manager->m_uart_sema );
    }
}


//---------------------------------------------------
//NETWORK FUNCTIONS


// Attempt to repair the the network with last used credentials
bool NetworkManager::repairConnection(){
    xEventGroupClearBits( m_network_events , NE_CONNECTED); //Clear connected event flag
    (this->totalAttempts)++;
    while(!connect(5));
    return m_is_connected;
}


// Responsible for sending connect packet to broker
bool NetworkManager::protocolConnect(){
    
    lwmqtt_will_t p_will = { 
        .topic=(lwmqtt_string_t) { .len=0, .data="" }, 
        .qos=LWMQTT_QOS0, 
        .retained=false, 
        .payload=(lwmqtt_string_t) { .len=0, .data="" } 
    };
    
    // char uuid[37];              //size 37 to add the null terminating character 0
    // UUIDGEN::genUUIDv4(mqttClientId);

    lwmqtt_options_t options = { 
        .client_id=(lwmqtt_string_t){.len = strlen(m_mqtt_client_id), .data = m_mqtt_client_id },    //36 because don't need null terminating with strings
        .keep_alive=1000, 
        .clean_session=true, 
        .username=(lwmqtt_string_t) { .len=0, .data="" }, 
        .password=(lwmqtt_string_t) { .len=0, .data="" }  
    };



    //Attempt connection a number of times
    lwmqtt_err_t connect_error;
    
    for(int i = 0; i<MQTT_CONNNECT_ATTEMPTS; i++){
        connect_error = m_mqtt_client.lwmqtt_connect( options, &p_will, &m_last_connection_return_code, MQTT_CONNNECT_TIMEOUT);
        if(connect_error == LWMQTT_SUCCESS) break;
    }

    //If all the above attempts fail, repair connection
    if(connect_error != LWMQTT_SUCCESS){
        Serial.print("Attempt to connect was unsuccessful. NO CONNACK RECIEVED. lwqtt_err_code: ");
        Serial.println(connect_error);        
        return false;
    }

    //Connection packet was received but the response was not success, return false
    if(m_last_connection_return_code != LWMQTT_CONNECTION_ACCEPTED){   
        Serial.print("Attempt to connect was rejected. CONNACK WAS RECIEVED. lwqtt_return_code: ");
        Serial.print(m_last_connection_return_code);
        return false;
    }    

    return subscribe("Test"); //If success, do subscriptions;
}


// Subscribe to a topic
bool NetworkManager::subscribe(const char* p_topic){
    //Attempt an extraction loop incase there is something to receive
    extractLoop();

    //Attempt to subscribe
    lwmqtt_err_t error = m_mqtt_client.lwmqtt_subscribe_one( (lwmqtt_string_t) { .len=(uint16_t)strlen(p_topic), .data=p_topic }, LWMQTT_QOS0, 3000 );

    //If you fail don't worry about reconnection. This all happens in the network connectino and will connect automatically on failures
    if(error !=  LWMQTT_SUCCESS){
        Serial.print("Attempt to subscribe was unsuccessful. lwqtt_err_code: ");
        Serial.print(error);
        return false;
    }

    Serial.print("Subscription successful to: ");
    Serial.println(p_topic);
    return true;
}

// Publish a message to a topic
bool NetworkManager::protocolSend(int p_channel, const char *p_topic, const char *p_message){
#ifdef DEBUG
    v.vpl(2, "Sending data");
#endif
    //Set up to topic and message properly
    lwmqtt_string_t topic = (lwmqtt_string_t) { .len=(uint16_t)strlen(p_topic), .data=p_topic };
    lwmqtt_message_t message = (lwmqtt_message_t) { .qos=LWMQTT_QOS0, .retained=false, .payload=(uint8_t*) p_message, (size_t)strlen(p_message) };

    //Attempt to publish a few times
    lwmqtt_err_t error;

    for(int i = 0; i<MQTT_PUBLISH_ATTEMPTS; i++){ 
#ifdef DEBUG
        if(i>=1){

            v.vpl(2, "! Publish failed, Reattempt number ", i);
        } 
#endif

        error =  m_mqtt_client.lwmqtt_publish(topic, message, MQTT_PUBLISH_TIMEOUT);
        if(error == LWMQTT_SUCCESS) break;
    }

    if(error !=  LWMQTT_SUCCESS){
#ifdef DEBUG
        v.vpl(2, "Attempt to publish was unsuccessful. lwqtt_err_code: ", error);
#endif
        xEventGroupClearBits( m_network_events , NE_CONNECTED); //Make sure connected is clear
        m_is_connected = false;
        return false;
    }

    return true;
}

bool NetworkManager::protocolSend(int p_channel, const char *p_topic, uint8_t *p_message, unsigned int p_message_size){
#ifdef DEBUG
    v.vpl(2, "Sending data");
#endif
    //Set up to topic and message properly
    lwmqtt_string_t topic = (lwmqtt_string_t) { .len=(uint16_t)strlen(p_topic), .data=p_topic };
    lwmqtt_message_t message = (lwmqtt_message_t) { .qos=LWMQTT_QOS0, .retained=false, .payload=p_message, p_message_size };

    //Attempt to publish a few times
    lwmqtt_err_t error;

    for(int i = 0; i<MQTT_PUBLISH_ATTEMPTS; i++){ 
#ifdef DEBUG
        if(i>=1){
            v.vpl(2, "! Publish failed, Reattempt number ", i);
        } 
#endif

        error =  m_mqtt_client.lwmqtt_publish(topic, message, MQTT_PUBLISH_TIMEOUT);
        if(error == LWMQTT_SUCCESS) break;
    }

    if(error !=  LWMQTT_SUCCESS){
#ifdef DEBUG
        v.vpl(2, "Attempt to publish was unsuccessful. lwqtt_err_code: ", error);
#endif
        xEventGroupClearBits( m_network_events , NE_CONNECTED); //Make sure connected is clear
        m_is_connected = false;
        return false;
    }

    return true;
}


//---------------------------------------------------
//Message extraction


// Loop that handles if data needs extracting
void NetworkManager::extractLoop(){
    //if data available, extract that data. Else move on
    size_t data_size;
    if(m_tcp_connector.recDataAvailable(&data_size)) m_tcp_connector.extractRecievedData((void *)this, &dealWithExtraction);
}

// Callback that deals with the extraction. I know this is a bit convuluted but it lets us work around some stuff with out changing too much code
void NetworkManager::dealWithExtraction(void* p_ref, char* p_extraction, int p_len){
    NetworkManager* ref = (NetworkManager*)p_ref;
    ref->m_mqtt_client.lwmqtt_process_packet(p_extraction, p_len);      //Basically pass the extraction to the lwmqtt_process_packet
}




//---------------------------------------------------
// CALLBACKS FOR MQTT

//Callback that reads data from the network to the buffer
lwmqtt_err_t NetworkManager::mqtt_read(void* p_ref, uint8_t *buf, size_t len, size_t *read_length, int32_t timeout)
{
    //Set reference to correct type
    NetworkManager* p_network = (NetworkManager*) p_ref;
    
    //use the reference to call receive data from the m_tcp_connector variable
    int length = 0;
    char *recData = p_network->m_tcp_connector.recieveData(&length, timeout);

    if(length == 0) return LWMQTT_SUCCESS;

    //Copy data over to buffer. If we go over buffer limit, just stop reading and finalize with a 0 character. 
    for (int i = 0; i < length; i++)
    {
        *(buf + i) = *(recData + i);

        if (i == len - 1)
        {
            *(buf + i) = 0;
            length = len;
            break;
        }
    }

    *read_length = length;

    return LWMQTT_SUCCESS;
}



// Callback that writes data to UART out
lwmqtt_err_t NetworkManager::mqtt_write(void* p_ref, int buf, size_t p_len, size_t *sent, int32_t timeout, unsigned int p_socket)
{       
    //Set void* type properly
    NetworkManager* p_network = (NetworkManager*) p_ref;
#ifdef DEBUG
    p_network->v.vpl(4, "(Write Callback) Writing to the UART out");
#endif
    //Attempt to send data during timeout
    signed int t = millis() + timeout;

    do
    {
#ifdef DEBUG
        p_network->v.vpl(4, "Attempting to send data");
#endif
        if (p_network->m_tcp_connector.sendTCPData((const char *)buf, p_len, p_socket)){
#ifdef DEBUG
            p_network->v.vpl(4, "(/Write Callback/) SUCCESS");
#endif
            return LWMQTT_SUCCESS;
        }  
    } while (t - (signed int)millis() > 0);

#ifdef DEBUG
    p_network->v.vpl(4, "(/Write Callback/) FAILURE");
#endif
    return LWMQTT_NETWORK_FAILED_WRITE;
}

// Callback that forwards received packets
void NetworkManager::mqtt_callback(lwmqtt_string_t str, lwmqtt_message_t msg)
{
    Serial.println("RECEIVED SUBSCRIPTION MESSAGE: ");

    for(int i = 0; i<msg.payload_len; i++){
        Serial.print((char)msg.payload[i]);
    }

    Serial.println("");
}


void NetworkManager::sntp_extract(void* p_ref, char* p_extract, int p_len){
    NetworkManager* ref = (NetworkManager*) p_ref;
#ifdef DEBUG
    Serial.println(p_len);
#endif
    if(p_len != 48) return;

#ifdef DEBUG
    for(int i = 0; i<p_len; i++){
        Serial.print(p_extract[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
#endif

    ref->m_sntp.setDeviceTime((uint8_t*)p_extract);
}
