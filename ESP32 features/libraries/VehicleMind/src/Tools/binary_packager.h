//---------------------------------------------------
//INCLUDE GUARD
#ifndef BINARY_PACKAGER_H_INCLUDED
#define BINARY_PACKAGER_H_INCLUDED

//---------------------------------------------------
//DEPS
#include <string.h>
#include <stdint.h>

#include <CRC32.h>        //Uses the CRC-CCITT(XModem)

#include <time.h>
#include <sys/time.h>

//---------------------------------------------------
//HEADER

class BinaryPackager
{
  public:
    static int addBytesAsInt8(uint8_t *o_buffer, void *to_copy, int num_bytes);

    //Set up the packet by giving a buffer+size to hold packet, and the size of data indicator, crc and each data chunk.
    void startPackage(uint8_t* o_buffer, int o_buffer_size, int p_chunk_size );

    //Is there space available to add another chunk to the packet?
    bool spaceAvailable();

    // Overloaded to check for the chunk size
    bool spaceAvailable(int p_chunk_size);

    //Returns the pointer to the index of the buffer where the next chunk should be written.
    bool addChunk(uint8_t* p_chunk);

    // Overloaded method that allows to define a different chunk size
    bool addChunk(uint8_t* p_chunk, int p_chunk_size);

    bool hasData();

    //Calculate the data length and the CRC. Retunr the final length of the packet. (The packet is in the buffer passed in from the scope calling the BinaryPackager)
    int finalizePacket();


  private:
    //Varaibles which dictate the way the buffer is filled
    int m_chunk_size;

    //Pointer and size of the total buffer <data size> <data> <CRC>
    uint8_t* m_buffer;
    int m_buffer_size;

    //Pointer to the start of the data section of the buffer <data>
    uint8_t* m_inner_buffer;
    int m_inner_buffer_size;
    int m_bytes_written;
};

#endif
