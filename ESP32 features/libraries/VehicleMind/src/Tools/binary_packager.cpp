#include "binary_packager.h"
#include "../NetworkManager/sntp/sntp_udp.h"


//Set up the packet by giving a buffer+size to hold packet, and the size of data indicator, crc and each data chunk.
void BinaryPackager::startPackage(uint8_t* o_buffer, int o_buffer_size, int p_chunk_size ){
    //Save the parameters
    m_buffer = o_buffer;
    m_buffer_size = o_buffer_size;
    m_chunk_size = p_chunk_size;

    //Setup the inner (data) buffer;
    m_inner_buffer = m_buffer + 2 + 8;    //The start of the inner buffer is after the datalength (size 2) and timestamp (size 8)
    m_inner_buffer_size = m_buffer_size - 2 - 8 - 4; //The size of the inner buffer is same as outer buffer minus the space for datalength, timestamp and CRC
    m_bytes_written = 0;  //Set index of inner buffer to 0

    //Now we have an inner buffer that can be used to add data
}

//Is there space available to add another chunk to the packet?
bool BinaryPackager::spaceAvailable(){
    return (m_bytes_written + m_chunk_size) < m_inner_buffer_size;   //If adding another chunk goes over inner buffer size the can't add another chunk
}

// Overloaded to check for the chunk size
bool BinaryPackager::spaceAvailable(int p_chunk_size){
    return (m_bytes_written + p_chunk_size) < m_inner_buffer_size;   //If adding another chunk goes over inner buffer size the can't add another chunk
}

//Returns the pointer to the index of the buffer where the next chunk should be written.
bool BinaryPackager::addChunk(uint8_t* p_chunk){
    if(!spaceAvailable()) return false;   //Don't add chunk if no space is available

    memcpy(m_inner_buffer + m_bytes_written, p_chunk, m_chunk_size);   //Cpy the chunk to the packet
    m_bytes_written += m_chunk_size;    //Increment the inner buffer index

    return true;
}

// Overloaded method that allows to define a different chunk size
bool BinaryPackager::addChunk(uint8_t* p_chunk, int p_chunk_size){
    if(!spaceAvailable(p_chunk_size)) return false;   //Don't add chunk if no space is available

    memcpy(m_inner_buffer + m_bytes_written, p_chunk, p_chunk_size);   //Cpy the chunk to the packet
    m_bytes_written += p_chunk_size;    //Increment the inner buffer index

    return true;
}

//Returns true if atleast 1 chunck was written
bool BinaryPackager::hasData(){
    return m_bytes_written;
}

//Calculate the data length and the CRC. Return the final length of the packet. (The packet is in the buffer passed in from the scope calling the BinaryPackager)
int BinaryPackager::finalizePacket(){
    uint16_t bytes_written = (uint16_t) (m_bytes_written + 8);                   // +8 for the timestamp
    addBytesAsInt8(m_buffer, &bytes_written, sizeof(bytes_written));

    uint64_t timestamp = SNTPUDP::getTimestamp();

    addBytesAsInt8(m_buffer+2, &timestamp, sizeof(timestamp));
    uint32_t crc = CRC32::calculate(m_buffer+2, m_bytes_written+8);
    addBytesAsInt8(m_inner_buffer+m_bytes_written, &crc, sizeof(crc));

    return m_bytes_written + 2 + 8 + 4;                 // +2 for the packet size, +8 for the timestamp, +4 for the CRC
}

int BinaryPackager::addBytesAsInt8(uint8_t *o_buffer, void *to_copy, int num_bytes){
    memcpy(o_buffer, to_copy, num_bytes);
    return num_bytes;
}
