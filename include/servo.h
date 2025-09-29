#include "pwm_lib.h"

class Servo {
public:
    Servo(){}
    void attach(uint gpio);
    void reset();
    void setAngle(uint angle);
private:
    PWM* out;
};