#include "pico/stdlib.h"

class LED {
public:
LED(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    this->pin = pin;
}

bool isOn(){
    return turn_on;
}

void setState(bool on) {
    gpio_put(this->pin, on);
    turn_on = on;
}
private:
int pin;
bool turn_on = false;
};