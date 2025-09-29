#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#define TOP_MAX 65534

class PWM {
    private:
        uint8_t pin;
        uint32_t top = 0;
        uint32_t div = 0;
        uint slice;
        uint channel;

    public:
        PWM(uint8_t gpio);
        uint freq(uint32_t frequency);
        void duty(uint32_t value);
        
};