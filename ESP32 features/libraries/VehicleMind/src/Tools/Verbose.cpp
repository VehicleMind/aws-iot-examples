#include "Verbose.h"

//HELPER FUNCTIONS
Verbose::Verbose(bool p_active, const char* p_signature){
    m_active = p_active;
    m_signature = p_signature;
}

void Verbose::vp(int p_level, const char* p_print){  
    if(m_active){
        Serial.print(m_signature);
        Serial.print(" ");
        for(int i=0; i<p_level; i++){
            Serial.print('-');
        }
        Serial.print(" ");
        Serial.print(p_print);
    } 
}
  
void Verbose::vp(int p_level, int p_print){  
    if(m_active){
        Serial.print(m_signature);
        Serial.print(" ");
        for(int i=0; i<p_level; i++){
            Serial.print('-');
        }
        Serial.print(" ");
        Serial.print(p_print);
    } 
}

void Verbose::vp(int p_level, const char* p_print, int p_print2){  
    if(m_active){
        Serial.print(m_signature);
        Serial.print(" ");
        for(int i=0; i<p_level; i++){
            Serial.print('-');
        }
        Serial.print(" ");
        Serial.print(p_print);
        Serial.print(p_print2);
    } 
}

void Verbose::vpl(int p_level, const char* p_print){  
        if(m_active){
        Serial.print(m_signature);
        Serial.print(" ");
        for(int i=0; i<p_level; i++){
            Serial.print('-');
        }
        Serial.print(" ");
        Serial.println(p_print);
    } 
}

void Verbose::vpl(int p_level, int p_print){  
        if(m_active){
        Serial.print(m_signature);
        Serial.print(" ");
        for(int i=0; i<p_level; i++){
            Serial.print('-');
        }
        Serial.print(" ");
        Serial.println(p_print);
    } 
}

void Verbose::vpl(int p_level, const char* p_print, int p_print2){  
    if(m_active){
        Serial.print(m_signature);
        Serial.print(" ");
        for(int i=0; i<p_level; i++){
            Serial.print('-');
        }
        Serial.print(" ");
        Serial.print(p_print);
        Serial.println(p_print2);
    } 
}

void Verbose::p(const char* p_print){  
    if(m_active){
        Serial.print(p_print);
    } 
}

void Verbose::p(int p_print){  
    if(m_active){
        Serial.print(p_print);
    } 
}

void Verbose::l(){  
    if(m_active){
        Serial.println("");
    } 
}