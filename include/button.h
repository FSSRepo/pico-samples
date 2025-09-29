#include "pico/stdlib.h"

class Button {
public:
Button(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_down(pin);
    this->pin = pin;
}

bool isPressed() {
    return gpio_get(this->pin);
}
private:
int pin;
};