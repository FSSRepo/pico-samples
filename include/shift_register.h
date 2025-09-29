#ifndef SHIFTREGISTER
#define SHIFTREGISTER

#include "pico/stdlib.h"

class ShiftRegister {
    private:
    uint8_t dataPin = 0;
    uint8_t latchPin = 0;
    uint8_t clockPin = 0;
    
    public:
    ShiftRegister(){}
    void init(uint8_t data = 13,uint8_t latch = 14,uint8_t clock = 15);
    void update(uint32_t data_packed);
};

#endif