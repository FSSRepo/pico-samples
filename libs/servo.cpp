#include "servo.h"

void Servo::attach(uint gpio){
    out = new PWM(gpio);
    out->freq(50); // default 50 Hz
}

void Servo::reset(){
    out->duty(1637);
    sleep_ms(1);
}

void Servo::setAngle(uint angle) {
    out->duty(1637 + (uint32_t)(angle * 36.33f));
    sleep_ms(1);
}