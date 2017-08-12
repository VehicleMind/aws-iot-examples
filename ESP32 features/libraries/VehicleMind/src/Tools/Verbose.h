//---------------------------------------------------
//INCLUDE GUARD
#ifndef VERBOSE_H_INCLUDED
#define VERBOSE_H_INCLUDED

//---------------------------------------------------
//INCLUDE DEPS
#include "HardwareSerial.h"

//---------------------------------------------------
//HEADER
class Verbose{

public:
    Verbose(bool m_active, const char* p_signature);

    void vp(int p_level, const char* p_string);   //Verbose Print
    void vp(int p_level, int p_int);
    void vp(int p_level, const char* p_string, int p_int);
    
    void vpl(int p_level, const char* p_string);   //Verbose Print
    void vpl(int p_level, int p_int);
    void vpl(int p_level, const char* p_string, int p_int);

    void p( const char* p_string);
    void p(int p_int);
    
    void l();

private:
    bool m_active;
    const char* m_signature;
};

#endif